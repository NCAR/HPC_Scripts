#!/usr/bin/python
import sys
import os
import re

if len(sys.argv) < 3:
    print >> sys.stderr, '%s {dump|average} {separator} {label|filename}* ' % (sys.argv[0])
    os._exit(1)

args = sys.argv;
args.pop(0); #prog
output_type=args.pop(0)
separator=args.pop(0)
if separator == '':
    separator = None

data = {}   #big dictionary with all data
labels = {} #list of all known labels (use dictionary for speed)

for arg in args:
    file_label = ''
    filename = ''
 
    if arg.find('|') == -1:
	filename = arg
	file_label = os.path.basename(filename)
    else:
	file_label, filename = arg.split('|')

    src = open(filename, 'r')
    for line in src:
	if line[0] != '#':
	    label = None
	    value = None
	    buf = line.split(separator)
	    label = buf[0]
	    if label != 'lsfLibVerNum' and label != '.':
		value = buf[1]

		if not label in labels:
		    labels[label] = None
		if not file_label in data:
		    data[file_label] = {}
		data[file_label][label] = float(value)
    src.close()

if output_type == 'average': 
    averages = {}

    for label in labels.keys():
	values = []
	for jobid in data.keys():
	    if label in data[jobid]: #just ignore non-existance
		values.append(data[jobid][label])
	averages[label] = sum(values, 0.0) / len(values)

    data = {}
    data['average'] = averages

#dump label,jobids[]
sys.stdout.write('#label')
for jobid in data.keys():
    sys.stdout.write(',%s' % (jobid))

for label in labels.keys():
    sys.stdout.write('\n')
    sys.stdout.write('%s' % (label))
    for jobid in data.keys():
	sys.stdout.write(',%s' % (data[jobid][label]))

