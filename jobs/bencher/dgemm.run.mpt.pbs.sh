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
#PBS -J 1-$NODES
#PBS -m n
#PBS -o $OUTPUT_PATH/
#PBS -V
#PBS -l select=1:ncpus=72:mpiprocs=72
#PBS -l select=group=switch
#PBS -l place=scatter:exclhost
#PBS -l walltime=0:60:00
#PBS -r y
#PBS -q TuHTNoHPall
cd $PBS_O_WORKDIR
ldd $P_DGEMM

mpiexec_mpt -v $P_DGEMM >> $OUTPUT_PATH/dgemm.\$(hostname -s).\$USER.\$(date +%s).log
EOF

