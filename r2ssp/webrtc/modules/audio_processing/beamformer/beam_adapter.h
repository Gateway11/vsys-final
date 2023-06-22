/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_BEAMFORMER_BEAM_ADAPTER_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_BEAMFORMER_BEAM_ADAPTER_H_

#include <vector>

#include "webrtc/common_audio/lapped_transform.h"
#include "webrtc/modules/audio_processing/beamformer/complex_matrix.h"
#include "webrtc/modules/audio_processing/beamformer/array_util.h"

namespace webrtc {

class BeamAdapter : public LappedTransform::Callback {
 public:
  explicit BeamAdapter();

  // Sample rate corresponds to the lower band.
  // Needs to be called before the NonlinearBeamformer can be used.
  virtual void Initialize(int chunk_size_ms, int sample_rate_hz);

  int IsAdapted() { return is_adapted_; }
  int GetFreqScales(float *scales, int length);

  // Process one time-domain chunk of audio. The audio is expected to be split
  // into frequency bands inside the ChannelBuffer. The number of frames and
  // channels must correspond to the constructor parameters. The same
  // ChannelBuffer can be passed in as |input| and |output|.
  virtual void ProcessChunk(const ChannelBuffer<float>* input,
                            ChannelBuffer<float>* output);

 protected:
  // Process one frequency-domain block of audio. This is where the fun
  // happens. Implements LappedTransform::Callback.
  void ProcessAudioBlock(const complex<float>* const* input,
                         int num_input_channels,
                         int num_freq_bins,
                         int num_output_channels,
                         complex<float>* const* output) override;

 private:
  typedef Matrix<float> MatrixF;
  typedef ComplexMatrix<float> ComplexMatrixF;
  typedef complex<float> complex_f;

  static const int kFftSize = 256;
  static const int kNumFreqBins = kFftSize / 2 + 1;

  // Deals with the fft transform and blocking.
  int chunk_length_;
  rtc::scoped_ptr<LappedTransform> lapped_transform_;
  float window_[kFftSize];

  float refs_[kNumFreqBins];
  float mics_[kNumFreqBins];
  float scales_[kNumFreqBins];
  int frames_;
  int is_adapted_;

  // Parameters exposed to the user.
  int sample_rate_hz_;
};

}  // namespace webrtc

#endif  // WEBRTC_MODULES_AUDIO_PROCESSING_BEAMFORMER_BEAM_ADAPTER_H_
