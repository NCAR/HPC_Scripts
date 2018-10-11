#!/bin/bash
[ -z "$1" -o -z "$2" -o -z "$3"  ] && echo $0 '{run count} {path to montecarlo dir} {path to output dir} {resource requirement} {hosts} {cpu}' && exit 1

QUEUE=pronghorn
COUNT="$1"
P_JOB="$(readlink -e "$2")"
P_OUTPUT="$3"
RESREQ="$4"
NHOST="$5"
NCPU="$6"

P_INTELCC=${P_INTELCC:-/ncar/opt/intel/2013/}
P_IMPI=${P_IMPI:-/ncar/opt/intel/2013/impi/4.1.3.048/}

. /etc/profile.d/modules.sh
module purge

mkdir -p $P_OUTPUT/job/

[ ! -z "$RESREQ" ] && RESREQ="#BSUB -R \"$RESREQ\""

#create job batch file
cat <<-EOF
	#!/bin/bash
	#BSUB -q $QUEUE
	#BSUB -x
	##BSUB -J testbench[1-$COUNT]
	#BSUB -J testbench
	#BSUB -n $NHOST
	#BSUB -R "span[ptile=1]"
	##BSUB -o $P_OUTPUT/job/%J.log
	##BSUB -e $P_OUTPUT/job/%J.log
	#BSUB -W 0:30
	#BSUB -cwd $P_OUTPUT
	#BSUB -P SSSG0001
	$RESREQ
	
	. $P_IMPI/bin64/mpivars.sh
	export I_MPI_MIC=enable I_MPI_FABRICS_LIST=tcp I_MPI_MIC_POSTFIX=.mic I_MPI_TCP_NETMASK=10.12.203.0/24:brmic:mic0 HYDRA_HOSTNAME_PROPAGATION=1
	sed 's#-ib##g' \$LSB_DJOB_HOSTFILE | awk '{print \$1 "-mic-gw"; print \$1 "-mic0"; print \$1 "-mic1"}'  > $P_OUTPUT/job/\$LSB_JOBID.hosts
	set -x
	
	#awk '{print \$1}' < \$LSB_DJOB_HOSTFILE | sed 's#-ib##g' | sort -u | awk '{
	#    printf "%s-mic0:$NCPU ifhn=mic0\n", \$1
	#    printf "%s-mic1:$NCPU ifhn=mic0\n", \$1
	#    printf "%s-mic-gw:$NCPU ifhn=brmic\n", \$1
	#}' > /tmp/montecarlo.machinefile.\$\$
	awk '{print \$1}' < \$LSB_DJOB_HOSTFILE | sed 's#-ib##g' | sort -u | awk '{
	    printf "%s-mic0:$NCPU\n", \$1
	    printf "%s-mic1:$NCPU\n", \$1
	    printf "%s-mic-gw:$NCPU\n", \$1
	}' > $P_OUTPUT/job/\$LSB_JOBID.machinefile
	cat $P_OUTPUT/job/\$LSB_JOBID.machinefile
	
	mpirun -v -rmk user -bootstrap ssh -disable-x -hosts $P_OUTPUT/job/\$LSB_JOBID.hosts -machinefile $P_OUTPUT/job/\$LSB_JOBID.machinefile $P_JOB/montecarlo
	EOF

