#!/bin/bash
[ -z "$1" -o -z "$2" -o -z "$3" -o -z "$4" ] && echo $0 '{path to hpcg dir} {path to output dir} {ptile} {nodes}' && exit 1

QUEUE=special
P_HPCG="$(readlink -e "$1")"
P_OUTPUT="$2"
PTILE="$3"
NODES="$4"

export LM_LICENSE_FILE=28518@128.117.177.41,28518@128.117.183.213,28518@128.117.183.214,28518@128.117.183.215
export P_INTELCC=/ncar/opt/intel/psxe-2015/
export P_IMPI=$P_INTELCC/impi_latest/
export P_PMPI=${P_PMPI:-/ncar/opt/platform_mpi/9.1.2/}
export P_POE=${P_POE:-/opt/ibmhpc/pecurrent/base/}
export P_MVAPICH=${P_MVAPICH:-/picnic/scratch/ssgadmin/mvapich/2-2.0/}

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

. /etc/profile.d/modules.sh
module purge

mkdir -p $P_OUTPUT/job/
#cp $P_HPCG/hpcg.dat $P_OUTPUT/job/

#(  #create job batch file
#	cat <<-EOF
#		#!/bin/bash
#
#		. $P_INTELCC/bin/iccvars.sh intel64 lp64
#	EOF
#	
#	for mpi in "${!mpiV[@]}"
#	do
#		echo "("
#		echo "${mpiV[$mpi]}"
#
#		ls $P_OSU/$mpi/libexec/osu-micro-benchmarks/mpi | while read type
#		do
#			PE=$P_OSU/$mpi/libexec/osu-micro-benchmarks/mpi/$type
#			ls $PE  | while read test
#			do
#				PL=$P_OUTPUT/$mpi/$type/$test
#				mkdir -p $PL
#
#				echo "echo ${mpiR[$mpi]} $PE/$test 1>&2 "
#				echo "/usr/bin/time -o $PL/\$LSB_JOBID.\$LSB_JOBINDEX.time ${mpiR[$mpi]} $PE/$test > $PL/\$LSB_JOBID.\$LSB_JOBINDEX;"
#			done
#		done
#
#		echo "); true; "
#	done
#)  > $P_OUTPUT/job/job.sh

PEREQ=""
[ "$USEPOE" = "yes" ] && PEREQ="#BSUB -a poe -network 'type=sn_all'"
[ ! -z "$RESREQ" ] && RESREQ="#BSUB -R \"$RESREQ\""

#create job batch file
cat <<-EOF
	#!/bin/bash
	#BSUB -q $QUEUE
	#BSUB -x
	#BSUB -J hpcg
	#BSUB -a poe -network 'type=sn_all'
	#BSUB -n $(($NODES * $PTILE))
	#BSUB -R "span[ptile=$PTILE]"
	#BSUB -o $P_OUTPUT/job/%J.log
	#BSUB -e $P_OUTPUT/job/%J.log
	#BSUB -W 10:00
	#BSUB -cwd $P_OUTPUT
	#BSUB -P SSSG0001

	. /etc/profile.d/modules.sh
	module purge
	export OMP_NUM_THREADS=1
	export PATH="/opt/ibmhpc/pecurrent/base/bin:${PATH}"
	
	poe $P_HPCG/xhpcg
EOF

#http://community.mellanox.com/groups/hpc/blog/2013/10/28/some-notes-for-using-connect-ib-with-intel-mpi

##!/bin/bash
##BSUB -P SSSG0001
##BSUB -W 05:00
##BSUB -R "span[ptile=16]" #one per node
##BSUB -n 512
##BSUB -q special
##BSUB -J test #name
##BSUB -x #exclusive
#
##module purge
#
#export PATH=/opt/ibmhpc/pecurrent/base/bin:$PATH
##. /ncar/opt/intel/psxe-2015/bin/compilervars.sh intel64
##. /ncar/opt/intel/impi/4.1.0.030/bin64/mpivars.sh intel64
#export MP_COMPILER=intel
#export MP_EUILIB=us
#export MP_PE_AFFINITY=yes
#
#echo $LD_LIBRARY_PATH
#echo $PATH
#
##ldd ./xhpcg
##mpirun -env LD_LIBRARY_PATH $LD_LIBRARY_PATH -hostfile $LSB_DJOB_HOSTFILE -n $LSB_DJOB_NUMPROC ldd ./xhpcg
##mpirun -env LD_LIBRARY_PATH  $LD_LIBRARY_PATH -hostfile $LSB_DJOB_HOSTFILE -n $LSB_DJOB_NUMPROC env | grep LD_LIBRARY_PATH
#echo poe ./xhpcg
#/bin/bash

