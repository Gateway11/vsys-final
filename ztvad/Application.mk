#APP_ABI := armeabi-v7a arm64-v8a
APP_ABI := armeabi-v7a
#APP_ABI := arm64-v8a
# c++_static for c++11 support of STL
APP_STL := c++_static
APP_OPTIM := release

OPT_CFLAGS := -O3
APP_CFLAGS := $(APP_CFLAGS) $(OPT_CFLAGS)

APP_CPPFLAGS := $(APP_CLFAGS)
APP_CPPFLAGS += -std=c++11 -frtti
#APP_CPPFLAGS += -fpermissive

APP_PLATFORM := android-19

NDK_TOOLCHAIN_VERSION := 4.9
