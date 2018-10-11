#!/bin/bash
[ -z "$1" ] && echo $0 '{path to MPI dir}' && exit 1
P_OSU="$(readlink -e "$1")"
[ ! -d "$P_OSU" ] && echo "MPI directory incorrect" && exit 1

P_INTELCC=${P_INTELCC:-/ncar/opt/intel/2013/}
P_IMPI=${P_IMPI:-/ncar/opt/intel/2013/impi/4.1.3.048/}
P_PMPI=${P_PMPI:-/ncar/opt/platform_mpi/9.1.2/}
P_POE=${P_POE:-/opt/ibmhpc/pecurrent/base/}
P_MVAPICH=${P_MVAPICH:-/picnic/scratch/ssgadmin/mvapich/2-2.0/}

#mpi environment
declare -A mpiV
mpiV[impi]=". $P_IMPI/bin64/mpivars.sh;"
mpiV[pmpi]="export PATH=$P_PMPI/bin:\$PATH MPI_ROOT=$P_PMPI"
[ "$USEPOE" = "yes" ] && mpiV[poe]="export PATH=$P_POE/bin:\$PATH TARGET_CPU_RANGE=\"-1\" LANG=en_US MP_PE_AFFINITY=no"
mpiV[mvapich]="export PATH=$P_MVAPICH/bin:\$PATH RSH_CMD=/usr/bin/ssh"

declare -A mpiR 
mpiR[impi]="mpirun -n \$LSB_DJOB_NUMPROC -hostfile \$LSB_DJOB_HOSTFILE"
mpiR[pmpi]="mpirun -lsf "
[ "$USEPOE" = "yes" ] && mpiR[poe]=poe
mpiR[mvapich]="mpirun_rsh -ssh -np \$LSB_DJOB_NUMPROC -hostfile \$LSB_DJOB_HOSTFILE"

mkdir -p $OUTPUT_PATH/job/

(  #create job batch file
	cat <<-EOF
		#!/bin/bash

		. $P_INTELCC/bin/iccvars.sh intel64 lp64
	EOF
	
	for mpi in "${!mpiV[@]}"
	do
		echo "("
		echo "${mpiV[$mpi]}"

		ls $P_OSU/$mpi/libexec/osu-micro-benchmarks/mpi | while read type
		do
			PE=$P_OSU/$mpi/libexec/osu-micro-benchmarks/mpi/$type
			ls $PE  | while read test
			do
				PL=$OUTPUT_PATH/$mpi/$type/$test
				mkdir -p $PL

				echo "echo ${mpiR[$mpi]} $PE/$test 1>&2 "
				echo "/usr/bin/time -o $PL/\$LSB_JOBID.\$LSB_JOBINDEX.time ${mpiR[$mpi]} $PE/$test > $PL/\$LSB_JOBID.\$LSB_JOBINDEX;"
			done
		done

		echo "); true; "
	done
)  > $OUTPUT_PATH/job/job.sh

PEREQ=""
[ "$USEPOE" = "yes" ] && PEREQ="#BSUB -a poe -network 'type=sn_all'"

#create job batch file
cat <<-EOF
	#!/bin/bash
	#BSUB -q $QUEUE
	#BSUB -x
	#BSUB -J osu[1-$RUNLOOPS]
	#BSUB -n $CPUS
	#BSUB -R "span[ptile=$PTILE]"
	#BSUB -o $OUTPUT_PATH/job/%J.log
	#BSUB -e $OUTPUT_PATH/job/%J.log
	#BSUB -W $WCTIME
	#BSUB -cwd $OUTPUT_PATH
	#BSUB -P SSSG0001
	$PEREQ
	
	. /etc/profile.d/modules.sh
	module purge
	
	bash job/job.sh
	EOF

