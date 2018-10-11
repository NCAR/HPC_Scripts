#!/bin/bash

export CC=icc CXX=icpc F77=ifort FC=ifort 
. $P_INTELCC/bin/iccvars.sh intel64
. $P_INTELCC/mkl/bin/mklvars.sh intel64
export PATH=$P_MVAPICH:$PATH
export LD_LIBRARY_PATH=$P_MVAPICH/lib:$LD_LIBRARY_PATH

export CLAGS="$CFLAGS -O3 -ipo -mavx -xhost -no-prec-div -fp-model fast=2 "
export CXXLAGS="$CXXFLAGS -O3 -ipo -mavx -xhost -no-prec-div -fp-model fast=2 -DMPICH_SKIP_MPICXX "

