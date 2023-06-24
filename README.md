# vsys-final

### blis in android

    https://github.com/xianyi/OpenBLAS/wiki/How-to-build-OpenBLAS-for-Android

    cd 3rd-party/blis

    export NDK=../../../toolbox/ndk-r21
    export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64/bin

    ./configure CC=$TOOLCHAIN/clang AR=$TOOLCHAIN/aarch64-linux-android-ar RANLIB=$TOOLCHAIN/aarch64-linux-android-ranlib --enable-cblas arm64 #ARMV7
    python ./build/flatten-headers.py -l -v1 xxxx // please see build.sh

    cd ../../jni

    mv Android-blis.mk Android.mk
    ndk-build
