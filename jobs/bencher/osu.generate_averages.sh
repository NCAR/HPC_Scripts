#!/bin/bash
[ -z "$1"  ] && echo "$0 {path to mergeouputs.py} {path to output}" && exit 1

pmerge="$1"
path="$2"

for mpi in impi mvapich pmpi poe
do
    for type in one-sided collective pt2pt
    do
	if [ -d $path/$mpi/$type ]
	then
	    ls $path/$mpi/$type|grep -v averages | while read test
	    do
		if [ -d $path/$mpi/$type/$test/ ]
		then
		    echo $mpi:$type:$test
		    if [ ! -f $path/$mpi/$type/$test.averages.txt ]
		    then
			find $path/$mpi/$type/$test/ ! -name \*.time -type f | xargs $pmerge average '' > $path/$mpi/$type/$test.averages.txt
		    else
			echo skip $mpi:$type:$test
		    fi
		else
		    echo "$mpi:$type:$test failed"	>&2
		    echo $path/$mpi/$type/$test	>&2
		fi
	    done
	fi
    done
done

