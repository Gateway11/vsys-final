#!/bin/sh
export NDK_HOME=/Users/hadoop/software/android-ndk-r10e
export PATH=$NDK_HOME:$PATH

cd r2vt4
ndk-build clean
ndk-build
cd ../testmat
ndk-build clean
ndk-build
cd ..
