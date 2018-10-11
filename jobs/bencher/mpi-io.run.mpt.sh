#!/bin/bash
[ -z "$INSTALL_PATH" ] && echo 'load cluster environment first' && exit 1
[ -z "$1" -o -z "$2" ] && echo "$0 {path to mpi-io-test} {path to scratch file (will append random numbers)}" && exit 1

export P_IO="$(readlink -f $1)"
export P_SCRATCH="$2"

mkdir -m 0770 -p "$OUTPUT_PATH"

cat <<-EOF
#!/bin/bash -l
#PBS -S /bin/bash
#PBS -N mpi-io
#PBS -j oe
#PBS -m n
#PBS -o $OUTPUT_PATH/
#PBS -V
#PBS -l select=$NODES:ncpus=72:mpiprocs=72
#PBS -l select=group=switch
#PBS -l place=scatter:exclhost
#PBS -l walltime=0:60:00
#PBS -r y
#PBS -q TuHTNoHPall
cd $PBS_O_WORKDIR
ldd $P_IO

export P_SCRATCH="$P_SCRATCH.\$RANDOM.\$RANDOM.\$(date +%s).\$(hostname -s)"
mkdir -p \$P_SCRATCH 
cd \$P_SCRATCH 

mpiexec_mpt -v $P_IO -b $(($NODES*1*1024*1024)) -i 500 

unlink \$P_SCRATCH/test.out
rmdir \$P_SCRATCH
EOF

