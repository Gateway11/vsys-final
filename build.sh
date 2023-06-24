#!/usr/bin/env bash

cd 3rd-party

if [ ! -d blis ]; then
    git clone git@github.com:flame/blis.git

    cd blis
    export NDK=../../../toolbox/ndk-r21
    export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64/bin
    
    ./configure CC=$TOOLCHAIN/clang AR=$TOOLCHAIN/aarch64-linux-android-ar RANLIB=$TOOLCHAIN/aarch64-linux-android-ranlib arm64 #ARMV7
    sed -i '' s/:=\ Darwin/:=\ Linux/g config.mk
    sed -i '' s/LIBPTHREAD/#LIBPTHREAD/g config.mk
    sed -i '' s/LDFLAGS\ +=/#LDFLAGS\ +=/g common.mk

    # What to do?
    sed -i '' s/\ F77_sdot_sub/\ \\/\\/F77_sdot_sub/g frame/compat/cblas/src/cblas_sdot.c
    sed -i '' s/\ F77_ddot_sub/\ \\/\\/F77_ddot_sub/g frame/compat/cblas/src/cblas_ddot.c
:<<EOF
    cd blis && make V=1 \
        TARGET=CORTEXA57 \
        ONLY_CBLAS=1 \
        CC=$TOOLCHAIN/aarch64-linux-android29-clang \
        AR=$TOOLCHAIN/aarch64-linux-android-ar \
        RANLIB=$TOOLCHAIN/aarch64-linux-android-ranlib \
        HOSTCC=gcc \
        CFLAGS=-D__ANDROID_API__=29 \
        -j8
    
    cd ..
EOF
    python ./build/flatten-headers.py -l -v1 ./frame/include/blis.h include/arm64/blis.h "./include" "./ ./config/arm64/ ./config/cortexa53/ ./config/cortexa57/ ./config/firestorm/ ./config/generic/ ./config/thunderx2/ ./kernels/armv8a/ ./kernels/armv8a/3/ ./frame/0/ ./frame/0/copysc/ ./frame/1/ ./frame/1d/ ./frame/1f/ ./frame/1m/ ./frame/1m/packm/ ./frame/1m/unpackm/ ./frame/2/ ./frame/2/gemv/ ./frame/2/ger/ ./frame/2/hemv/ ./frame/2/her/ ./frame/2/her2/ ./frame/2/symv/ ./frame/2/syr/ ./frame/2/syr2/ ./frame/2/trmv/ ./frame/2/trsv/ ./frame/3/ ./frame/3/gemm/ ./frame/3/gemm/ind/ ./frame/3/gemmt/ ./frame/3/hemm/ ./frame/3/symm/ ./frame/3/trmm/ ./frame/3/trmm3/ ./frame/3/trsm/ ./frame/base/ ./frame/base/cast/ ./frame/base/check/ ./frame/base/noopt/ ./frame/base/proj/ ./frame/compat/ ./frame/compat/blis/thread/ ./frame/compat/cblas/ ./frame/compat/cblas/f77_sub/ ./frame/compat/cblas/src/ ./frame/compat/check/ ./frame/compat/extra/ ./frame/compat/f2c/ ./frame/compat/f2c/util/ ./frame/include/ ./frame/include/level0/ ./frame/include/level0/1e/ ./frame/include/level0/1m/ ./frame/include/level0/1r/ ./frame/include/level0/bb/ ./frame/include/level0/ri/ ./frame/thread/ ./frame/util/"

    cd ..
fi

if [ ! -d fftw-3.3.10 ]; then
    wget https://www.fftw.org/fftw-3.3.10.tar.gz
    tar -zxvf fftw-3.3.10.tar.gz
    rm *.tar.gz

    cd fftw-3.3.10
    sed -i '' s/single-precision\"\ OFF/single-precision\"\ ON/g CMakeLists.txt

    mkdir build && cd build
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE=../../../../toolbox/ndk-r21/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=armeabi-v7a \
        -DANDROID_PLATFORM=android-29
    
    #make -j8
    cd ../..
fi

cd ../jni

ndk-build
