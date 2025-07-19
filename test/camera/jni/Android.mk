LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := mycmaera
LOCAL_SRC_FILES := native-lib.cpp
LOCAL_LDLIBS := -lcamera2ndk -lmediandk -landroid -llog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := nativeapp
LOCAL_SRC_FILES := native-camera.cpp
LOCAL_LDLIBS := -lcamera2ndk -lmediandk -llog
LOCAL_STATIC_LIBRARIES := android_native_app_glue
include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
