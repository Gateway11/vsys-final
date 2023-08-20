APP_ABI                 := arm64-v8a
#APP_ABI                 := armeabi-v7a #arm64-v8a
APP_CFLAGS              := -O3
APP_CPPFLAGS            := -std=c++11 -frtti -Wno-register #-fpermissive
#APP_LDFLAGS             := "-Wl,--start-group" -llog "-Wl,--end-group"
APP_LDFLAGS             := -llog
APP_PLATFORM            := android-32
APP_STL                 := c++_static
NDK_TOOLCHAIN_VERSION   := 4.9
