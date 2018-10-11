#!/bin/bash

export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc MP_COMPILER=icc I_MPI_CXX=icc
. $P_INTELCC/bin/iccvars.sh intel64
. $P_IMPI/bin64/mpivars.sh intel64
. $P_INTELCC/mkl/bin/mklvars.sh intel64

export CLAGS="$CFLAGS -O3 -ipo -mavx -xhost -no-prec-div -fp-model fast=2 "
export CXXLAGS="$CXXFLAGS -O3 -ipo -mavx -xhost -no-prec-div -fp-model fast=2 -DMPICH_SKIP_MPICXX "

