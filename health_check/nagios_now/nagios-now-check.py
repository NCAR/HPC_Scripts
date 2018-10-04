#!/usr/bin/env python
#todo: add depends? does this actually matter?

import syslog
import argparse #yum install python-argparse.noarch
import copy
import os
import socket
import subprocess 
import pipes
import sys
import re
import shlex
from datetime import datetime
import fcntl
import json
import time

def msg(*strmsg):
    """ Send message to user """
    global args, reportfile

    try:
        out = ' '.join(map(str,strmsg)) + '\n'
        sys.stderr.write(out)
        syslog.syslog(str(out))
        return True

    except:
        return False
def vmsg(*strmsg):
    """ Send verbose message to user """
    global args
    if args.verbose:
	msg(*strmsg)
def exec_to_file ( cmd, output_file ):
    """ Runs cmd and pipes STDOUT to output_file """
    fo = open(output_file, 'w')
    if args.verbose:
	msg('Running command: %s' % cmd) 
	msg('Dumping command STDOUT: %s' % output_file) 
	p = subprocess.Popen(cmd, stdout=fo, cwd='/tmp/')
    else:
	p = subprocess.Popen(cmd, stdout=fo, stderr=FNULL, cwd='/tmp/')
    p.wait()
    fo.flush()
    fo.close()
    return p.returncode
def exec_to_string_with_input ( cmd, input):
    """ Runs cmd, sends input to STDIN and places Return Value, STDOUT, STDERR into returned list  """
    if args.verbose:
        msg('Running command: %s with input:\n%s' % (cmd, input)) 
    try:
	p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd='/tmp/')
	stdout, stderr = p.communicate(input=input)
	return [p.returncode, stdout, stderr ]
    except:
	msg('Command %s failed' % cmd)
	return [-1, '', 'Failed to run']
def exec_to_string ( cmd ):
    """ Runs cmd and places Return Value, STDOUT, STDERR into returned list  """
    if args.verbose:
	msg('Running command: %s' % cmd) 
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd='/tmp/')
    stdout, stderr = p.communicate()
    return [p.returncode, stdout, stderr ]

def eval_check_host(retcode, stdout, stderr, check):
    """ Evaluates if check commands passed or failed depending on type """

    cpass = False
    if check['Type'] == "Return Success":
        cpass = retcode == 0
        cwarn = 0
    if check['Type'] == "List Errors":
        cpass = len(stdout) == 0
        cwarn = 0
    if check['Type'] == "Nagios Plugin":
        cpass = retcode == 0
        cwarn = retcode == 1

    return ( cpass, cwarn )
 
def run_check_host(check, host_name ):
    """ Run check on single host """

    #detemine how many times to try
    check_tries = 1
    if 'Retry' in check and check['Retry']:
        check_tries = check['Retry']

    #default to fail
    cpass   = False
    cwarn   = False
    retcode = None
    stdout  = None
    stderr  = None

    for i in range(0, check_tries):
        #replace the host into the command
        cmd = copy.copy(check['Command']);
        for cmd_iterator, cmd_str in enumerate(cmd):
            cmd[cmd_iterator] = cmd_str.replace('%HOST%', host_name);

        #call the command
        retcode, stdout, stderr = exec_to_string(cmd);
        vmsg('Try=%s/%s Return Code=%s stdout=%s stderr=%s' % (i + 1, check_tries, retcode, stdout.strip(), stderr.strip()))

        cpass, cwarn = eval_check_host(retcode, stdout, stderr, check)

        #all done if it worked
        if cpass and not cwarn:
            break

        #something went wrong, sleep for next try
        if i != check_tries - 1:
            vmsg('Sleep for %s seconds before retry' % (check["Retry Delay"]))
            time.sleep(check["Retry Delay"])

    return ( cpass, cwarn, retcode, stdout, stderr )

def run_check(check, hosts):
    stats = {'pass': 0, 'fail': 0, 'status': 'Fail', 'details': []}
    if check['Type'] in ["Nagios Plugin", "Return Success", "List Errors"]:
        for host_group in check['Hosts']:
            for host_name in hosts[host_group]:
                cpass, cwarn, retcode, stdout, stderr = run_check_host(check, host_name)

                if check['SLA']['Type'] == "PassCount":
                    if cpass:
                        stats['pass'] += 1;
                    else:
                        stats['fail'] += 1;

                        why = ""
                        linesp = stdout.split(os.linesep)
                        if len(linesp) > 1:
                            why = linesp[1]

                        stats['details'].append('{0} failed {1}:{2}'.format(check['Name'],  host_name, why));
                elif check['SLA']['Type'] == "Return":
                    if cpass:
                        stats['pass'] += 1;
                        stats['status'] = 'green';
                    else:
                        if cwarn:
                            stats['fail'] += 1;
                            stats['status'] = 'yellow';
                        else: # retcode == 2: 
                            stats['fail'] += 1;
                            stats['status'] = 'red';

                        #pull out the Nagios description from the sensor
                        err_message = stdout.split('|');

                        stats['details'].append('{0} failed {1}:{2}'.format(check['Name'], host_name, err_message[1] if len(err_message) > 1 else stdout ));
    else:
        msg('Unknown Type {0} in Check'.format(check['Type']))

    if "SLA" in check:
        if check['SLA']['Type'] == "PassCount":
            if 'Green' in check['SLA'] and stats['pass'] >= check['SLA']['Green']:
                stats['status'] = 'green';
            elif 'Yellow' in check['SLA'] and stats['pass'] >= check['SLA']['Yellow']:
                stats['status'] = 'yellow';
            else:
                if 'Red' in check['SLA']:
                    if stats['pass'] >= check['SLA']['Red']:
                        stats['status'] = 'red';
                    else: #if number is below Red, then unknown is implied
                        stats['status'] = 'unknown';
                else: #assume red is default if Red isnt set
                        stats['status'] = 'red';
        elif check['SLA']['Type'] == "Return":
            None #dont do anything here
        else:
            msg('Unknown Type {0} in SLA'.format(stats['SLA']['Type']))
    else: #If there is no SLA, assume any failure is red
        if stats['fail'] > 0:
            stats['status'] = 'red';
        else: #nothing failed so we are good
            stats['status'] = 'green';

    vmsg('Status=%s Details=%s' % (stats['status'], stats['details']))
    return stats


syslog.openlog('nagios-now-check.py')

parser = argparse.ArgumentParser(description='SSG\'s Big Nagios Now check script')
parser.add_argument('-c','--config', dest='config_path', help='Path to JSON config', required=True)
parser.add_argument('-v','--verbose', dest='verbose', help='Be verbose')
args = parser.parse_args()

#only run with lock
LOCK = open('/var/run/nagios-now-check.py', 'a')
fcntl.flock(LOCK.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)

#open config
config_file = open(args.config_path,'r')
config = json.load(config_file)
config_file.close()

#vmsg(config);

nagios_states = { #nagios state to human readable
    0: 'green',
    1:'yellow',
    2:'red',
    3:'unknown',
    'green': 0,
    'yellow': 1,
    'red': 2,
    'unknown': 3
}
nagios_nsca = {}; #updates to send via nsca
for nagios_name in config['Nagios']:
    nagios_nsca[nagios_name] = list();

for monitor_name in config['Monitor']:
    monitor = config['Monitor'][monitor_name];
    if monitor['Type'] == "Aggregated": 
        status = 'unknown'
        details = []
        for check in iter(monitor['Checks']):
            stats = run_check(check, config['Hosts']);
            details.extend(stats['details'])
            if stats['status'] == 'red':
                status = 'red';
            if stats['status'] == 'yellow' and status != 'red':
                status = 'yellow';
            if stats['status'] == 'green' and status != 'red' and status != 'yellow':
                status = 'green';

        #Allow certain systems to only send warnings
        if 'Nagios Override' in monitor and monitor['Nagios Override'] == "Warn Only":
            if status != 'green':
                status = 'yellow';

        for nagios_name, nagios in config['Nagios'].iteritems():
            detail = '';
            if nagios['Type'] == "Full":
                detail = '.'.join(details);
            elif nagios['Type'] == "Minimal":
                if status in nagios['Service Detail']:
                    detail = nagios['Service Detail'][status];
                if 'Service Detail' in monitor and status in monitor['Service Detail']: 
                    detail = monitor['Service Detail'][status];

            #nsca cant handle an empty detail statement
            if not detail:
                detail = 'ok'

            #Update Nagios Host with green since this check is working
            nagios_nsca[nagios_name].append("{0};0;{1}".format(
                monitor['Nagios Name'], 
                nagios['Host Detail']
            ));
            #Update Nagios service with status
            nagios_nsca[nagios_name].append('{0};{0};{1};{2}'.format(
                monitor['Nagios Name'],
                nagios_states[status],
                detail
            ));
    else:
        msg('Unknown Type {0} in Monitor {1}'.format(monitor['Type'], monitor_name))
    
for nagios_name, nagios in config['Nagios'].iteritems():
    cmd = nagios['NSCA'];
    updates = nagios_nsca[nagios_name]
    updates.append('')
    retcode, stdout, stderr = exec_to_string_with_input(cmd, "\n".join(nagios_nsca[nagios_name]));
    if len(stderr) > 0:
        for line in stderr.split('\n'):
            vmsg('nsca errors: %s' % (line))
    if len(stdout) > 0:
        for line in stdout.split('\n'):
            vmsg('nsca output: %s' % (line))

