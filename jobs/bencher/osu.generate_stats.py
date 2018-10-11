#!/usr/bin/python
import sys
import os
import re
import subprocess 

if len(sys.argv) < 3:
    print >> sys.stderr, '%s {path to mergeouputs.py} {path to output dir} {path to data}* ' % (sys.argv[0])
    os._exit(1)


sources = sys.argv;
sources.pop(0); #prog
pmerge=os.path.abspath(sources.pop(0))
poutput=sources.pop(0)

data = {}

#walk every source and make array of data sources for merging
for psrc in sources:
    if os.path.isdir(psrc):
	for mpi in ['impi', 'mvapich', 'pmpi', 'poe']:
	    if os.path.isdir('%s/%s' % (psrc, mpi)):
		for ttype in ['one-sided', 'collective', 'pt2pt']:
		    path='%s/%s/%s/' % (psrc, mpi, ttype)
		    if os.path.isdir(path):
			for filename in os.listdir(path):
			    if os.path.isdir(path + filename):
				#use directory to get the average file name
				tname="%s:%s" % (ttype, filename)
				fpath="%s%s.averages.txt" % (path, filename)
				label="%s:%s" % (os.path.basename(psrc), mpi)
				if not tname in data:
				    data[tname] = []
				data[tname].append("%s|%s" % (label, fpath))


for tname, sources in data.iteritems():
    outf = open("%s/%s.csv" % (poutput, tname), 'w')
    cmd = [ pmerge, 'dump', ',' ] + sources
    p = subprocess.Popen(cmd, stdout=outf, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()
    if stderr != "":
	print cmd
	print stderr
    outf.close()

