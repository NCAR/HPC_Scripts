#!/bin/bash
#export CC=icc CXX=icpc F77=ifort FC=ifort MPI_CC=icc OMPI_CC=icc MP_COMPILER=icc
#. $P_INTELCC/bin/iccvars.sh intel64 lp64
#. $P_IMPI/bin64/mpivars.sh intel64
export PATH="/opt/ibmhpc/pecurrent/base/bin/:${PATH}"

#mpiR[impi]="mpirun -n \$LSB_DJOB_NUMPROC -hostfile \$LSB_DJOB_HOSTFILE"
../configure POE && make 
