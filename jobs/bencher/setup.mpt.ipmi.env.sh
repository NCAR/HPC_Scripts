#!/bin/bash

export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc MP_COMPILER=icc I_MPI_CXX=icc
. $P_INTELCC/bin/iccvars.sh intel64
. $P_INTELCC/mkl/bin/mklvars.sh intel64

export PATH=$P_MPT/bin:$PATH
export CFLAGS="$CFLAGS -I$P_MPT/include"
export LD_LIBRARY_PATH=$P_MPT/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$P_MPT/lib:$LD_LIBRARY_PATH
export MPI_ROOT=$P_MPT
export MPT_VERSION=$(basename $P_MPT)
export MANPATH=$P_MPT/man:$MANPATH

