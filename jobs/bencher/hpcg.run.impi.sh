#!/bin/bash
[ -z "$BENCHER" ] && echo 'import cluster profile from bencher first!' && exit 1
. $BENCHER/setup.intel.impi.env.sh

[ -z "$1" -o -z "$2" -o -z "$3" -o -z "$4" ] && echo $0 '{path to hpcg dir} {path to output dir} {ptile} {nodes}' && exit 1


P_HPCG="$(readlink -e "$1")"
P_OUTPUT="$2"
PTILE="$3"
NODES="$4"

mkdir -p $P_OUTPUT/job/

#create job batch file
cat <<-EOF
	#!/bin/bash
	#BSUB -q $QUEUE
	#BSUB -x
	#BSUB -J hpcg
	#BSUB -n $(($NODES * $PTILE))
	#BSUB -R "span[ptile=$PTILE]"
	#BSUB -o $P_OUTPUT/job/%J.log
	#BSUB -e $P_OUTPUT/job/%J.log
	#BSUB -W 10:00
	#BSUB -cwd $P_OUTPUT
	#BSUB -P SSSG0001

	. /etc/profile.d/modules.sh
	module purge
	. $BENCHER/setup.intel.impi.env.sh

	export I_MPI_FABRIC=shm:ofa
	#export I_MPI_DEVICE=rdma
	export I_MPI_FALLBACK=0
	export MV2_USE_APM=0
	export I_MPI_OFA_ADAPTER_NAME=mlx4_0
	export I_MPI_OFA_NUM_PORTS=1 
	export OMP_NUM_THREADS=1
	export KMP_AFFINITY='granularity=fine,compact,1,0'
	export INTELMPI_TOP=$P_IMPI
	export PATH=\$INTELMPI_TOP/intel64/bin/:\$PATH
	export I_MPI_HYDRA_BOOTSTRAP=lsf
	export I_MPI_HYDRA_BRANCH_COUNT=$NODES #64 is number of hosts, i.e., 1024/16
	export I_MPI_LSF_USE_COLLECTIVE_LAUNCH=1
	#mpiexec.hydra $P_HPCG/xhpcg
	#mpirun -IB $P_HPCG/xhpcg
	
	mpirun -IB -n \$LSB_DJOB_NUMPROC -perhost $PTILE -hostfile \$LSB_DJOB_HOSTFILE -print-rank-map -binding -rmk=lsf -ckpoint=off -print-all-exitcodes $P_HPCG/xhpcg
EOF

