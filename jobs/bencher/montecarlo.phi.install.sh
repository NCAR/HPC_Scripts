#!/bin/bash
#http://www.arc.vt.edu/resources/hpc/blueridge_mic.php
[ -z "$1" ] && echo "$0 {install path}" && exit 1
. /etc/profile.d/modules.sh
module purge
. $P_INTELCC/bin/iccvars.sh intel64 lp64
. $P_IMPI/bin64/mpivars.sh

export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc MP_COMPILER=icc
export CFLAGS="-mmic" CXXFLAGS="-mmic"
export LDFLAGS="-L$P_INTELCC/composerxe/lib/mic/ -Wl,-rpath,$P_INTELCC/composerxe/lib/mic/"
export PDSH_EXE=/ncar/ssg/pdsh/current/bin/pdsh

SRC=/$(dirname "$(readlink -f "$0")")/src/

mkdir -p "$1"
mpiicc -mmic $SRC/montecarlo.c -o "$1/montecarlo.mic"
mpiicc $SRC/montecarlo.c -o "$1/montecarlo"
