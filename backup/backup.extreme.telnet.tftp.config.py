#!/usr/bin/python
import pexpect
import sys

#https://pexpect.readthedocs.io/en/stable/overview.html

if len(sys.argv) != 2:
    print >> sys.stderr, '''
    %s {switch hostname}

    This script will connect a given Extreme switch via telnet and will send the switch config to local SAC node tftp dir
    
    ''' % (__file__)
    sys.exit(1)    

SW=sys.argv[1]
print 'Connecting to %s' % (SW)

def ecmd(child):
    return child.expect('Slot-\d [a-z0-9\.]+ # ')

child = pexpect.spawn('/usr/bin/telnet %s' % (SW))
child.logfile = sys.stdout

child.expect('login:')
child.sendline('admin')
child.expect('password:')
child.sendline('admin')

ecmd(child)
child.sendline('disable clipaging')

ecmd(child)

child.sendline('save config as-script %s-config' % (SW))

child.expect('Do you want to save configuration to script')

child.sendline('Y')

ecmd(child)

child.sendline('tftp put 172.23.0.1 vr vr-default %s-config.xsf' % (SW))

ecmd(child)

child.sendline('exit')


i = child.expect(['Do you wish to save your configuration changes to primary.cfg?',pexpect.EOF])
if i == 0:
    child.sendline('N')

