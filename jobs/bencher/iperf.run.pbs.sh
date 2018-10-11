#!/bin/bash
[ -z "$BENCHER" ] && echo 'import cluster profile from bencher first!' && exit 1
[ -z "$1" ] && echo $0 '{path to iperf3}' && exit 1

P_IPERF="$(readlink -e "$1")"

#create job batch file
cat <<-EOF
#!/bin/bash -l
#PBS -S /bin/bash
#PBS -N iperf3
#PBS -j oe
#PBS -o $OUTPUT_PATH/
#PBS -m n
#PBS -V
#PBS -l select=2:ncpus=2:mpiprocs=1
#PBS -l select=group=switch
#PBS -l place=scatter:exclhost
#PBS -l walltime=0:20:00
#PBS -r n
#PBS -q TuHTNoHPall
export MPI_SHEPHERD=true
cd \$PBS_O_WORKDIR
ldd $(which $P_IPERF)
export LD_LIBRARY_PATH=~/apps/lib/:/opt/sgi/mpt/mpt-2.14/lib/:\$LD_LIBRARY_PATH
export PATH=/opt/sgi/mpt/mpt-2.14/bin/:\$PATH

#s=\$(cat \$PBS_NODEFILE | head -1)
#c=\$(cat \$PBS_NODEFILE | tail -1)
#echo "server: \$s"
#echo "client: \$c"

#ssh -n -o batchmode=true \$s $P_IPERF -s  &
#sleep 5
#$P_IPERF -c \$s -t 60

mpiexec_mpt bash -c "hostname;
[ \\\$MPT_MPI_RANK -eq 0 ] && $P_IPERF -s;
[ \\\$MPT_MPI_RANK -ne 0 ] && for i in {1..15}; do sleep 15; $P_IPERF -c \\\$REMOTEHOST -t 300; done;
"

EOF

