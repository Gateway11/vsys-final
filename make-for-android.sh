cd 3rd-party

git clone git@github.com:flame/blis.git
configure auto

wget https://www.fftw.org/fftw-3.3.10.tar.gz
tar -zxvf fftw-3.3.10.tar.gz

cd fftw-3.3.10
mkdir build
cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../../../../toolbox/ndk-r21/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=armeabi-v7a \
    -DANDROID_PLATFORM=android-29

cd ../../../jni
ndk-build
