#!/bin/bash
[ -z "$1" ] && echo "$0 [path to mpi_oom]" && exit 1

mkdir -p $P_OUTPUT/

cat <<-EOF
	#!/bin/sh
	#BSUB -n 2
	#BSUB -R "span[ptile=2]"
	#BSUB -W 0:30
	#BSUB -a poe
	#BSUB -network 'type=sn_all'
	#BSUB -q $QUEUE
	#BSUB -J mpi_oom
	#BSUB -o $P_OUTPUT/%J.log
	#BSUB -e $P_OUTPUT/%J.log
	#BSUB -cwd $P_OUTPUT
	#BSUB -P SSSG0001

	export TARGET_CPU_RANGE="-1"
	export LANG=en_US
	export MP_PE_AFFINITY=no
	
	poe $(readlink -e $1)
	EOF

