#!/bin/sh
 
export NDK_DIR="../../toolbox/ndk-r21"
export INSTALL_DIR="`pwd`/jni_arm"
export API=32
export TOOLCHAIN="$NDK_DIR/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin"
export SYS_ROOT="$NDK_DIR/platforms/android-$API/arch-arm/"
export CC="$TOOLCHAIN/arm-linux-androideabi-gcc --sysroot=$SYS_ROOT"
export LD="$TOOLCHAIN/arm-linux-androideabi-ld"
export LD_LIBRARY_PATH="$NDK_DIR/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/lib/gcc/arm-linux-androideabi/4.9.x"
export AR="$TOOLCHAIN/arm-linux-androideabi-ar"
export RANLIB="$TOOLCHAIN/arm-linux-androideabi-ranlib"
export STRIP="$TOOLCHAIN/arm-linux-androideabi-strip"
export CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=neon -fno-builtin-memmove -mthumb -D__ANDROID_API__=$API" 
export C_INCLUDE_PATH="$NDK_DIR/sysroot/usr/include:$NDK_DIR/sysroot/usr/include/arm-linux-androideabi"
 
mkdir -p $INSTALL_DIR
./configure --host=arm-linux-androideabi \
            --prefix=$INSTALL_DIR \
            LIBS="-L$LD_LIBRARY_PATH -L$SYS_ROOT/usr/lib -lc -lgcc" \
            --enable-shared\
            --enable-float --enable-neon

make -j4
make install
