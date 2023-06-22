#!/bin/sh
export NDK_HOME=/Users/hadoop/software/android-ndk-r10e
export PATH=$NDK_HOME:$PATH

#cd r2signal
#ndk-build clean
#ndk-build
#cd ../r2ad2
#ndk-build clean
#ndk-build
#cd ../testr2
#ndk-build clean
#ndk-build
#cd ..

#cd r2vt4
#ndk-build clean
#ndk-build
#cd ../testcf
cd testcf
ndk-build clean
ndk-build

