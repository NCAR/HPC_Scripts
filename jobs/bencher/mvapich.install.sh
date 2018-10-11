#!/bin/bash

make distclean
./configure --prefix="$ABSIPATH" && make clean && make -j36 && make install

