#!/bin/sh
export NDK_HOME=/Users/hadoop/software/android-ndk-r10e
export PATH=$NDK_HOME:$PATH

cd r2ad3
ndk-build clean
cd ../r2vt4
ndk-build clean
cd ..
