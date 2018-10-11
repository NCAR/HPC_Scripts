#!/bin/bash

export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc MP_COMPILER=icc
export PATH=/ncar//ssg/make/4.1/bin/:$PATH
. $P_INTELCC/bin/compilervars.sh intel64
. $P_IMPI/bin64/mpivars.sh intel64
#. $P_INTELCC/mkl/bin/mklvars.sh intel64 

##export TOPdir=~/cc_mp_linpack 
##export INCdir=$TOPdir/include
##export BINdir=$TOPdir/bin
##export LIBdir=$TOPdir/lib
#export MPdir=$P_IMPI/
#export MPinc="-I$P_IMPI/include64/"
##export MKLINCDIR=$MPinc
#export MPlib="-L$P_IMPI/lib64/"
#export LAdir=$P_INTELCC/mkl/lib/intel64/
#export LAinc=$P_INTELCC/mkl/include/
##export LAlib="$LAdir/libmkl_intel_lp64.a $LAdir/libmkl_intel_thread.a $LAdir/libmkl_core.a $LAdir/libmkl_blacs_intelmpi_lp64.a $LAdir/libmkl_core.a $LAdir/libmkl_sequential.a $LAdir/libmkl_intel_thread.a $LAdir/libmkl_cdft_core.a -lpthread"
#export LAlib="$LAdir/libmkl_intel_lp64.a $LAdir/libmkl_sequential.a $LAdir/libmkl_core.a -lpthread -ldl"
#export CLAGS="$CFLAGS -O3 -ipo -mavx -xhost -no-prec-div -fp-model fast=2 "

#mkdir -p $TOPdir $INCdir $LIBdir

#make arch=test1 clean
rm -Rf hpl-2.1
tar -xf hpl-2.1.tar.gz

(
	cd hpl-2.1
	cp ~/bencher/mplinpack/Make.test1 .
	make arch=test1 install
)

