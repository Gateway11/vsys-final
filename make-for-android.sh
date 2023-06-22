#!/usr/bin/env bash

cd 3rd-party

if [ ! -d blis ]; then
    git clone git@github.com:flame/blis.git
    configure auto

    mkdir armeabi-v7a/blis-new
    find blis -name *.h -exec cp {} ./armeabi-v7a/blis-new \;
    find ./armeabi-v7a/blis-new -name "test*" | xargs rm -rf
fi

if [ ! -d fftw-3.3.10 ]; then
    wget https://www.fftw.org/fftw-3.3.10.tar.gz
    tar -zxvf fftw-3.3.10.tar.gz

    cd fftw-3.3.10
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE=../../../../toolbox/ndk-r21/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=armeabi-v7a \
        -DANDROID_PLATFORM=android-29

    cd ../..
fi

cd ../jni

ndk-build
