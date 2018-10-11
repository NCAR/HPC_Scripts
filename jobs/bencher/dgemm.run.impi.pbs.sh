#!/bin/bash
[ -z "$BENCHER" ] && echo 'import cluster profile from bencher first!' && exit 1
[ -z "$1" ] && echo $0 '{path to dgemm}' && exit 1

P_DGEMM="$(readlink -e "$1")"

#create job batch file
cat <<-EOF
#!/bin/bash -l
#PBS -S /bin/bash
#PBS -N dgemm
#PBS -j oe
#PBS -m n
#PBS -o $OUTPUT_PATH/
#PBS -V
#PBS -l select=$NODES:ncpus=72:mpiprocs=72
#PBS -l select=group=switch
#PBS -l place=scatter:exclhost
#PBS -l walltime=0:60:00
#PBS -r n
#PBS -q TuHTNoHPall
cd $PBS_O_WORKDIR
ldd $P_DGEMM

export I_MPI_FABRIC=shm:ofa
#export I_MPI_DEVICE=rdma
export I_MPI_FALLBACK=0
export MV2_USE_APM=0
export I_MPI_OFA_ADAPTER_NAME=mlx4_0
export I_MPI_OFA_NUM_PORTS=1 
export OMP_NUM_THREADS=1
export KMP_AFFINITY='granularity=fine,compact,1,0'
#export KMP_AFFINITY='granularity=fine,compact,1,0,verbose'
export INTELMPI_TOP=$P_IMPI
export PATH=\$INTELMPI_TOP/intel64/bin/:\$PATH
export I_MPI_HYDRA_BOOTSTRAP=pbsdsh
export I_MPI_HYDRA_BRANCH_COUNT=1 #64 is number of hosts, i.e., 1024/16
export I_MPI_LSF_USE_COLLECTIVE_LAUNCH=1
export OMP_NUM_THREADS=16

mpirun $P_DGEMM >> $OUTPUT_PATH/dgemm.\$(hostname -s).\$USER.log
EOF

