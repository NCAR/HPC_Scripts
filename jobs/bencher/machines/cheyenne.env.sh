#!/bin/bash

. /etc/profile.d/modules.sh
module purge

export BENCHER=~/bencher/
export QUEUE=regular
export RUNLOOPS=${RUNLOOPS:-32}
export NODES=${NODES:-32}
export PTILE=${PTILE:-16}
export CPUS=$(($NODES * $PTILE))
export RESREQ=""
export JOBNAME="test benchmark" 
export WCTIME="12:00" #wallclock time
export PROJ="SSSG0001"
export OMP_NUM_THREADS=${OMP_NUM_THREADS:-1}

export P_MPT=/glade/u/apps/opt/mpt/2.15-sgi715a158/
export P_INTELCC=/glade/u/apps/opt/intel/2018u1/
export P_MPLINPACK=$P_INTELCC//composerxe/mkl/benchmarks/mp_linpack/
export P_IMPI=/glade/u/apps/opt/intel/psxe-2016_update1/impi/5.1.2.150/
export P_MVAPICH=${P_MVAPICH:-/glade/scratch/ssgadmin/mvapich/2-2.3b/}
export PATH=~/bin/:~/build/bin/:$PATH
export LM_LICENSE_FILE=28518@128.117.177.41,28518@128.117.183.213,28518@128.117.183.214,28518@128.117.183.215

#mpi environment
declare -A mpiV
mpiV[impi]=". $P_IMPI/bin64/mpivars.sh;"
mpiV[mvapich]="export PATH=$P_MVAPICH/bin:\$PATH RSH_CMD=/usr/bin/ssh"

declare -A mpiR 
mpiR[impi]="mpirun -n \$LSB_DJOB_NUMPROC -hostfile \$LSB_DJOB_HOSTFILE"
mpiR[mvapich]="mpirun_rsh -ssh -np \$LSB_DJOB_NUMPROC -hostfile \$LSB_DJOB_HOSTFILE"

export mpiV mpiR

#need latest GCC since latest images dont have full install
#export PATH=/ncar/ssg/gcc/current/bin/:$PATH
#export LD_LIBARY_PATH=/ncar/ssg/gcc/current/lib64/:/ncar/ssg/gcc/current/lib/:$LD_LIBARY_PATH
#export CFLAGS="$CFLAGS -L/ncar/ssg/gcc/current/lib/ -L/ncar/ssg/gcc/current/lib64/ -I/picnic/u/home/ssgadmin/src/include/ "

export INSTALL_PATH=${INSTAL_PATH:=~/test/}
export BUILD_PATH=${BUILD_PATH:=~/build/}
export OUTPUT_PATH=${OUTPUT_PATH:=~/output/}
export ABSBPATH="$(readlink -f "$BUILD_PATH")" #path to build
export ABSIPATH="$(readlink -f "$INSTALL_PATH")" #path to install root
 

export CMAKE_VERSION="v3.6.2"
export CMAKE_GIT_URL="git://cmake.org/cmake.git"

export PDSH_VERSION="pdsh-2.31"
export PDSH_GIT_URL="https://github.com/grondo/pdsh/"

export MVAPICH_URL='http://mvapich.cse.ohio-state.edu/download/mvapich/mv2/mvapich2-2.2.tar.gz'
export GCC_URL='http://mirrors-usa.go-parts.com/gcc/releases/gcc-6.2.0/gcc-6.2.0.tar.gz'

export HPCG_SVN_URL="http://www.hpcg-benchmark.org/downloads/hpcg-3.0.tar.gz" 

#INTEL_LINPACK_URL="http://registrationcenter.intel.com/irc_nas/4547/l_lpk_p_11.2.0.003.tgz"




