#!/usr/bin/env python
import argparse #yum install python-argparse.noarch
import os
import socket
import subprocess 
import pipes
import sys
import re

def msg(*args):
    """ Send message to user """
    sys.stderr.write(' '.join(map(str,args)) + '\n')
def exec_to_string ( cmd ):
    """ Runs cmd and places Return Value, STDOUT, STDERR into returned list  """
    if args.verbose:
	msg('Running command: %s' % cmd) 
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd='/tmp/')
    stdout, stderr = p.communicate()
    return [p.returncode, stdout, stderr ]
def parse_port ( label ):
    """ Parse the name of a IB port 
    returns dictionary with parsed values

    Known Formats:
	'ys4618 HCA-1'(4594/1)
	'ys4618 HCA-1'(4594/1)
	MF0;ys75ib1:SXX536/L05/U1/P2
	ys75ib1/L05/U1/P2
	ys46ib1:SX60XX/U1/P26
	MF0;ca00ib1a:SXX512/S01/U1
	'MF0;ys72ib1:SXX536/L22/U1'(395/1)
	geyser1/H3/P1
	ys70ib1 L05 P12
	ys22ib1 P13 
    	ys2324 HCA-1
    	geyser01 HCA-1 P3

    """
    global args

    name  = None
    hca   = None
    leaf  = None
    spine = None #only used for internal orca connections
    port  = None   

    global ib_portname_type1_regex
    global ib_portname_type2_regex
    if ib_portname_type1_regex == None:
	#regex matches following:
	#'ys4618 HCA-1'(4594/1)
	#'ys4618 HCA-1'(4594/1)
	#MF0;ys75ib1:SXX536/L05/U1/P2
	#ys75ib1/L05/U1/P2
	#ys46ib1:SX60XX/U1/P26
	#MF0;ca00ib1a:SXX512/S01/U1
	#'MF0;ys72ib1:SXX536/L22/U1'(395/1)
	#geyser1/H3/P1
	ib_portname_type1_regex = re.compile(
		r"""
		^\s*
		(?:\'|)
		(?:
		    (?P<hca_host_name>\w+)\s+			#Host name
		    [hcaHCA]+-(?P<hca_id>\d+)			#HCA number
		    |                          	
		    (?:MF0;|)					#MF0 - useless id	
		    (?P<tca_host_name>\w+)			#TCA Name
		    (?::SX\w+|)					#Switch Type
		    (?:\/[hcaHCA]{1,3}(?P<hca_id2>\d+)|)	#HCA number
		    (?:\/[lLiIdD]+(?P<leaf>\d+)|)		#Leaf (sometimes called /LID in error)
		    (?:\/S(?P<spine>\d+)|)			#Spine
		    (?:\/U\d+|)					#U number
		    (?:\/P(?P<port1>\d+)|)			#Port
		)
		(?:
		    (?:\'|)
		    \(
			\d+					#LID: just assume it is wrong
			\/
			(?P<port2>\d+)				#Port
		    \)
		    |
		)
		\s*$
		""",
		re.VERBOSE
		) 

    match = ib_portname_type1_regex.match(label)
    if match:
	if args.verbose:
	    msg("matched: %s %s" % (label, match.groups()))
	if match.group('hca_host_name'):
	    name = match.group('hca_host_name')
	    hca = match.group('hca_id')
	if match.group('tca_host_name'):
	    name = match.group('tca_host_name')
	    spine = match.group('spine')
	    hca = match.group('hca_id2')
	    leaf = match.group('leaf') 
	if match.group('port1'):
	    port = match.group('port1')
	if match.group('port2'):
	    port = match.group('port2')
    else:
	if ib_portname_type2_regex == None:
	    #regex matches following: (these are usually from human entry)
	    #ys70ib1 L05 P12
	    #ys22ib1 P13 
	    #ys2324 HCA-1
	    #geyser01 HCA-1 P3
	    ib_portname_type2_regex = re.compile(
		    r"""
		    ^\s*
		    (?P<name>\w+)			#name
		    (?:
			(?:\s+
			[hcaHCA]+(?:-|)(?P<hca>\d+)	#hca id
			)
			|
		    )
		    (?:\s+
			[lLiIdD]+			
			(?P<leaf>\d+)			#leaf (called lid in error)
			|
		    )	
		    (?:\s+U\d+|)			#/U useless
		    (?:
			(?:\s+[pP](?P<port>\d+))	#port number
			|
			)
		    \s*$
		    """,
		    re.VERBOSE
		    ) 
	match = ib_portname_type2_regex.match(label)
	if match:
	    if args.verbose:
		msg("matched: %s %s" % (label, match.groups()))
	    name = match.group('name')
	    hca = match.group('hca')
	    leaf = match.group('leaf')
	    port = match.group('port')

    return {
		'name'	: name,
		'hca'	: hca,
		'leaf'	: leaf,
		'spine'	: spine,
		'port'	: port,
		'connection' : None,
		'guid' : None,
		'type' : None,
		'speed' : None,
		'width' : None,
		'lid' : None,
		'serial' : None,
		'partnumber' : None,
		'partrevision' : None,
		'length' : None
	    }
 
def register_cable ( port1, port2 ):
    """ add cable ports to ports list (for now). port2 can be None for unconnected ports. """
    global ports, portlids

    if port1['lid'] in portlids and port1['port'] in portlids[port1['lid']]:
	return
    if port2 and port2['lid'] in portlids and port2['port'] in portlids[port2['lid']]:
	return

    ports.append(port1)
    if not port1['lid'] in portlids:
	portlids[port1['lid']] = {}
    portlids[port1['lid']][port1['port']] = port1
    if port2:
	ports.append(port2) 
	if not port2['lid'] in portlids:
	    portlids[port2['lid']] = {}
	portlids[port2['lid']][port2['port']] = port2 

def parse_ibnetdiscover_cables ( contents ):
    """ Parse the output of 'ibnetdiscover -p' 

    Two types of line formats:
    CA    44  1 0x0002c9030045f121 4x FDR - SW     2 17 0x0002c903006e1430 ( 'localhost HCA-1' - 'MF0;js01ib2:SX60XX/U1' )
    SW     2 19 0x0002c903006e1430 4x SDR                                    'MF0;js01ib2:SX60XX/U1'

    """
    global args

    ibcable_regex = re.compile(
	    r"""
	    ^(?P<HCA1_type>CA|SW)\s+		#HCA1 type
	    (?P<HCA1_lid>\d+)\s+		#HCA1 LID
	    (?P<HCA1_port>\d+)\s+		#HCA1 Port
	    (?P<HCA1_guid>0x\w+)\s+		#HCA1 GUID
	    (?P<width>\w+)\s+			#Cable Width
	    (?P<speed>\w+)\s+			#Cable Speed
	    (
		\'(?P<HCA_name>.+)\'		#Port Name
		|				#cable is connected
		-\s+		
		(?P<HCA2_type>CA|SW)\s+		#HCA2 Type
		(?P<HCA2_lid>\d+)\s+		#HCA2 LID
		(?P<HCA2_port>\d+)\s+		#HCA2 Port
		(?P<HCA2_guid>0x\w+)\s+		#HCA2 GUID
		\(\s+
		    \'(?P<HCA1_name>.+)\'	#HCA1 Name
		    \s+-\s
		    +\'(?P<HCA2_name>.+)\'	#HCA2 Name
		\s+\)
	    )$
	    """,
	    re.VERBOSE
	    ) 
    for line in contents.split(os.linesep):
	match = ibcable_regex.match(line)
	if match:
	    if match.group('HCA_name'):
		port = parse_port(match.group('HCA_name'))
		port['port'] = match.group('HCA1_port')
		port['lid'] = match.group('HCA1_lid')
		port['guid'] = match.group('HCA1_guid')
		port['type'] = match.group('HCA1_type')
		port['speed'] = match.group('speed')
		port['width'] = match.group('width')
		port['connection'] = None
		register_cable(port, None)
		if args.verbose:
		    msg(port)
	    else:
		port1 = parse_port(match.group('HCA1_name'))
		port1['port'] = match.group('HCA1_port')
		port1['lid'] = match.group('HCA1_lid')
		port1['type'] = match.group('HCA1_type')
		port1['guid'] = match.group('HCA1_guid')
		port1['speed'] = match.group('speed')
		port1['width'] = match.group('width')

		port2 = parse_port(match.group('HCA2_name'))
		port2['port'] = match.group('HCA2_port')
		port2['lid'] = match.group('HCA2_lid')
		port2['guid'] = match.group('HCA2_guid')
		port2['type'] = match.group('HCA2_type')
		port2['speed'] = match.group('speed')
		port2['width'] = match.group('width')

		if args.verbose:
		    msg(port1)
		    msg(port2)

		#cross reference connecting port
		port1['connection'] = port2
		port2['connection'] = port1
		register_cable(port1, port2)
	else:
	    if args.verbose and line != "":
		msg('Parse fail: %s' % line ) 
def parse_cable_info ( contents ):
    """ Parse the output of '/ssg/bin/mlnx_cable_info ' 
	populates the port info
    """
    global args, ports
    #js01ib1: Slot 1 port 13 state
    #js01ib1:        identifier             : QSFP+
    #js01ib1:        cable/ module type     : Passive copper, unequalized
    #js01ib1:        infiniband speeds      : SDR , DDR , QDR , FDR
    #js01ib1:        vendor                 : Mellanox
    #js01ib1:        cable length           : 1 m
    #js01ib1:        part number            : 00W0049
    #js01ib1:        revision               : A1
    #js01ib1:        serial number          : 2004923W019
    #js01ib1:

    #ys74ib: Slot L01 port 2 state
    #ys74ib:         identifier             : QSFP+
    #ys74ib:         cable/ module type     : Optical cable/ module
    #ys74ib:         infiniband speeds      : SDR , DDR , QDR , FDR
    #ys74ib:         vendor                 : Mellanox
    #ys74ib:         cable length           : 15 m
    #ys74ib:         part number            : 00W0081
    #ys74ib:         revision               : A1
    #ys74ib:         serial number          : 40081394030


    ib_cable_regex = re.compile(r"""
	(?P<switch>^[a-zA-Z0-9]+)	    #switch name
	:\s
	(?:
	    Slot\s
	    (?:1|L(?P<leaf>[0-9]+))\s  #slot 1 or LEAF id on orcas
	    port\s
	    (?P<port>\d+)		    #port
	    \sstate
	    |
	    \s+(?P<property>[a-zA-Z0-9/]+|(?:[a-zA-Z0-9/]+[ ][a-zA-Z0-9/]+)+)\s+:\s	#property name
	    (?P<value>.*?)								#property value
	    |
	    \s+(?:Cable\sis\snot\spresent\.)					#no cable exists
	)
	\s*$
	""", re.VERBOSE) 

    mport = None #matched port
    cport = None #connected matched port
    for line in contents.split("\n"):
	match = ib_cable_regex.match(line)
	if match:
	    #is this a slot/port statement? find the port
	    if match.group('port'): 
		found = None
		for port in ports:
		    if ( 
			port['name'] == match.group('switch') and 
			port['leaf'] == match.group('leaf') and 
			port['port'] == match.group('port')
		       ):
			#if mport:
			#    print mport
			mport = port
			cport = port['connection']
			found = True
		#if not found:
		#    print 'match failed: %s' % (match.group())
		#    print match.groups()
	    #record properties
	    if found and match.group('property') and mport: 
		if match.group('property') == 'serial number' and match.group('value') != "-":
		    mport['serial'] = match.group('value')
		    if cport:
			cport['serial'] = match.group('value')
		if match.group('property') == 'cable length' and match.group('value') != "-":
		    mport['length'] = match.group('value')
		    if cport:
			cport['length'] = match.group('value')
		if match.group('property') == 'part number' and match.group('value') != "-":
		    mport['partnumber'] = match.group('value')
		    if cport:
			cport['partnumber'] = match.group('value')
		if match.group('property') == 'revision' and match.group('value') != "-":
		    mport['partrevision'] = match.group('value')
		    if cport:
			cport['partrevision'] = match.group('value')
                         
def port_pretty ( port ):
    """ return pretty port label """
    if not port:
	return 'None'
    if port['spine']: #spine
	return '%s/S%s/P%s' % (port['name'], port['spine'], port['port'])
    if port['leaf']: #port on orca
	return '%s/L%s/P%s' % (port['name'], port['leaf'], port['port']) 
    if port['hca']: #hca on node
	return '%s/HCA%s/P%s' % (port['name'], port['hca'], port['port'])
    return '%s/P%s' % (port['name'], port['port']) #tor port 
def dump_cable ( port ):
    """ return pretty cable label """
    cport = None
    clid = 'NA'
    if port['connection']:
	cport = port['connection'] 
	clid = cport['lid']

    if args.dumplid:
	print '%s LID=%s <--> %s LID=%s' % (port_pretty(port), port['lid'],port_pretty(cport), clid)
    elif args.dumpportstate: 
	comment='# %s <--> %s' % (port_pretty(port), port_pretty(cport))  
	print 'ibportstate %s %s %s # %s <--> %s' % (port['lid'], port['port'], args.dumpportstate, port_pretty(port), port_pretty(cport))
	if cport != None:
	    print 'ibportstate %s %s %s # %s <--> %s' % (cport['lid'], cport['port'], args.dumpportstate, port_pretty(cport), port_pretty(port))
    else:
	if args.cableinfo:
	    print '%s <--> %s SN=%s PN=%s REV=%s LEN=%s ' % (
		    port_pretty(port), 
		    port_pretty(cport), 
		    port['serial'],
		    port['partnumber'],
		    port['partrevision'],
		    port['length']
		    ) 
	else:
	    print '%s <--> %s' % (port_pretty(port), port_pretty(cport)) 
 

parser = argparse.ArgumentParser(description='search against ibnetdiscover.')
parser.add_argument('-s','--spine', dest='spine', help='search for given spine', required=False)
parser.add_argument('-l','--leaf',  dest='leaf', help='search for given leaf', required=False)
parser.add_argument('--lid',  dest='lid', help='search for given LID', required=False)
parser.add_argument('--guid',  dest='guid', help='search for given GUID', required=False)
parser.add_argument('--full',  dest='fullport', help='parse full port label to search', required=False)
parser.add_argument('-n','--name',  dest='name', help='search for given name', required=False)
parser.add_argument('-p','--port',  dest='port', help='search for given port', required=False)
parser.add_argument('-c','--hca',   dest='hca', help='search for given hca', required=False)
parser.add_argument('-i','--ibnetdiscover',   dest='source', help='path to ibnetdiscover output', required=True)
parser.add_argument('-v','--verbose',   dest='verbose', help='Be verbose', action="store_true", required=False)
parser.add_argument('--dump-lid',   dest='dumplid', help='Dump port LID', action="store_true", required=False)
parser.add_argument('--dump-portstate', dest='dumpportstate', help='Dump as portstate', required=False)
parser.add_argument('--dump-cableinfo', dest='cableinfo', help='Parse output of Cable info (dumps serial and product number)', required=False)
args = parser.parse_args()

#globals for later
ib_portname_type1_regex = None 
ib_portname_type2_regex = None
ports = []  #list containing all known ports
portlids = {}  #list containing all known lid->ports => port

#todo: implement direct query
if(args.source):
    file = open(args.source,'r')
    parse_ibnetdiscover_cables(file.read()) 
    file.close()          

if(args.cableinfo):
    file = open(args.cableinfo,'r')
    parse_cable_info(file.read()) 
    file.close()           

if args.fullport == None:
    #regex compares
    rspine	= None
    rleaf	= None
    rname	= None
    rhca	= None
    rport	= None
    rlid	= None
    rguid	= None
    if args.lid:
	rlid = re.compile(args.lid) 
    if args.guid:
	rguid = re.compile(args.guid) 
    if args.spine:
	rspine = re.compile(args.spine)
    if args.leaf:
	rleaf = re.compile(args.leaf)
    if args.name:
	rname = re.compile(args.name)
    if args.hca:
	rhca = re.compile(args.hca)
    if args.port:
	rport = re.compile(args.port)

    #search every port
    for port in ports:
	match = False
	fail = False
 	if rguid:
	    if port['guid'] and rguid.match(port['guid']):
		match=True
	    else:
		fail=True  
	if rlid:
	    if port['lid'] and rlid.match(port['lid']):
		match=True
	    else:
		fail=True 
	if rspine:
	    if port['spine'] and rspine.match(port['spine']):
		match=True
	    else:
		fail=True
	if rleaf:
	    if port['leaf'] and rleaf.match(port['leaf']): 
		match=True
	    else:
		fail=True
	if rname:
	    if port['name'] and rname.match(port['name']): 
		match=True
	    else:
		fail=True
	if rport:
	    if port['port'] and rport.match(port['port']): 
		match=True
	    else:
		fail=True
	if rhca:
	    if port['hca'] and rhca.match(port['hca']): 
		match=True
	    else:
		fail=True
	if match and not fail:
	    dump_cable(port)

if args.fullport != None:
    sport = parse_port(args.fullport)
    for port in ports:
	if( #only check for values that would exist in a label
	    port['name']    == sport['name'] and
	    port['hca']	    == sport['hca'] and
	    port['leaf']    == sport['leaf'] and
	    port['spine']   == sport['spine'] and
	    port['port']    == sport['port'] 
	    ):
	    dump_cable(port)

 
