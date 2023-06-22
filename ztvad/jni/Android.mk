LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := fftw
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libs/libfftw3f.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := blis
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libs/libblis.a
include $(PREBUILT_STATIC_LIBRARY)
#include Android-blis.mk

include $(CLEAR_VARS)
LOCAL_MODULE    := r2ssp
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libs/libr2ssp.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ztvad
LOCAL_SRC_FILES := \
    ../vad_jni.cpp \
	../VAD.cpp \
	../VoiceDetector.cpp \
	../NNVadIntf.cpp \
	../amlib/dnn.cpp \
	../felib/AllocSpace.cpp \
	../felib/feintf.cpp \
	../felib/FloatFFT.cpp \
	../felib/PLP.cpp \
	../felib/dictionary.cpp \
	../felib/iniparser.cpp \
	../felib/fftwrapper.cpp \
	../webrtc/common_audio/vad/vad_core.c \
	../webrtc/common_audio/vad/vad_filterbank.c \
	../webrtc/common_audio/vad/vad_gmm.c \
	../webrtc/common_audio/vad/vad_sp.c \
	../webrtc/common_audio/vad/vad.cc \
	../webrtc/common_audio/vad/webrtc_vad.c \
	../webrtc/common_audio/signal_processing/spl_init.c \
	../webrtc/common_audio/signal_processing/min_max_operations.c \
	../webrtc/common_audio/signal_processing/cross_correlation.c \
	../webrtc/common_audio/signal_processing/resample_48khz.c \
	../webrtc/common_audio/signal_processing/resample_by_2_internal.c \
	../webrtc/common_audio/signal_processing/resample_fractional.c \
	../webrtc/common_audio/signal_processing/division_operations.c \
	../webrtc/common_audio/signal_processing/energy.c \
	../webrtc/common_audio/signal_processing/get_scaling_square.c \
	../webrtc/common_audio/signal_processing/downsample_fast.c \
	../webrtc/common_audio/signal_processing/vector_scaling_operations.c \
	../webrtc/base/checks.cc \
	../webrtc/system_wrappers/source/cpu_features.cc \
	../webrtc/system_wrappers/source/cpu_features_android.c \
	../kaldi-pitch/base/io-funcs.cc \
	../kaldi-pitch/base/kaldi-error.cc \
	../kaldi-pitch/base/kaldi-math.cc \
	../kaldi-pitch/base/kaldi-utils.cc \
	../kaldi-pitch/feat/feature-fbank.cc \
	../kaldi-pitch/feat/feature-functions.cc \
	../kaldi-pitch/feat/feature-mfcc.cc \
	../kaldi-pitch/feat/feature-plp.cc \
	../kaldi-pitch/feat/feature-spectrogram.cc \
	../kaldi-pitch/feat/mel-computations.cc \
	../kaldi-pitch/feat/online-feature.cc \
	../kaldi-pitch/feat/pitch-functions.cc \
	../kaldi-pitch/feat/resample.cc \
	../kaldi-pitch/feat/signal.cc \
	../kaldi-pitch/feat/sinusoid-detection.cc \
	../kaldi-pitch/feat/wave-reader.cc \
	../kaldi-pitch/matrix/compressed-matrix.cc \
	../kaldi-pitch/matrix/kaldi-gpsr.cc \
	../kaldi-pitch/matrix/kaldi-matrix.cc \
	../kaldi-pitch/matrix/kaldi-vector.cc \
	../kaldi-pitch/matrix/matrix-functions.cc \
	../kaldi-pitch/matrix/optimization.cc \
	../kaldi-pitch/matrix/packed-matrix.cc \
	../kaldi-pitch/matrix/qr.cc \
	../kaldi-pitch/matrix/sparse-matrix.cc \
	../kaldi-pitch/matrix/sp-matrix.cc \
	../kaldi-pitch/matrix/srfft.cc \
	../kaldi-pitch/matrix/tp-matrix.cc \
	../kaldi-pitch/transform/cmvn.cc \
	../kaldi-pitch/util/kaldi-io.cc \
	../kaldi-pitch/util/kaldi-table.cc \
	../kaldi-pitch/util/parse-options.cc \
	../kaldi-pitch/util/simple-io-funcs.cc \
	../kaldi-pitch/util/text-utils.cc \

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/.. \
    $(LOCAL_PATH)/../kaldi-pitch \
    $(LOCAL_PATH)/$(TARGET_ARCH_ABI)/include \
    
LOCAL_CFLAGS += -DWEBRTC_POSIX #-DFE_USE_NE10_FFT

LOCAL_CPP_FEATURES += exceptions

LOCAL_STATIC_LIBRARIES := cpufeatures fftw blis #ne10

LOCAL_SHARED_LIBRARIES := r2ssp #blis

LOCAL_LDLIBS := -llog -lm -ldl

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)
