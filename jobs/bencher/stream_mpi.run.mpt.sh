#!/bin/bash
[ -z "$INSTALL_PATH" ] && echo 'load cluster environment first' && exit 1
[ -z "$1" ] && echo "$0 {path to steam_mpi}" && exit 1

export P_STREAM="$(readlink -f $1)"
mkdir -m 0770 -p "$OUTPUT_PATH"

cat <<-EOF
#!/bin/bash -l
#PBS -S /bin/bash
#PBS -N stream_mpi
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

mpiexec_mpt -v $P_STREAM
EOF

