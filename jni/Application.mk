APP_ABI                 := arm64-v8a #armeabi-v7a
APP_CFLAGS              := -O3
APP_CPPFLAGS            := -std=c++11 -frtti -Wno-register #-fpermissive
#APP_LDFLAGS             := "-Wl,--start-group" -llog "-Wl,--end-group"
APP_LDFLAGS             := -llog
APP_PLATFORM            := android-32
APP_STL                 := c++_static
