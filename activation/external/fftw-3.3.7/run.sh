#!/bin/bash

wget http://www.fftw.org/fftw-3.3.7.tar.gz

tar -xvzf fftw-3.3.7.tar.gz -C `pwd`

cd fftw-3.3.7

./configure --prefix=/Users/daixiang/Desktop/vsys/external --enable-shared --enable-float --disable-fortran

make -j 8

make install

