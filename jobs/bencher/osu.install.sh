#!/bin/bash
[ -z "$1" ] && echo "$0 {install path}" && exit 1

#P_IMPI=/ncar/opt/intel/2013/impi/4.1.3.048/
#P_PMPI=/ncar/opt/platform_mpi/9.1.2/
#P_POE=/opt/ibmhpc/pecurrent/base/
#P_MVAPICH=/picnic/u/home/ssgadmin/mvapich/2-2.0/

. /etc/profile.d/modules.sh
module purge
. $P_INTELCC/bin/iccvars.sh intel64 lp64
export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc MP_COMPILER=icc

path="$1"
mkdir -p $path
path=$(readlink -e $path)

#http://mvapich.cse.ohio-state.edu/download/mvapich/osu-micro-benchmarks-4.4.tar.gz
# code change required to work around missing def of WIN_ALLOCATE
#--- osu-micro-benchmarks-4.4/./mpi/one-sided/osu_1sc.c  2014-09-03 11:30:22.231685613 -0600
#+++ osu-micro-benchmarks-4.4/./mpi/one-sided/osu_1sc.c   2014-08-23 20:15:58.000000000 -0600
#@@ -304,11 +304,11 @@
#             return po_bad_usage;
#         }
# 
#-//       if ((options.rank0 == 'D' || options.rank1 == 'D') 
#-//           && *win == WIN_ALLOCATE)
#-//       {
#-//           *win = WIN_CREATE;
#-//       }
#+        if ((options.rank0 == 'D' || options.rank1 == 'D') 
#+            && *win == WIN_ALLOCATE)
#+        {
#+            *win = WIN_CREATE;
#+        }
#     }
# 
#     return po_okay;

function installit {
	make distclean
	./configure --prefix=$1 CC=$2 && make clean && make && make install
}

( #impi  https://software.intel.com/sites/products/documentation/hpc/mpi/linux/getting_started.pdf
	. $P_IMPI/bin64/mpivars.sh
	installit $path/impi $(which mpiicc)
)
( #pmpi
	export PATH=$P_PMPI/bin:$PATH
	export CFLAGS="$CFLAGS -L$P_PMPI/lib/linux_amd64 -I$P_PMPI/include/ -Wl,-rpath,$P_PMPI/lib/linux_amd64"
	#export LDFLAGS="$LDFLAGS -L$P_PMPI/lib/linux_amd64 "
	installit $path/pmpi $(which mpicc)
)
( #poe
	export PATH=$P_POE/bin:$PATH
	#export CFLAGS="$CFLAGS -L$P_PMPI/lib/linux_amd64 -I$P_PMPI/include/ -Wl,-rpath,$P_PMPI/lib/linux_amd64"
	#export LDFLAGS="$LDFLAGS -L$P_PMPI/lib/linux_amd64 "
	installit $path/poe $(which mpicc)
)
( #mvapich
	export PATH=$P_MVAPICH/bin:$PATH
	export CFLAGS="$CFLAGS -L$P_MVAPICH/lib/ -I$P_MVAPICH/include/ -Wl,-rpath,$P_MVAPICH/lib/"
	installit $path/mvapich $(which mpicc)
)

#readelf -a |grep __intel_new_memcpy
