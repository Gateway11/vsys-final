APP_ABI := armeabi-v7a #arm64-v8a

# c++_static for c++11 support of STL
APP_STL := c++_static
APP_OPTIM := release

OPT_CFLAGS := -O3
APP_CFLAGS := $(APP_CFLAGS) $(OPT_CFLAGS)

APP_CPPFLAGS += -std=c++11 -fPIC -fpermissive -Wno-register

APP_PLATFORM := android-29

NDK_TOOLCHAIN_VERSION := 4.9
