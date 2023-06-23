#!/usr/bin/env bash

cd 3rd-party

:<<EOF
if [ ! -d blis ]; then
    git clone git@github.com:flame/blis.git
    configure auto

    mkdir armeabi-v7a/blis-new
    find blis -name *.h -exec cp {} ./armeabi-v7a/blis-new \;
    find ./armeabi-v7a/blis-new -name "test*" | xargs rm -rf

    cd blis
    export NDK=../../../toolbox/ndk-r21
    export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64
    
    ./configure CC=$TOOLCHAIN/bin/clang AR=$TOOLCHAIN/bin/aarch64-linux-android-ar arm64 #ARMV7
    make V=1 \
        TARGET=CORTEXA57 \
        ONLY_CBLAS=1 \
        CC=$TOOLCHAIN/bin/aarch64-linux-android29-clang \
        AR=$TOOLCHAIN/bin/aarch64-linux-android-ar \
        HOSTCC=gcc \
        CFLAGS=-D__ANDROID_API__=29 \
        -j4

    cd ..
fi
EOF

if [ ! -d fftw-3.3.10 ]; then
    wget https://www.fftw.org/fftw-3.3.10.tar.gz
    tar -zxvf fftw-3.3.10.tar.gz
    rm *.tar.gz

    cd fftw-3.3.10
    sed -i '' s/single-precision\"\ OFF/single-precision\"\ ON/g `grep "single-precision\"" -rl  ./`

    mkdir build
    cd build
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE=../../../../toolbox/ndk-r21/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=armeabi-v7a \
        -DANDROID_PLATFORM=android-29
    #make -j8

    cd ../..
fi

cd ../jni

ndk-build
