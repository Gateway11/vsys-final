LOCAL_PATH := $(call my-dir)

define all-named-files-under
$(patsubst ./%,%, \
  $(shell cd $(LOCAL_PATH) ; \
          find -L $(2) -name "$(1)" -and -not -name ".*") \
 )
endef

include $(CLEAR_VARS)
LOCAL_MODULE    := fftw
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libs/libfftw3f.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := blis
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libs/libblis.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := r2ssp
LOCAL_SRC_FILES := \
    ../r2ssp.cpp \
    ../bf/r2_beamformer.cc \
    ../bf/r2_soundtracker.cpp \
    ../fe/fftwrapper.cpp \
    ../speexdsp/libspeexdsp/buffer.c \
    ../speexdsp/libspeexdsp/fftwrap.c \
    ../speexdsp/libspeexdsp/filterbank.c \
    ../speexdsp/libspeexdsp/jitter.c \
    ../speexdsp/libspeexdsp/kiss_fft.c \
    ../speexdsp/libspeexdsp/kiss_fftr.c \
    ../speexdsp/libspeexdsp/mdf.c \
    ../speexdsp/libspeexdsp/preprocess.c \
    ../speexdsp/libspeexdsp/resample.c \
    ../speexdsp/libspeexdsp/scal.c \
    ../speexdsp/libspeexdsp/smallft.c \
    ../webrtc/modules/audio_processing/utility/delay_estimator_wrapper.c \
    ../webrtc/modules/audio_processing/utility/delay_estimator.c \
    ../webrtc/modules/audio_processing/utility/fft4g.c \
    ../webrtc/modules/audio_processing/aec/aec_core.c \
    ../webrtc/modules/audio_processing/aec/aec_rdft.c \
    ../webrtc/modules/audio_processing/aec/aec_resampler.c \
    ../webrtc/modules/audio_processing/aec/echo_cancellation.c \
    ../webrtc/modules/audio_processing/aecm/aecm_core_c.c \
    ../webrtc/modules/audio_processing/aecm/aecm_core.c \
    ../webrtc/modules/audio_processing/aecm/echo_control_mobile.c \
    ../webrtc/modules/audio_processing/ns/noise_suppression_x.c \
    ../webrtc/modules/audio_processing/ns/noise_suppression.c \
    ../webrtc/modules/audio_processing/ns/ns_core.c \
    ../webrtc/modules/audio_processing/ns/nsx_core_c.c \
    ../webrtc/modules/audio_processing/ns/nsx_core.c \
    ../webrtc/modules/audio_processing/agc/agc_audio_proc.cc \
    ../webrtc/modules/audio_processing/agc/agc.cc \
    ../webrtc/modules/audio_processing/agc/circular_buffer.cc \
    ../webrtc/modules/audio_processing/agc/gmm.cc \
    ../webrtc/modules/audio_processing/agc/histogram.cc \
    ../webrtc/modules/audio_processing/agc/pitch_based_vad.cc \
    ../webrtc/modules/audio_processing/agc/pitch_internal.cc \
    ../webrtc/modules/audio_processing/agc/pole_zero_filter.cc \
    ../webrtc/modules/audio_processing/agc/standalone_vad.cc \
    ../webrtc/modules/audio_processing/agc/utility.cc \
    ../webrtc/modules/audio_processing/agc/legacy/analog_agc.c \
    ../webrtc/modules/audio_processing/agc/legacy/digital_agc.c \
    ../webrtc/modules/audio_processing/beamformer/nonlinear_beamformer.cc \
    ../webrtc/modules/audio_processing/beamformer/covariance_matrix_generator.cc \
    ../webrtc/modules/audio_processing/beamformer/pcm_utils.cc \
    ../webrtc/modules/audio_processing/beamformer/beam_localizer.cc \
    ../webrtc/modules/audio_processing/beamformer/beam_filter.cc \
    ../webrtc/modules/audio_processing/beamformer/beam_adapter.cc \
    ../webrtc/modules/audio_coding/codecs/isac/main/source/intialize.c \
    ../webrtc/modules/audio_coding/codecs/isac/main/source/pitch_estimator.c \
    ../webrtc/modules/audio_coding/codecs/isac/main/source/filterbanks.c \
    ../webrtc/modules/audio_coding/codecs/isac/main/source/lpc_analysis.c \
    ../webrtc/modules/audio_coding/codecs/isac/main/source/filter_functions.c \
    ../webrtc/modules/audio_coding/codecs/isac/main/source/transform.c \
    ../webrtc/modules/audio_coding/codecs/isac/main/source/pitch_filter.c \
    ../webrtc/modules/audio_coding/codecs/isac/main/source/filterbank_tables.c \
    ../webrtc/modules/audio_coding/codecs/isac/main/source/fft.c \
    ../webrtc/common_audio/signal_processing/randomization_functions.c \
    ../webrtc/common_audio/signal_processing/real_fft.c \
    ../webrtc/common_audio/signal_processing/min_max_operations.c \
    ../webrtc/common_audio/signal_processing/spl_init.c \
    ../webrtc/common_audio/signal_processing/division_operations.c \
    ../webrtc/common_audio/signal_processing/resample.c \
    ../webrtc/common_audio/signal_processing/resample_by_2_internal.c \
    ../webrtc/common_audio/signal_processing/resample_48khz.c \
    ../webrtc/common_audio/signal_processing/resample_fractional.c \
    ../webrtc/common_audio/signal_processing/energy.c \
    ../webrtc/common_audio/signal_processing/get_scaling_square.c \
    ../webrtc/common_audio/signal_processing/complex_fft.c \
    ../webrtc/common_audio/signal_processing/cross_correlation.c \
    ../webrtc/common_audio/signal_processing/downsample_fast.c \
    ../webrtc/common_audio/signal_processing/vector_scaling_operations.c \
    ../webrtc/common_audio/signal_processing/copy_set_operations.c \
    ../webrtc/common_audio/signal_processing/resample_by_2.c \
    ../webrtc/common_audio/signal_processing/dot_product_with_scale.c \
    ../webrtc/common_audio/signal_processing/spl_sqrt.c \
    ../webrtc/common_audio/resampler/push_resampler.cc \
    ../webrtc/common_audio/resampler/push_sinc_resampler.cc \
    ../webrtc/common_audio/resampler/resampler.cc \
    ../webrtc/common_audio/resampler/sinc_resampler.cc \
    ../webrtc/common_audio/resampler/sinusoidal_linear_chirp_source.cc \
    ../webrtc/common_audio/audio_util.cc \
    ../webrtc/common_audio/lapped_transform.cc \
    ../webrtc/common_audio/window_generator.cc \
    ../webrtc/common_audio/blocker.cc \
    ../webrtc/common_audio/real_fourier.cc \
    ../webrtc/common_audio/audio_ring_buffer.cc \
    ../webrtc/common_audio/ring_buffer.c \
    ../webrtc/common_audio/fir_filter.cc \
    ../webrtc/common_audio/vad/vad_core.c \
    ../webrtc/common_audio/vad/vad_filterbank.c \
    ../webrtc/common_audio/vad/vad_gmm.c \
    ../webrtc/common_audio/vad/vad_sp.c \
    ../webrtc/common_audio/vad/vad.cc \
    ../webrtc/common_audio/vad/webrtc_vad.c \
    ../webrtc/system_wrappers/source/cpu_features.cc \
    ../webrtc/system_wrappers/source/aligned_malloc.cc \
    ../webrtc/base/checks.cc \
    ../webrtc/common_audio/signal_processing/spl_sqrt_floor.c \
    ../webrtc/common_audio/signal_processing/complex_bit_reverse.c \
    ../third_party/openmax_dl/dl/sp/src/arm/omxSP_FFTGetBufSize_R_F32.c \
    ../third_party/openmax_dl/dl/sp/src/arm/omxSP_FFTGetBufSize_R_S32.c \
    ../third_party/openmax_dl/dl/sp/src/arm/omxSP_FFTInit_R_F32.c \
    ../third_party/openmax_dl/dl/sp/src/arm/omxSP_FFTInit_R_S32.c \
    ../third_party/openmax_dl/dl/sp/src/arm/armSP_FFT_S32TwiddleTable.c \
    ../third_party/openmax_dl/dl/sp/src/arm/armSP_FFT_F32TwiddleTable.c \
    ../webrtc/modules/audio_processing/aec/aec_core_neon.c \
    ../webrtc/modules/audio_processing/aec/aec_rdft_neon.c \
    ../webrtc/modules/audio_processing/aecm/aecm_core_neon.c \
    ../webrtc/modules/audio_processing/ns/nsx_core_neon.c \
    ../webrtc/common_audio/signal_processing/min_max_operations_neon.c \
    ../webrtc/common_audio/signal_processing/cross_correlation_neon.c \
    ../webrtc/common_audio/signal_processing/downsample_fast_neon.c \
    ../webrtc/common_audio/resampler/sinc_resampler_neon.cc \
    ../webrtc/common_audio/fir_filter_neon.cc

#https://android.googlesource.com/platform/external/chromium_org/third_party/openmax_dl
#http://aospxref.com/android-5.1.1_r9/xref/external/chromium_org/third_party/openmax_dl
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_SRC_FILES += $(call all-named-files-under,*.S,../third_party/openmax_dl/dl/sp/src/arm/armv7)
LOCAL_CFLAGS += -DWEBRTC_ANDROID -DWEBRTC_THREAD_RR -DWEBRTC_CLOCK_TYPE_REALTIME -DWEBRTC_POSIX -DWEBRTC_ARCH_ARM -DWEBRTC_ARCH_ARM_NEON -DWEBRTC_ARCH_ARM_V7 -DHAVE_CONFIG_H
else
LOCAL_SRC_FILES += $(call all-named-files-under,*.S,../third_party/openmax_dl/dl/sp/src/arm/arm64)
LOCAL_CFLAGS += -DWEBRTC_ANDROID -DWEBRTC_THREAD_RR -DWEBRTC_CLOCK_TYPE_REALTIME -DWEBRTC_POSIX -DWEBRTC_ARCH_ARM -DWEBRTC_ARCH_ARM64_NEON -DDL_ARM_NEON -DHAVE_CONFIG_H
endif

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/.. \
    $(LOCAL_PATH)/../speexdsp \
    $(LOCAL_PATH)/../speexdsp/include \
    $(LOCAL_PATH)/../third_party/openmax_dl \
    $(LOCAL_PATH)/$(TARGET_ARCH_ABI)/include \
    
LOCAL_CPP_FEATURES += exceptions 

LOCAL_STATIC_LIBRARIES := fftw blis

LOCAL_LDLIBS := -llog -lm -ldl

include $(BUILD_SHARED_LIBRARY)
