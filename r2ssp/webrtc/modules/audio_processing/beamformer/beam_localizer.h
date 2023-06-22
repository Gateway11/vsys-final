/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_AUDIO_PROCESSING_BEAMFORMER_BEAMLOCALIZER_H_
#define WEBRTC_MODULES_AUDIO_PROCESSING_BEAMFORMER_BEAMLOCALIZER_H_

#include <vector>

#include "webrtc/common_audio/lapped_transform.h"
#include "webrtc/modules/audio_processing/beamformer/complex_matrix.h"
#include "webrtc/modules/audio_processing/beamformer/array_util.h"

#include "bf/r2_soundtracker.h"

namespace webrtc {

struct SCandidate {
	int azimuth;
	int elevation;
	float confidence;
};

// Enhances sound sources coming directly in front of a uniform linear array
// and suppresses sound sources coming from all other directions. Operates on
// multichannel signals and produces single-channel output.
//
// The implemented nonlinear postfilter algorithm taken from "A Robust Nonlinear
// Beamforming Postprocessor" by Bastiaan Kleijn.
//
// TODO: Target angle assumed to be 0. Parameterize target angle.
class BeamLocalizer: public LappedTransform::Callback {
public:
	// At the moment it only accepts uniform linear microphone arrays. Using the
	// first microphone as a reference position [0, 0, 0] is a natural choice.
	explicit BeamLocalizer(const std::vector<Point>& array_geometry);

	// Sample rate corresponds to the lower band.
	// Needs to be called before the NonlinearBeamformer can be used.
	virtual void Initialize(int chunk_size_ms, int sample_rate_hz);

	// Process one time-domain chunk of audio. The audio is expected to be split
	// into frequency bands inside the ChannelBuffer. The number of frames and
	// channels must correspond to the constructor parameters. The same
	// ChannelBuffer can be passed in as |input| and |output|.
	virtual void ProcessChunk(const ChannelBuffer<float>* input,
			ChannelBuffer<float>* output);
	// After processing each block |is_target_present_| is set to true if the
	// target signal is present and to false otherwise. This methods can be called
	// to know if the data is target signal or interference and process it
	// accordingly.
	//virtual bool is_target_present() {
	//	return is_target_present_;
	//}

	void Reset();

    int GetCandidateNum() { return sound_tracker_.GetCandidateNum(); }
	int GetCandidate(int nCandi, R2SoundCandidate &sc);

//protected:
	// Process one frequency-domain block of audio. This is where the fun
	// happens. Implements LappedTransform::Callback.
	void ProcessAudioBlock(const complex<float>* const * input,
			int num_input_channels, int num_freq_bins, int num_output_channels,
			complex<float>* const * output) override;

private:
	typedef Matrix<float> MatrixF;
	typedef ComplexMatrix<float> ComplexMatrixF;
	typedef complex<float> complex_f;

	void InitDelaySumMasks();
	//void InitTargetCovMats();
    //void InitInterfCovMats();

	// An implementation of equation 18, which calculates postfilter masks that,
	// when applied, minimize the mean-square error of our estimation of the
	// desired signal. A sub-task is to calculate lambda, which is solved via
	// equation 13.
	void CalculatePostfilterMasks(const complex<float>* const * input,
			int num_input_channels, int isVertical, int azimuthIdx, int hcandiIdx);
	float CalculatePostfilterMask(const ComplexMatrixF& interf_cov_mat,
			float rpsiw, float ratio_rxiw_rxim, float rmxi_r,
			float mask_threshold);

	// Prevents the postfilter masks from degenerating too quickly (a cause of
	// musical noise).
	void ApplyMaskSmoothing(int isVertical);

	// The postfilter masks are unreliable at low frequencies. Calculates a better
	// mask by averaging mid-low frequency values.
	//void ApplyLowFrequencyCorrection();

	// Postfilter masks are also unreliable at high frequencies. Average mid-high
	// frequency masks to calculate a single mask per block which can be applied
	// in the time-domain. Further, we average these block-masks over a chunk,
	// resulting in one postfilter mask per audio chunk. This allows us to skip
	// both transforming and blocking the high-frequency signal.
	//void ApplyHighFrequencyCorrection();

	// Applies both sets of masks to |input| and store in |output|.
	//void ApplyMasks(const complex_f* const* input, complex_f* const* output);

	//void EstimateTargetPresence(int azimuthIdx);

	static const int kFftSize = 256; // 16ms at 16khz
	static const int kNumFreqBins = kFftSize / 2 + 1;
    static const int kBlockSkipStep = 6; // change smooth factor when change this

	// Deals with the fft transform and blocking.
	int chunk_length_;
	rtc::scoped_ptr<LappedTransform> lapped_transform_;
	float window_[kFftSize];

	// Parameters exposed to the user.
	const int num_input_channels_;
	int sample_rate_hz_;
    float eig_m_norm_factor_scale_;

	const std::vector<Point> array_geometry_;
    std::vector<float> array_delays_;

	// Calculated based on user-input and constants in the .cc file.
	int low_average_start_bin_;
	int low_average_end_bin_;
	int high_average_start_bin_;
	int high_average_end_bin_;

	static const int kNumInterfDirections = 72;
	static const int kNumVInterfDirections = 19;

	// Old masks are saved for smoothing. Matrix of size 1 x |kNumFreqBins|.
	float postfilter_masks_[kNumInterfDirections][kNumFreqBins];
	float new_masks_[kNumInterfDirections][kNumFreqBins];
	float postfilter_vmasks_[kNumVInterfDirections][kNumFreqBins];
	float new_vmasks_[kNumVInterfDirections][kNumFreqBins];

	// Array of length |kNumFreqBins|, Matrix of size |1| x |num_channels_|.
	ComplexMatrixF delay_sum_masks_[kNumInterfDirections][kNumFreqBins];
	//ComplexMatrixF normalized_delay_sum_masks_[kNumFreqBins];
	ComplexMatrixF vdelay_sum_masks_[kNumInterfDirections][kNumVInterfDirections][kNumFreqBins];

	// Array of length |kNumFreqBins|, Matrix of size |num_input_channels_| x
	// |num_input_channels_|.
	ComplexMatrixF target_cov_mats_[kNumInterfDirections][kNumFreqBins];
	ComplexMatrixF vtarget_cov_mats_[kNumInterfDirections][kNumVInterfDirections][kNumFreqBins];

	// Array of length |kNumFreqBins|, Matrix of size |num_input_channels_| x
	// |num_input_channels_|.
	//ComplexMatrixF interf_cov_mats_[kNumFreqBins];
	//ComplexMatrixF reflected_interf_cov_mats_[kNumFreqBins];

	//ComplexMatrixF interf_cov_mats_[kNumInterfDirections][kNumFreqBins];
	//ComplexMatrixF interf_cov_mats2_[kNumInterfDirections][kNumFreqBins];
	//ComplexMatrixF interf_cov_mats3_[kNumFreqBins];

	// Of length |kNumFreqBins|.
	//float mask_thresholds_[kNumFreqBins];
	//float wave_numbers_[kNumFreqBins];

	// Preallocated for ProcessAudioBlock()
	// Of length |kNumFreqBins|.
	float rxiws_[kNumInterfDirections][kNumFreqBins];
	float vrxiws_[kNumInterfDirections][kNumVInterfDirections][kNumFreqBins];
	//float rpsiws_[kNumFreqBins];
	//float reflected_rpsiws_[kNumFreqBins];

	//float rpsiws_[kNumInterfDirections][kNumFreqBins];
	//float rpsiws2_[kNumInterfDirections][kNumFreqBins];
	//float rpsiws3_[kNumInterfDirections][kNumFreqBins];

	// The microphone normalization factor.
	ComplexMatrixF eig_m_;

	// For processing the high-frequency input signal.
	//float high_pass_postfilter_mask_;

	// True when the target signal is present.
	//bool is_target_present_;
	// Number of blocks after which the data is considered interference if the
	// mask does not pass |kMaskSignalThreshold|.
	//int hold_target_blocks_;
	// Number of blocks since the last mask that passed |kMaskSignalThreshold|.
	//int interference_blocks_count_;

	int block_count_;

	float avg_masks_[kNumInterfDirections];
	float avg_vmasks_[kNumVInterfDirections];
	std::vector<SCandidate> candidates_;
    
    R2SoundTracker  sound_tracker_;
    std::vector<R2SoundCandidate> st_candidates_;
};

}  // namespace webrtc

#endif  // WEBRTC_MODULES_AUDIO_PROCESSING_BEAMFORMER_BEAMLOCALIZER_H_
