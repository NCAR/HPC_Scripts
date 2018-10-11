#!/bin/bash

ABSPATH="$(dirname "$(readlink -f "$0")")" #path to this script

mpicc -cc=icc \
	-O3 -mavx -xhost -no-prec-div -fno-alias -fp-model fast=2 -DMPICH_SKIP_MPICXX \
	-ffreestanding -openmp -mcmodel=medium -restrict -opt-streaming-stores always  \
	-DSTREAM_ARRAY_SIZE=$((1110000000 * $NODES)) -DNTIMES=$RUNLOOPS -DVERBOSE   \
	$ABSPATH/src/stream_mpi.c -o stream_mpi

