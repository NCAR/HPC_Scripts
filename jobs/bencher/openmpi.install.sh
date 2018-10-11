#!/bin/bash
[ -z "$1" ] && echo "$0 {install path}" && exit 1
. /etc/profile.d/modules.sh
module purge
. $P_INTELCC/bin/iccvars.sh intel64 lp64
export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc MP_COMPILER=icc
export PDSH_EXE=/ncar/ssg/pdsh/current/bin/pdsh

make distclean
./configure --prefix="$1" && make clean && make -j4 && make install

