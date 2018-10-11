#!/bin/bash
[ -z "$BENCHER" ] && echo 'import cluster profile from bencher first!' && exit 1
[ -z "$1" ] && echo $0 '{path to osu_mbw_mr}' && exit 1

P_OSU="$(readlink -e "$1")"

#create job batch file
cat <<-EOF
#!/bin/bash -l
#PBS -S /bin/bash
#PBS -N osu_mbw_mr
#PBS -j oe
#PBS -m n
#PBS -o $OUTPUT_PATH/
#PBS -V
#PBS -l select=$NODES:ncpus=72:mpiprocs=4
#PBS -l select=group=switch
#PBS -l place=scatter:exclhost
#PBS -l walltime=0:60:00
#PBS -r n
#PBS -q system
cd $PBS_O_WORKDIR
ldd $P_OSU

echo MPI_USE_XPMEM = $MPI_USE_XPMEM 
echo MPI_USE_OPA   = $MPI_USE_OPA   
echo MPI_USE_IB    = $MPI_USE_IB    
echo MPI_USE_UD    = $MPI_USE_UD    
echo MPI_USE_TCP   = $MPI_USE_TCP   

#for i in {1..150};
#do
mpiexec_mpt -v $P_OSU
#done

EOF

