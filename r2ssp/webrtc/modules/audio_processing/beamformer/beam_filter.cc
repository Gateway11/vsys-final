/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#define _USE_MATH_DEFINES

#include "webrtc/modules/audio_processing/beamformer/beam_filter.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

#include "webrtc/base/arraysize.h"
#include "webrtc/common_audio/window_generator.h"
#include "webrtc/modules/audio_processing/beamformer/covariance_matrix_generator.h"

namespace webrtc {

// Alpha for the Kaiser Bessel Derived window.
const float kKbdAlpha = 1.5f;

BeamFilter::BeamFilter() {
  WindowGenerator::KaiserBesselDerived(kKbdAlpha, kFftSize, window_);
}

void BeamFilter::Initialize(int chunk_size_ms, int sample_rate_hz,
                            int start_freq_bin, int stop_freq_bin, int is_band_pass) {
  chunk_length_ = sample_rate_hz / (1000.f / chunk_size_ms);
  sample_rate_hz_ = sample_rate_hz;
    start_freq_bin_ = start_freq_bin;
    stop_freq_bin_ = stop_freq_bin;
    is_band_pass_ = is_band_pass;
    use_scales_ = 0;

  lapped_transform_.reset(new LappedTransform(1,
                                              1,
                                              chunk_length_,
                                              window_,
                                              kFftSize,
                                              kFftSize / 2,
                                              this));
}

int BeamFilter::SetFreqScales(float *scales, int length) {
    if (length != kFftSize/2+1)
        return -1;

    memcpy(scales_, scales, length*sizeof(float));
    use_scales_ = 1;
    return 0;
}

void BeamFilter::ProcessChunk(const ChannelBuffer<float>* input,
                              ChannelBuffer<float>* output) {
  DCHECK_EQ(input->num_channels(), 1);
  DCHECK_EQ(input->num_frames_per_band(), chunk_length_);

  lapped_transform_->ProcessChunk(input->channels(0), output->channels(0));
}

void BeamFilter::ProcessAudioBlock(const complex_f* const* input,
                                   int num_input_channels,
                                   int num_freq_bins,
                                   int num_output_channels,
                                   complex_f* const* output) {
  CHECK_EQ(num_freq_bins, kNumFreqBins);
  CHECK_EQ(num_input_channels, 1);
  CHECK_EQ(num_output_channels, 1);

    for (int i=0; i<num_freq_bins; i++) {
        float scale = 1.0f;
        if (use_scales_)
            scale = scales_[i];
        else {
            if (is_band_pass_) {
                if (i < start_freq_bin_ || i > stop_freq_bin_)
                    scale = 0.0f;
            }
            else {
                if (i >= start_freq_bin_ && i <= stop_freq_bin_)
                    scale = 0.0f;
            }
        }
        output[0][i] = input[0][i] * scale;
    }
}

}  // namespace webrtc
