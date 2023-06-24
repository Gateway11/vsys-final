LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := r2ssp
LOCAL_SRC_FILES := \
    ../r2ssp/r2ssp.cpp \
    ../r2ssp/bf/r2_beamformer.cc \
    ../r2ssp/bf/r2_soundtracker.cpp \
    ../r2ssp/fe/fftwrapper.cpp \
    ../r2ssp/speexdsp/libspeexdsp/buffer.c \
    ../r2ssp/speexdsp/libspeexdsp/fftwrap.c \
    ../r2ssp/speexdsp/libspeexdsp/filterbank.c \
    ../r2ssp/speexdsp/libspeexdsp/jitter.c \
    ../r2ssp/speexdsp/libspeexdsp/kiss_fft.c \
    ../r2ssp/speexdsp/libspeexdsp/kiss_fftr.c \
    ../r2ssp/speexdsp/libspeexdsp/mdf.c \
    ../r2ssp/speexdsp/libspeexdsp/preprocess.c \
    ../r2ssp/speexdsp/libspeexdsp/resample.c \
    ../r2ssp/speexdsp/libspeexdsp/scal.c \
    ../r2ssp/speexdsp/libspeexdsp/smallft.c \
    ../r2ssp/webrtc/modules/audio_processing/utility/delay_estimator_wrapper.c \
    ../r2ssp/webrtc/modules/audio_processing/utility/delay_estimator.c \
    ../r2ssp/webrtc/modules/audio_processing/utility/fft4g.c \
    ../r2ssp/webrtc/modules/audio_processing/aec/aec_core.c \
    ../r2ssp/webrtc/modules/audio_processing/aec/aec_rdft.c \
    ../r2ssp/webrtc/modules/audio_processing/aec/aec_resampler.c \
    ../r2ssp/webrtc/modules/audio_processing/aec/echo_cancellation.c \
    ../r2ssp/webrtc/modules/audio_processing/aecm/aecm_core_c.c \
    ../r2ssp/webrtc/modules/audio_processing/aecm/aecm_core.c \
    ../r2ssp/webrtc/modules/audio_processing/aecm/echo_control_mobile.c \
    ../r2ssp/webrtc/modules/audio_processing/ns/noise_suppression_x.c \
    ../r2ssp/webrtc/modules/audio_processing/ns/noise_suppression.c \
    ../r2ssp/webrtc/modules/audio_processing/ns/ns_core.c \
    ../r2ssp/webrtc/modules/audio_processing/ns/nsx_core_c.c \
    ../r2ssp/webrtc/modules/audio_processing/ns/nsx_core.c \
    ../r2ssp/webrtc/modules/audio_processing/agc/agc_audio_proc.cc \
    ../r2ssp/webrtc/modules/audio_processing/agc/agc.cc \
    ../r2ssp/webrtc/modules/audio_processing/agc/circular_buffer.cc \
    ../r2ssp/webrtc/modules/audio_processing/agc/gmm.cc \
    ../r2ssp/webrtc/modules/audio_processing/agc/histogram.cc \
    ../r2ssp/webrtc/modules/audio_processing/agc/pitch_based_vad.cc \
    ../r2ssp/webrtc/modules/audio_processing/agc/pitch_internal.cc \
    ../r2ssp/webrtc/modules/audio_processing/agc/pole_zero_filter.cc \
    ../r2ssp/webrtc/modules/audio_processing/agc/standalone_vad.cc \
    ../r2ssp/webrtc/modules/audio_processing/agc/utility.cc \
    ../r2ssp/webrtc/modules/audio_processing/agc/legacy/analog_agc.c \
    ../r2ssp/webrtc/modules/audio_processing/agc/legacy/digital_agc.c \
    ../r2ssp/webrtc/modules/audio_processing/beamformer/nonlinear_beamformer.cc \
    ../r2ssp/webrtc/modules/audio_processing/beamformer/covariance_matrix_generator.cc \
    ../r2ssp/webrtc/modules/audio_processing/beamformer/pcm_utils.cc \
    ../r2ssp/webrtc/modules/audio_processing/beamformer/beam_localizer.cc \
    ../r2ssp/webrtc/modules/audio_processing/beamformer/beam_filter.cc \
    ../r2ssp/webrtc/modules/audio_processing/beamformer/beam_adapter.cc \
    ../r2ssp/webrtc/modules/audio_coding/codecs/isac/main/source/intialize.c \
    ../r2ssp/webrtc/modules/audio_coding/codecs/isac/main/source/pitch_estimator.c \
    ../r2ssp/webrtc/modules/audio_coding/codecs/isac/main/source/filterbanks.c \
    ../r2ssp/webrtc/modules/audio_coding/codecs/isac/main/source/lpc_analysis.c \
    ../r2ssp/webrtc/modules/audio_coding/codecs/isac/main/source/filter_functions.c \
    ../r2ssp/webrtc/modules/audio_coding/codecs/isac/main/source/transform.c \
    ../r2ssp/webrtc/modules/audio_coding/codecs/isac/main/source/pitch_filter.c \
    ../r2ssp/webrtc/modules/audio_coding/codecs/isac/main/source/filterbank_tables.c \
    ../r2ssp/webrtc/modules/audio_coding/codecs/isac/main/source/fft.c \
    ../r2ssp/webrtc/common_audio/signal_processing/randomization_functions.c \
    ../r2ssp/webrtc/common_audio/signal_processing/real_fft.c \
    ../r2ssp/webrtc/common_audio/signal_processing/min_max_operations.c \
    ../r2ssp/webrtc/common_audio/signal_processing/spl_init.c \
    ../r2ssp/webrtc/common_audio/signal_processing/division_operations.c \
    ../r2ssp/webrtc/common_audio/signal_processing/resample.c \
    ../r2ssp/webrtc/common_audio/signal_processing/resample_by_2_internal.c \
    ../r2ssp/webrtc/common_audio/signal_processing/resample_48khz.c \
    ../r2ssp/webrtc/common_audio/signal_processing/resample_fractional.c \
    ../r2ssp/webrtc/common_audio/signal_processing/energy.c \
    ../r2ssp/webrtc/common_audio/signal_processing/get_scaling_square.c \
    ../r2ssp/webrtc/common_audio/signal_processing/complex_fft.c \
    ../r2ssp/webrtc/common_audio/signal_processing/cross_correlation.c \
    ../r2ssp/webrtc/common_audio/signal_processing/downsample_fast.c \
    ../r2ssp/webrtc/common_audio/signal_processing/vector_scaling_operations.c \
    ../r2ssp/webrtc/common_audio/signal_processing/copy_set_operations.c \
    ../r2ssp/webrtc/common_audio/signal_processing/resample_by_2.c \
    ../r2ssp/webrtc/common_audio/signal_processing/dot_product_with_scale.c \
    ../r2ssp/webrtc/common_audio/signal_processing/spl_sqrt.c \
    ../r2ssp/webrtc/common_audio/resampler/push_resampler.cc \
    ../r2ssp/webrtc/common_audio/resampler/push_sinc_resampler.cc \
    ../r2ssp/webrtc/common_audio/resampler/resampler.cc \
    ../r2ssp/webrtc/common_audio/resampler/sinc_resampler.cc \
    ../r2ssp/webrtc/common_audio/resampler/sinusoidal_linear_chirp_source.cc \
    ../r2ssp/webrtc/common_audio/audio_util.cc \
    ../r2ssp/webrtc/common_audio/lapped_transform.cc \
    ../r2ssp/webrtc/common_audio/window_generator.cc \
    ../r2ssp/webrtc/common_audio/blocker.cc \
    ../r2ssp/webrtc/common_audio/real_fourier.cc \
    ../r2ssp/webrtc/common_audio/audio_ring_buffer.cc \
    ../r2ssp/webrtc/common_audio/ring_buffer.c \
    ../r2ssp/webrtc/common_audio/fir_filter.cc \
    ../r2ssp/webrtc/common_audio/vad/vad_core.c \
    ../r2ssp/webrtc/common_audio/vad/vad_filterbank.c \
    ../r2ssp/webrtc/common_audio/vad/vad_gmm.c \
    ../r2ssp/webrtc/common_audio/vad/vad_sp.c \
    ../r2ssp/webrtc/common_audio/vad/vad.cc \
    ../r2ssp/webrtc/common_audio/vad/webrtc_vad.c \
    ../r2ssp/webrtc/system_wrappers/source/cpu_features.cc \
    ../r2ssp/webrtc/system_wrappers/source/aligned_malloc.cc \
    ../r2ssp/webrtc/base/checks.cc \
    ../r2ssp/webrtc/common_audio/signal_processing/spl_sqrt_floor.c \
    ../r2ssp/webrtc/common_audio/signal_processing/complex_bit_reverse.c \
    ../r2ssp/third_party/openmax_dl/dl/sp/src/arm/omxSP_FFTGetBufSize_R_F32.c \
    ../r2ssp/third_party/openmax_dl/dl/sp/src/arm/omxSP_FFTGetBufSize_R_S32.c \
    ../r2ssp/third_party/openmax_dl/dl/sp/src/arm/omxSP_FFTInit_R_F32.c \
    ../r2ssp/third_party/openmax_dl/dl/sp/src/arm/omxSP_FFTInit_R_S32.c \
    ../r2ssp/third_party/openmax_dl/dl/sp/src/arm/armSP_FFT_S32TwiddleTable.c \
    ../r2ssp/third_party/openmax_dl/dl/sp/src/arm/armSP_FFT_F32TwiddleTable.c \
    ../r2ssp/webrtc/modules/audio_processing/aec/aec_core_neon.c \
    ../r2ssp/webrtc/modules/audio_processing/aec/aec_rdft_neon.c \
    ../r2ssp/webrtc/modules/audio_processing/aecm/aecm_core_neon.c \
    ../r2ssp/webrtc/modules/audio_processing/ns/nsx_core_neon.c \
    ../r2ssp/webrtc/common_audio/signal_processing/min_max_operations_neon.c \
    ../r2ssp/webrtc/common_audio/signal_processing/cross_correlation_neon.c \
    ../r2ssp/webrtc/common_audio/signal_processing/downsample_fast_neon.c \
    ../r2ssp/webrtc/common_audio/resampler/sinc_resampler_neon.cc \
    ../r2ssp/webrtc/common_audio/fir_filter_neon.cc

#https://android.googlesource.com/platform/external/chromium_org/third_party/openmax_dl
#http://aospxref.com/android-5.1.1_r9/xref/external/chromium_org/third_party/openmax_dl
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_SRC_FILES += $(call all-named-files-under,*.S, ../r2ssp/third_party/openmax_dl/dl/sp/src/arm/armv7)
LOCAL_CFLAGS += -DWEBRTC_ANDROID -DWEBRTC_THREAD_RR -DWEBRTC_CLOCK_TYPE_REALTIME -DWEBRTC_POSIX -DWEBRTC_ARCH_ARM -DWEBRTC_ARCH_ARM_NEON -DWEBRTC_ARCH_ARM_V7 -DHAVE_CONFIG_H
else
LOCAL_SRC_FILES += $(call all-named-files-under,*.S, ../r2ssp/third_party/openmax_dl/dl/sp/src/arm/arm64)
LOCAL_SRC_FILES += $(call all-named-files-under,*.c, ../r2ssp/third_party/openmax_dl/dl/sp/src/arm/arm64)
LOCAL_CFLAGS += -DWEBRTC_ANDROID -DWEBRTC_THREAD_RR -DWEBRTC_CLOCK_TYPE_REALTIME -DWEBRTC_POSIX -DWEBRTC_ARCH_ARM -DWEBRTC_ARCH_ARM64_NEON -DDL_ARM_NEON -DHAVE_CONFIG_H
endif

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../r2ssp \
    $(LOCAL_PATH)/../r2ssp/speexdsp \
    $(LOCAL_PATH)/../r2ssp/speexdsp/include \
    $(LOCAL_PATH)/../r2ssp/third_party/openmax_dl \
    $(LOCAL_PATH)/../3rd-party/armeabi-v7a/include \
    
LOCAL_STATIC_LIBRARIES := fftw #blis
LOCAL_SHARED_LIBRARIES := blis
LOCAL_LDLIBS := -llog -lm -ldl

LOCAL_CPP_FEATURES += exceptions 

include $(BUILD_SHARED_LIBRARY)
