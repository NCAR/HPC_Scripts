#!/bin/bash

ABSPATH="$(dirname "$(readlink -f "$0")")" #path to this script
export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc MP_COMPILER=icc
. $P_INTELCC/bin/compilervars.sh intel64
. $P_IMPI/bin64/mpivars.sh intel64

mpicc -cc=icc \
	-O3 -mavx -xhost -no-prec-div -fno-alias -fp-model fast=2 -DMPICH_SKIP_MPICXX \
	-ffreestanding -openmp -mcmodel=medium -restrict -opt-streaming-stores always  \
	$ABSPATH/src/mpi-io-test.c -o mpi-io-test

