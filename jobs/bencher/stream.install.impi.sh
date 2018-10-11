#!/bin/bash

ABSPATH="$(dirname "$(readlink -f "$0")")" #path to this script
export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc MP_COMPILER=icc
. $P_INTELCC/bin/compilervars.sh intel64

AS=$((1111000000 * 1))
mpicc -cc=icc \
	-O3 -xhost -no-prec-div -fno-alias -fp-model fast=2 -DMPICH_SKIP_MPICXX \
	-ffreestanding -openmp -mcmodel=medium -restrict -opt-streaming-stores always  \
	-DSTREAM_ARRAY_SIZE=$AS -DNTIMES=$RUNLOOPS -DVERBOSE   \
	$ABSPATH/src/stream.c -o stream

