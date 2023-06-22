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

#include "webrtc/modules/audio_processing/beamformer/beam_adapter.h"

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

BeamAdapter::BeamAdapter() {
  WindowGenerator::KaiserBesselDerived(kKbdAlpha, kFftSize, window_);
}

void BeamAdapter::Initialize(int chunk_size_ms, int sample_rate_hz) {
    chunk_length_ = sample_rate_hz / (1000.f / chunk_size_ms);
    sample_rate_hz_ = sample_rate_hz;

    memset(refs_, 0, sizeof(refs_));
    memset(mics_, 0, sizeof(mics_));
    memset(scales_, 0, sizeof(scales_));
    frames_ = 0;
    is_adapted_ = 0;

    lapped_transform_.reset(new LappedTransform(2,
                                              1,
                                              chunk_length_,
                                              window_,
                                              kFftSize,
                                              kFftSize / 2,
                                              this));
}

int BeamAdapter::GetFreqScales(float *scales, int length) {
    if (!scales || length != kFftSize/2+1)
        return -1;

    memcpy(scales, scales_, length*sizeof(float));
    return 0;
}

void BeamAdapter::ProcessChunk(const ChannelBuffer<float>* input,
                              ChannelBuffer<float>* output) {
  DCHECK_EQ(input->num_channels(), 2);
  DCHECK_EQ(input->num_frames_per_band(), chunk_length_);

  lapped_transform_->ProcessChunk(input->channels(0), output->channels(0));
}

void BeamAdapter::ProcessAudioBlock(const complex_f* const* input,
                                   int num_input_channels,
                                   int num_freq_bins,
                                   int num_output_channels,
                                   complex_f* const* output) {
    CHECK_EQ(num_freq_bins, kNumFreqBins);
    CHECK_EQ(num_input_channels, 2);
    CHECK_EQ(num_output_channels, 1);

    frames_ ++;
    for (int i=0; i<num_freq_bins; i++) {
        if (!is_adapted_) {
            refs_[i] += std::abs(input[0][i]);
            mics_[i] += std::abs(input[1][i]);
            if (mics_[i] < 0.0001f)
                mics_[i] = 0.0001f;
            scales_[i] = refs_[i] / mics_[i] / 2;
            if (scales_[i] > 4)
                scales_[i] = 4;
            else if (scales_[i] < 1.0f / 4)
                scales_[i] = 1.0f / 4;
        }
        output[0][i] = input[1][i] * scales_[i];
    }
    if (frames_ == 10) {
        is_adapted_ = 1;
    }
}

}  // namespace webrtc
