cmake_minimum_required(VERSION 2.6)
project(mvapich)

set(MVAPICH_URL "http://mvapich.cse.ohio-state.edu/download/mvapich/mv2/mvapich2-2.1a.tar.gz" CACHE STRING "tarball URL for mvapich")

include(ExternalProject)

ExternalProject_Add(
   mvapich
   URL ${MVAPICH_URL}
   PREFIX ${CMAKE_CURRENT_BINARY_DIR}/
   #CONFIGURE_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/configure --prefix=${CMAKE_INSTALL_PREFIX} --with-ssh --without-rsh --with-readline"
   CONFIGURE_COMMAND ${CMAKE_BINARY_DIR}/mvapich/src/mvapich/configure --prefix=${CMAKE_INSTALL_PREFIX}

   #BUILD_COMMAND make
)

#. $P_INTELCC/bin/iccvars.sh intel64 lp64
#export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc
#MP_COMPILER=icc
#export PDSH_EXE=/ncar/ssg/pdsh/current/bin/pdsh
#
#make distclean
#./configure --prefix="$1" && make clean && make -j4 && make install


