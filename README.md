# vsys-final

### blis in android
    
    NDK-25C

    https://github.com/xianyi/OpenBLAS/wiki/How-to-build-OpenBLAS-for-Android

    cd 3rd-party/blis

    export NDK_BUNDLE=../../../*/*
    export TOOLCHAIN=$NDK_BUNDLE/toolchains/llvm/prebuilt/$(uname | tr A-Z a-z)-x86_64/bin
    
    ./configure CC=$TOOLCHAIN/clang AR=$TOOLCHAIN/llvm-ar RANLIB=$TOOLCHAIN/llvm-ranlib --enable-cblas arm64
    python ./build/flatten-headers.py -l -v1 xxxx // please see run.sh

    cd ../../jni

    mv Android-blis.mk Android.mk
    ndk-build


#### voice system

    adb push activation/data/baomao_M_0020.16000.8.float.pcm /data/vsys-final/activation/data
    adb push 3rd-party/workdir_cn /data/vsys-final/3rd-party
    adb push libs/arm64-v8a/ data/vsys-final
    adb push activation/test_activation/run.sh /data/vsys-final
    ./run.sh
