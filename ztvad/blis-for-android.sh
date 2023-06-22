export NDK=../../toolbox/ndk-r21
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64

#https://github.com/xianyi/OpenBLAS/wiki/How-to-build-OpenBLAS-for-Android

make clean
make V=1\
    TARGET=CORTEXA57 \
    ONLY_CBLAS=1 \
    CC=$TOOLCHAIN/bin/aarch64-linux-android29-clang \
    AR=$TOOLCHAIN/bin/aarch64-linux-android-ar \
    HOSTCC=gcc \
    CFLAGS=-D__ANDROID_API__=29 \
    -j4
sudo make install
