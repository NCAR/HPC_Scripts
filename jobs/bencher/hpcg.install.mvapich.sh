#!/bin/bash
#export LM_LICENSE_FILE=28518@128.117.177.41,28518@128.117.183.213,28518@128.117.183.214,28518@128.117.183.215
#export P_INTELCC=/ncar/opt/intel/psxe-2015/
#export P_IMPI=$P_INTELCC/impi_latest/
#[ -z "$P_MVAPICH" ] && export P_MVAPICH=/picnic/scratch/ssgadmin/mvapich/2-2.0.intel.opt/
export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc MP_COMPILER=icc
. $P_INTELCC/bin/iccvars.sh intel64 lp64
export PATH=$P_MVAPICH/bin/:$PATH
export LD_LIBARY_PATH=$P_MVAPICH/lib/:$P_INTELCC/compiler/lib/intel64/:/usr/lib64/:$LD_LIBARY_PATH

#mpiR[impi]="mpirun -n \$LSB_DJOB_NUMPROC -hostfile \$LSB_DJOB_HOSTFILE"
../configure intel.mvapich && make clean && make 
