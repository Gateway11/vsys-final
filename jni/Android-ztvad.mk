LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := ztvad
LOCAL_SRC_FILES := \
    ../ztvad/vad_jni.cpp \
	../ztvad/VAD.cpp \
	../ztvad/VoiceDetector.cpp \
	../ztvad/NNVadIntf.cpp \
	../ztvad/amlib/dnn.cpp \
	../ztvad/felib/AllocSpace.cpp \
	../ztvad/felib/feintf.cpp \
	../ztvad/felib/FloatFFT.cpp \
	../ztvad/felib/PLP.cpp \
	../ztvad/felib/dictionary.cpp \
	../ztvad/felib/iniparser.cpp \
	../ztvad/felib/fftwrapper.cpp \
	../ztvad/webrtc/common_audio/vad/vad_core.c \
	../ztvad/webrtc/common_audio/vad/vad_filterbank.c \
	../ztvad/webrtc/common_audio/vad/vad_gmm.c \
	../ztvad/webrtc/common_audio/vad/vad_sp.c \
	../ztvad/webrtc/common_audio/vad/vad.cc \
	../ztvad/webrtc/common_audio/vad/webrtc_vad.c \
	../ztvad/webrtc/common_audio/signal_processing/spl_init.c \
	../ztvad/webrtc/common_audio/signal_processing/min_max_operations.c \
	../ztvad/webrtc/common_audio/signal_processing/cross_correlation.c \
	../ztvad/webrtc/common_audio/signal_processing/resample_48khz.c \
	../ztvad/webrtc/common_audio/signal_processing/resample_by_2_internal.c \
	../ztvad/webrtc/common_audio/signal_processing/resample_fractional.c \
	../ztvad/webrtc/common_audio/signal_processing/division_operations.c \
	../ztvad/webrtc/common_audio/signal_processing/energy.c \
	../ztvad/webrtc/common_audio/signal_processing/get_scaling_square.c \
	../ztvad/webrtc/common_audio/signal_processing/downsample_fast.c \
	../ztvad/webrtc/common_audio/signal_processing/vector_scaling_operations.c \
	../ztvad/webrtc/base/checks.cc \
	../ztvad/webrtc/system_wrappers/source/cpu_features.cc \
	../ztvad/webrtc/system_wrappers/source/cpu_features_android.c \
	../ztvad/kaldi-pitch/base/io-funcs.cc \
	../ztvad/kaldi-pitch/base/kaldi-error.cc \
	../ztvad/kaldi-pitch/base/kaldi-math.cc \
	../ztvad/kaldi-pitch/base/kaldi-utils.cc \
	../ztvad/kaldi-pitch/feat/feature-fbank.cc \
	../ztvad/kaldi-pitch/feat/feature-functions.cc \
	../ztvad/kaldi-pitch/feat/feature-mfcc.cc \
	../ztvad/kaldi-pitch/feat/feature-plp.cc \
	../ztvad/kaldi-pitch/feat/feature-spectrogram.cc \
	../ztvad/kaldi-pitch/feat/mel-computations.cc \
	../ztvad/kaldi-pitch/feat/online-feature.cc \
	../ztvad/kaldi-pitch/feat/pitch-functions.cc \
	../ztvad/kaldi-pitch/feat/resample.cc \
	../ztvad/kaldi-pitch/feat/signal.cc \
	../ztvad/kaldi-pitch/feat/sinusoid-detection.cc \
	../ztvad/kaldi-pitch/feat/wave-reader.cc \
	../ztvad/kaldi-pitch/matrix/compressed-matrix.cc \
	../ztvad/kaldi-pitch/matrix/kaldi-gpsr.cc \
	../ztvad/kaldi-pitch/matrix/kaldi-matrix.cc \
	../ztvad/kaldi-pitch/matrix/kaldi-vector.cc \
	../ztvad/kaldi-pitch/matrix/matrix-functions.cc \
	../ztvad/kaldi-pitch/matrix/optimization.cc \
	../ztvad/kaldi-pitch/matrix/packed-matrix.cc \
	../ztvad/kaldi-pitch/matrix/qr.cc \
	../ztvad/kaldi-pitch/matrix/sparse-matrix.cc \
	../ztvad/kaldi-pitch/matrix/sp-matrix.cc \
	../ztvad/kaldi-pitch/matrix/srfft.cc \
	../ztvad/kaldi-pitch/matrix/tp-matrix.cc \
	../ztvad/kaldi-pitch/transform/cmvn.cc \
	../ztvad/kaldi-pitch/util/kaldi-io.cc \
	../ztvad/kaldi-pitch/util/kaldi-table.cc \
	../ztvad/kaldi-pitch/util/parse-options.cc \
	../ztvad/kaldi-pitch/util/simple-io-funcs.cc \
	../ztvad/kaldi-pitch/util/text-utils.cc \

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../ztvad \
    $(LOCAL_PATH)/../ztvad/kaldi-pitch \
    $(LOCAL_PATH)/../3rd-party/armeabi-v7a/include \
    
LOCAL_STATIC_LIBRARIES := cpufeatures fftw #blis #ne10
LOCAL_SHARED_LIBRARIES := r2ssp blis
LOCAL_LDLIBS := -llog -lm -ldl

LOCAL_CFLAGS += -DWEBRTC_POSIX #-DFE_USE_NE10_FFT
LOCAL_CPP_FEATURES += exceptions

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)
