#!/bin/bash
[ -z "$INSTALL_PATH" ] && echo 'load cluster environment first' && exit 1

#Get real path
ABSPATH="$(dirname "$(readlink -f "$0")")" #path to this script
ABSICPATH="$(readlink -f "$INSTALL_PATH")/common/" #path to install of common apps

export PATH=$ABSIPATH/cmake/bin:$PATH

###ensure paths exist
if [ ! -f $ABSIPATH/cmake/bin/cmake ]
then
	rm -Rf $BUILD_PATH
	mkdir -m0700 -p $INSTALL_PATH $BUILD_PATH
	( #need latest cmake but RHEL6.4 is way too old
		cd $ABSPATH/cmake
		cmake -H$ABSPATH/cmake -B$ABSBPATH -DCMAKE_INSTALL_PREFIX="$ABSICPATH/" -DCMAKE_VERSION="$CMAKE_VERSION" -DCMAKE_GIT_URL="$CMAKE_GIT_URL"
		cd $ABSBPATH && make
	)
	rm -Rf $BUILD_PATH
	(
		cd $ABSPATH/pdsh
		cmake -H$ABSPATH/cmake -B$ABSBPATH -DCMAKE_INSTALL_PREFIX="$ABSICPATH/" -DPDSH_VERSION="$PDSH_VERSION" -DPDSH_GIT_URL="$PDSH_GIT_URL"
 		cd $ABSBPATH && make
	)
fi

#switch to Intel CC
#export CFLAGS="-mmic" CXXFLAGS="-mmic"
#export LDFLAGS="-L$P_INTELCC/composerxe/lib/mic/ -Wl,-rpath,$P_INTELCC/composerxe/lib/mic/"
export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc MP_COMPILER=icc
. $P_INTELCC/bin/iccvars.sh intel64 lp64
#. $P_IMPI/bin64/mpivars.sh

#build mvapich using intel
if [ ! -f $ABSIPATH/mvapich/bin/mpirun ]
then
	( 	
		rm -Rf $BUILD_PATH
		cmake -H$ABSPATH -B$ABSBPATH -DCMAKE_INSTALL_PREFIX="$ABSIPATH/mvapich/" -DMVAPICH_URL="$MVAPICH_URL" 
		cd $ABSBPATH && make
	)
fi

#build hpcg

