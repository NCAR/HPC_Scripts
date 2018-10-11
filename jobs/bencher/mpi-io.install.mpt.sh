#!/bin/bash

ABSPATH="$(dirname "$(readlink -f "$0")")" #path to this script

mpicc -cc=icc \
	-O3 -mavx -xhost -no-prec-div -fno-alias -fp-model fast=2 -DMPICH_SKIP_MPICXX \
	-ffreestanding -openmp -mcmodel=medium -restrict -opt-streaming-stores always  \
	$ABSPATH/src/mpi-io-test.c -o mpi-io-test

