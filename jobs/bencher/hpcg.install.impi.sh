#!/bin/bash
[ -z "$BENCHER" ] && echo 'import cluster profile from bencher first!' && exit 1
. $BENCHER/setup.intel.impi.env.sh

mkdir build
cd build
cp ~/bencher/hpcg/Make.IntelMPI ../setup/
../configure IntelMPI && make 
