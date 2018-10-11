#!/bin/bash

ABSPATH="$(dirname "$(readlink -f "$0")")" #path to this script
export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc MP_COMPILER=icc
. $P_INTELCC/bin/compilervars.sh intel64
. $BENCHER/setup.intel.impi.env.sh

#mpiifort -O3 -mavx -xhost -mkl -no-openmp $ABSPATH/src/do_dgemm_mpi.f -o dgemm_mpi
mpiifort -O3 -mavx -xhost -mkl -openmp $ABSPATH/src/do_dgemm_mpi.f -o dgemm_mpi

