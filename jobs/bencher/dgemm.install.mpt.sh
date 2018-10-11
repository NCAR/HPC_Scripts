#!/bin/bash
ABSPATH="$(dirname "$(readlink -f "$0")")" #path to this script

mpif90 -O3 -mavx -xhost -mkl -qopenmp $ABSPATH/src/do_dgemm_mpi.f -o dgemm_mpi

