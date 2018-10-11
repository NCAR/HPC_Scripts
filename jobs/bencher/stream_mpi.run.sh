#!/bin/bash
[ -z "$INSTALL_PATH" ] && echo 'load cluster environment first' && exit 1
[ -z "$1" ] && echo "$0 {path to steam_mpi}" && exit 1

export P_STREAM="$(readlink -f $1)"

mkdir -m 0770 -p "$OUTPUT_PATH/job/"

echo $OMP_NUM_THREADS

#create job batch file
cat <<-EOF
	#!/bin/bash
	#BSUB -q $QUEUE
	#BSUB -x
	#BSUB -J stream_mpi.$2
	#BSUB -n $CPUS
	#BSUB -R "span[ptile=$PTILE]"
	#BSUB -o $OUTPUT_PATH/job/%J.log
	#BSUB -e $OUTPUT_PATH/job/%J.log
	#BSUB -W $WCTIME
	#BSUB -cwd "$OUTPUT_PATH"
	#BSUB -P SSSG0001
	
	export I_MPI_EAGER_THRESHOLD=128000
	#          This setting may give 1-2% of performance increase over the
	#          default value of 262000 for large problems and high number of cores
	
	export OMP_NUM_THREADS=$OMP_NUM_THREADS
	#export I_MPI_FABRICS=shm:ofa
	#export I_MPI_FALLBACK=0
	#export I_MPI_OFA_DYNAMIC_QPS=1
	#export I_MPI_OFA_PACKET_SIZE=32800
	. $P_INTELCC/bin/compilervars.sh intel64
	. $P_IMPI/bin64/mpivars.sh intel64
	
	export I_MPI_FABRIC=shm:ofa
	export I_MPI_FALLBACK=disable
	export MV2_USE_APM=0
	export I_MPI_OFA_ADAPTER_NAME=mlx4_0
	export I_MPI_OFA_NUM_PORTS=1 
	export KMP_AFFINITY='granularity=fine,compact,1,0'
	export INTELMPI_TOP=$P_IMPI
	export PATH=\$INTELMPI_TOP/intel64/bin/:\$PATH
	export I_MPI_HYDRA_BOOTSTRAP=lsf
	export I_MPI_HYDRA_BRANCH_COUNT=$NODES #64 is number of hosts, i.e., 1024/16
	export I_MPI_LSF_USE_COLLECTIVE_LAUNCH=1

	pwd
	echo $P_STREAM
	ldd $P_STREAM

	mpiexec.hydra $P_STREAM
EOF

#readelf -a |grep __intel_new_memcpy
