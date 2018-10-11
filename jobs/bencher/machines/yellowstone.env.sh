#!/bin/bash

. /etc/profile.d/modules.sh
module purge

export BENCHER=~/bencher/
export QUEUE=special
export RUNLOOPS=${RUNLOOPS:-32}
export NODES=${NODES:-32}
export PTILE=${PTILE:-16}
export CPUS=$(($NODES * $PTILE))
export RESREQ=""
export JOBNAME="test benchmark" 
export WCTIME="12:00" #wallclock time
export PROJ="SSSG0001"
export OMP_NUM_THREADS=${OMP_NUM_THREADS:-1}

export P_INTELCC=/ncar//opt/intel/psxe-2016_update3/
export P_MPLINPACK=$P_INTELCC//composerxe/mkl/benchmarks/mp_linpack/
export P_IMPI=$P_INTELCC/impi_latest/
export P_PMPI=${P_PMPI:-/ncar/opt/platform_mpi/9.1.2/}
export P_POE=${P_POE:-/opt/ibmhpc/pecurrent/base/}
export P_MVAPICH=${P_MVAPICH:-/picnic/scratch/ssgadmin/mvapich/2-2.0/}
export PATH=/ncar/ssg/cmake/current/bin/:$PATH
export LM_LICENSE_FILE=28518@128.117.177.41,28518@128.117.183.213,28518@128.117.183.214,28518@128.117.183.215


#mpi environment
declare -A mpiV
mpiV[impi]=". $P_IMPI/bin64/mpivars.sh;"
mpiV[pmpi]="export PATH=$P_PMPI/bin:\$PATH MPI_ROOT=$P_PMPI"
[ "$USEPOE" = "yes" ] && mpiV[poe]="export PATH=$P_POE/bin:\$PATH TARGET_CPU_RANGE=\"-1\" LANG=en_US MP_PE_AFFINITY=no"
mpiV[mvapich]="export PATH=$P_MVAPICH/bin:\$PATH RSH_CMD=/usr/bin/ssh"

declare -A mpiR 
mpiR[impi]="mpirun -n \$LSB_DJOB_NUMPROC -hostfile \$LSB_DJOB_HOSTFILE"
mpiR[pmpi]="mpirun -lsf "
[ "$USEPOE" = "yes" ] && mpiR[poe]=poe
mpiR[mvapich]="mpirun_rsh -ssh -np \$LSB_DJOB_NUMPROC -hostfile \$LSB_DJOB_HOSTFILE"

export mpiV mpiR

#need latest GCC since latest images dont have full install
export PATH=/ncar/ssg/gcc/current/bin/:$PATH
export LD_LIBARY_PATH=/ncar/ssg/gcc/current/lib64/:/ncar/ssg/gcc/current/lib/:$LD_LIBARY_PATH
export CFLAGS="$CFLAGS -L/ncar/ssg/gcc/current/lib/ -L/ncar/ssg/gcc/current/lib64/ -I/picnic/u/home/ssgadmin/src/include/ "

export INSTALL_PATH=${INSTAL_PATH:=~/test/}
export BUILD_PATH=${BUILD_PATH:=~/build/}
export OUTPUT_PATH=${OUTPUT_PATH:=~/output/}

export CMAKE_VERSION="v3.0.2"
export CMAKE_GIT_URL="git://cmake.org/cmake.git"

export PDSH_VERSION="pdsh-2.29"
export PDSH_GIT_URL="https://code.google.com/p/pdsh/"

export MVAPICH_URL='http://mvapich.cse.ohio-state.edu/download/mvapich/mv2/mvapich2-2.1a.tar.gz'
export GCC_URL='http://mirrors-usa.go-parts.com/gcc/releases/gcc-4.9.1/gcc-4.9.1.tar.gz'

export HPCG_SVN_URL="https://software.sandia.gov/hpcg/downloads/hpcg-2.4.tar.gz" 

#INTEL_LINPACK_URL="http://registrationcenter.intel.com/irc_nas/4547/l_lpk_p_11.2.0.003.tgz"




