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

#include "webrtc/modules/audio_processing/beamformer/beam_localizer.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "webrtc/base/arraysize.h"
#include "webrtc/common_audio/window_generator.h"
#include "webrtc/modules/audio_processing/beamformer/covariance_matrix_generator.h"

namespace webrtc {
namespace {

// Alpha for the Kaiser Bessel Derived window.
const float kAlpha = 1.5f;

// The minimum value a post-processing mask can take.
//const float kMaskMinimum = 0.01f;

const float kSpeedOfSoundMeterSeconds = 343;

const float kInterfStepRadians = 2 * (float) M_PI / 72;
const float kVInterfStepRadians = (float) M_PI / 36;

const float kMinCandiAvgMask = 0.3f;
const float kMaxCandiAvgMask = 0.55f;
    
// When calculating the interference covariance matrix, this is the weight for
// the weighted average between the uniform covariance matrix and the angled
// covariance matrix.
// Rpsi = Rpsi_angled * kBalance + Rpsi_uniform * (1 - kBalance)
//const float kBalance = 0.4f;
//const float kBalance = 0.9f;

// TODO(claguna): need comment here.
//const float kBeamwidthConstant = 0.00002f;
//const float kBeamwidthConstant = 0.00002f;

// Alpha coefficient for mask smoothing.
const float kMaskSmoothAlpha = 0.4f;

// The average mask is computed from masks in this mid-frequency range. If these
// ranges are changed |kMaskQuantile| might need to be adjusted.
//const int kLowAverageStartHz = 200;
const int kLowAverageStartHz = 125;
const int kLowAverageEndHz = 400;

const int kHighAverageStartHz = 3000;
const int kHighAverageEndHz = 5000;

// Quantile of mask values which is used to estimate target presence.
//const float kMaskQuantile = 0.4f;
// Mask threshold over which the data is considered signal and not interference.
//const float kMaskTargetThreshold = 0.4f;
// Time in seconds after which the data is considered interference if the mask
// does not pass |kMaskTargetThreshold|.
//const float kHoldTargetSeconds = 0.25f;

// Does conjugate(|norm_mat|) * |mat| * transpose(|norm_mat|). No extra space is
// used; to accomplish this, we compute both multiplications in the same loop.
// The returned norm is clamped to be non-negative.
float Norm(const ComplexMatrix<float>& mat,
		const ComplexMatrix<float>& norm_mat) {
	CHECK_EQ(norm_mat.num_rows(), 1);
	CHECK_EQ(norm_mat.num_columns(), mat.num_rows());
	CHECK_EQ(norm_mat.num_columns(), mat.num_columns());

	complex<float> first_product = complex<float>(0.f, 0.f);
	complex<float> second_product = complex<float>(0.f, 0.f);

	const complex<float>* const * mat_els = mat.elements();
	const complex<float>* const * norm_mat_els = norm_mat.elements();

	for (int i = 0; i < norm_mat.num_columns(); ++i) {
		for (int j = 0; j < norm_mat.num_columns(); ++j) {
			first_product += conj(norm_mat_els[0][j]) * mat_els[j][i];
		}
		second_product += first_product * norm_mat_els[0][i];
		first_product = 0.f;
	}
	return std::max(second_product.real(), 0.f);
}

// Does conjugate(|lhs|) * |rhs| for row vectors |lhs| and |rhs|.
complex<float> ConjugateDotProduct(const ComplexMatrix<float>& lhs,
		const ComplexMatrix<float>& rhs) {
	CHECK_EQ(lhs.num_rows(), 1);
	CHECK_EQ(rhs.num_rows(), 1);
	CHECK_EQ(lhs.num_columns(), rhs.num_columns());

	const complex<float>* const * lhs_elements = lhs.elements();
	const complex<float>* const * rhs_elements = rhs.elements();

	complex<float> result = complex<float>(0.f, 0.f);
	for (int i = 0; i < lhs.num_columns(); ++i) {
		result += conj(lhs_elements[0][i]) * rhs_elements[0][i];
	}

	return result;
}

// Works for positive numbers only.
int Round(float x) {
	return std::floor(x + 0.5f);
}

// Calculates the sum of absolute values of a complex matrix.
/*float SumAbs(const ComplexMatrix<float>& mat) {
	float sum_abs = 0.f;
	const complex<float>* const * mat_els = mat.elements();
	for (int i = 0; i < mat.num_rows(); ++i) {
		for (int j = 0; j < mat.num_columns(); ++j) {
			sum_abs += std::abs(mat_els[i][j]);
		}
	}
	return sum_abs;
}*/

// Calculates the sum of squares of a complex matrix.
float SumSquares(const ComplexMatrix<float>& mat) {
	float sum_squares = 0.f;
	const complex<float>* const * mat_els = mat.elements();
	for (int i = 0; i < mat.num_rows(); ++i) {
		for (int j = 0; j < mat.num_columns(); ++j) {
			float abs_value = std::abs(mat_els[i][j]);
			sum_squares += abs_value * abs_value;
		}
	}
	return sum_squares;
}

// Does |out| = |in|.' * conj(|in|) for row vector |in|.
void TransposedConjugatedProduct(const ComplexMatrix<float>& in,
		ComplexMatrix<float>* out) {
	CHECK_EQ(in.num_rows(), 1);
	CHECK_EQ(out->num_rows(), in.num_columns());
	CHECK_EQ(out->num_columns(), in.num_columns());
	const complex<float>* in_elements = in.elements()[0];
	complex<float>* const * out_elements = out->elements();
	for (int i = 0; i < out->num_rows(); ++i) {
		for (int j = 0; j < out->num_columns(); ++j) {
			out_elements[i][j] = in_elements[i] * conj(in_elements[j]);
		}
	}
}

std::vector<Point> GetCenteredArray(std::vector<Point> array_geometry) {
	for (int dim = 0; dim < 3; ++dim) {
		float center = 0.f;
		for (size_t i = 0; i < array_geometry.size(); ++i) {
			center += array_geometry[i].c[dim];
		}
		center /= array_geometry.size();
		for (size_t i = 0; i < array_geometry.size(); ++i) {
			array_geometry[i].c[dim] -= center;
		}
	}
	return array_geometry;
}

}  // namespace

BeamLocalizer::BeamLocalizer(const std::vector<Point>& array_geometry) :
		num_input_channels_((int) array_geometry.size()), array_geometry_(
				GetCenteredArray(array_geometry)) {
	WindowGenerator::KaiserBesselDerived(kAlpha, kFftSize, window_);

    eig_m_norm_factor_scale_ = 1 / (sqrtf(num_input_channels_) * 30000);
}

void BeamLocalizer::Initialize(int chunk_size_ms, int sample_rate_hz) {
	chunk_length_ = sample_rate_hz / (1000.f / chunk_size_ms);
	sample_rate_hz_ = sample_rate_hz;
	low_average_start_bin_ = Round(
			kLowAverageStartHz * kFftSize / sample_rate_hz_);
	low_average_end_bin_ = Round(kLowAverageEndHz * kFftSize / sample_rate_hz_);
	high_average_start_bin_ = Round(
			kHighAverageStartHz * kFftSize / sample_rate_hz_);
	high_average_end_bin_ = Round(
			kHighAverageEndHz * kFftSize / sample_rate_hz_);
	//high_pass_postfilter_mask_ = 1.f;
	//is_target_present_ = false;
	//hold_target_blocks_ = kHoldTargetSeconds * 2 * sample_rate_hz / kFftSize;
	//interference_blocks_count_ = hold_target_blocks_;

	DCHECK_LE(low_average_end_bin_, kNumFreqBins);
	DCHECK_LT(low_average_start_bin_, low_average_end_bin_);
	DCHECK_LE(high_average_end_bin_, kNumFreqBins);
	DCHECK_LT(high_average_start_bin_, high_average_end_bin_);

	lapped_transform_.reset(
			new LappedTransform(num_input_channels_, 1, chunk_length_, window_,
					kFftSize,
					//kFftSize / 2, // frame shift
					kFftSize, this));
	//for (int i = 0; i < kNumFreqBins; ++i) {
		//postfilter_mask_[i] = 1.f;
		//float freq_hz = (static_cast<float>(i) / kFftSize) * sample_rate_hz_;
		//wave_numbers_[i] = 2 * M_PI * freq_hz / kSpeedOfSoundMeterSeconds;
		//mask_thresholds_[i] = num_input_channels_ * num_input_channels_
		//		* kBeamwidthConstant * wave_numbers_[i] * wave_numbers_[i];
	//}

	block_count_ = 0;

	// Initialize all nonadaptive values before looping through the frames.
	//high_pass_postfilter_mask_ = 1.f;
	//is_target_present_ = false;
	//interference_blocks_count_ = hold_target_blocks_;

	for (int i = 0; i < kNumFreqBins; ++i) {
		for (int j = 0; j < kNumInterfDirections; j++)
			postfilter_masks_[j][i] = 1.f;
        for (int j = 0; j < kNumVInterfDirections; j++)
            postfilter_vmasks_[j][i] = 1.f;
	}

	InitDelaySumMasks();
	//InitInterfCovMats();

	for (int i = 0; i < kNumFreqBins; ++i) {
		//rpsiws_[i] = Norm(interf_cov_mats_[i], delay_sum_masks_[i]);
		//reflected_rpsiws_[i] =
		//    Norm(reflected_interf_cov_mats_[i], delay_sum_masks_[i]);

		for (int j = 0; j < kNumInterfDirections; j++) {
			rxiws_[j][i] = Norm(target_cov_mats_[j][i], delay_sum_masks_[j][i]);
			for (int k = 0; k < kNumVInterfDirections; k++)
				vrxiws_[j][k][i] = Norm(vtarget_cov_mats_[j][k][i],
						vdelay_sum_masks_[j][k][i]);
			/*rpsiws_[j][i] = Norm(interf_cov_mats_[j][i],
			 delay_sum_masks_[j][i]);
			 rpsiws2_[j][i] = Norm(interf_cov_mats2_[j][i],
			 delay_sum_masks_[j][i]);
			 rpsiws3_[j][i] = Norm(interf_cov_mats3_[i], delay_sum_masks_[j][i]);*/
		}
	}
}

void BeamLocalizer::InitDelaySumMasks() {
	for (int f_ix = 0; f_ix < kNumFreqBins; ++f_ix) {
		float interfAzimuthRadians = 0.0f;
		float interfElevationRadians = 0.f;
		// calculate horizontal directions
		for (int j = 0; j < kNumInterfDirections; j++) {
			delay_sum_masks_[j][f_ix].Resize(1, num_input_channels_);
			CovarianceMatrixGenerator::PhaseAlignmentMasks(f_ix, kFftSize,
					sample_rate_hz_, kSpeedOfSoundMeterSeconds, array_geometry_,
					array_delays_, interfAzimuthRadians, interfElevationRadians,
					&delay_sum_masks_[j][f_ix]);

			complex_f norm_factor = sqrt(
					ConjugateDotProduct(delay_sum_masks_[j][f_ix],
							delay_sum_masks_[j][f_ix]));
			delay_sum_masks_[j][f_ix].Scale(1.f / norm_factor);

			target_cov_mats_[j][f_ix].Resize(num_input_channels_,
					num_input_channels_);
			TransposedConjugatedProduct(delay_sum_masks_[j][f_ix],
					&target_cov_mats_[j][f_ix]);
			complex_f normalization_factor = target_cov_mats_[j][f_ix].Trace();
			target_cov_mats_[j][f_ix].Scale(1.f / normalization_factor);

			// calculate vertical directions for every horizontal direction
            float interfVAzimuthRadians = interfAzimuthRadians;
            float interfVElevationRadians = 0.f;
			for (int k = 0; k < kNumVInterfDirections; k++) {
				vdelay_sum_masks_[j][k][f_ix].Resize(1, num_input_channels_);
				CovarianceMatrixGenerator::PhaseAlignmentMasks(f_ix, kFftSize,
						sample_rate_hz_, kSpeedOfSoundMeterSeconds,
						array_geometry_, array_delays_, interfVAzimuthRadians,
						interfVElevationRadians,
						&vdelay_sum_masks_[j][k][f_ix]);

				complex_f norm_factor = sqrt(
						ConjugateDotProduct(vdelay_sum_masks_[j][k][f_ix],
								vdelay_sum_masks_[j][k][f_ix]));
				vdelay_sum_masks_[j][k][f_ix].Scale(1.f / norm_factor);

				vtarget_cov_mats_[j][k][f_ix].Resize(num_input_channels_,
						num_input_channels_);
				TransposedConjugatedProduct(vdelay_sum_masks_[j][k][f_ix],
						&vtarget_cov_mats_[j][k][f_ix]);
				complex_f normalization_factor =
						vtarget_cov_mats_[j][k][f_ix].Trace();
				vtarget_cov_mats_[j][k][f_ix].Scale(1.f / normalization_factor);

				interfVElevationRadians -= kVInterfStepRadians; // upper directions
			}

			interfAzimuthRadians += kInterfStepRadians;
		}
	}
}
/*
void BeamLocalizer::InitInterfCovMats() {
	for (int i = 0; i < kNumFreqBins; ++i) {
		ComplexMatrixF uniform_cov_mat(num_input_channels_,
		 num_input_channels_);
		 CovarianceMatrixGenerator::UniformCovarianceMatrix(wave_numbers_[i],
		 array_geometry_, &uniform_cov_mat);
		 // Normalize matrices before averaging them.
		 complex_f normalization_factor = uniform_cov_mat.Trace();
		 uniform_cov_mat.Scale(1.f / normalization_factor);
		 uniform_cov_mat.Scale(1 - kBalance);

		 float interfAzimuthRadians = M_PI / 4;
		 float interfElevationRadians = 0.f;
		 float interfAzimuthRadians2 = - M_PI / 4;
		 float interfElevationRadians2 = 0.f;
		 for (int j = 0; j < kNumInterfDirections; j++) {
		 interf_cov_mats_[j][i].Resize(num_input_channels_,
		 num_input_channels_);
		 CovarianceMatrixGenerator::AngledCovarianceMatrix(
		 kSpeedOfSoundMeterSeconds, interfAzimuthRadians,
		 interfElevationRadians, i, kFftSize, kNumFreqBins,
		 sample_rate_hz_, array_geometry_, &interf_cov_mats_[j][i]);
		 normalization_factor = interf_cov_mats_[j][i].Trace();
		 interf_cov_mats_[j][i].Scale(1.f / normalization_factor);
		 interf_cov_mats_[j][i].Scale(kBalance);
		 interf_cov_mats_[j][i].Add(uniform_cov_mat);

		 interf_cov_mats2_[j][i].Resize(num_input_channels_,
		 num_input_channels_);
		 CovarianceMatrixGenerator::AngledCovarianceMatrix(
		 kSpeedOfSoundMeterSeconds, interfAzimuthRadians2,
		 interfElevationRadians2, i, kFftSize, kNumFreqBins,
		 sample_rate_hz_, array_geometry_, &interf_cov_mats2_[j][i]);
		 normalization_factor = interf_cov_mats2_[j][i].Trace();
		 interf_cov_mats2_[j][i].Scale(1.f / normalization_factor);
		 interf_cov_mats2_[j][i].Scale(kBalance);
		 interf_cov_mats2_[j][i].Add(uniform_cov_mat);

		 interfAzimuthRadians += kInterfStepRadians;
		 interfAzimuthRadians2 += kInterfStepRadians;
		 }

		 interfAzimuthRadians = 0.f;
		 interfElevationRadians = M_PI / 2;
		 interf_cov_mats3_[i].Resize(num_input_channels_, num_input_channels_);
		 CovarianceMatrixGenerator::AngledCovarianceMatrix(
		 kSpeedOfSoundMeterSeconds, interfAzimuthRadians,
		 interfElevationRadians, i, kFftSize, kNumFreqBins,
		 sample_rate_hz_, array_geometry_, &interf_cov_mats3_[i]);
		 normalization_factor = interf_cov_mats3_[i].Trace();
		 interf_cov_mats3_[i].Scale(1.f / normalization_factor);
		 interf_cov_mats3_[i].Scale(kBalance);
		 interf_cov_mats3_[i].Add(uniform_cov_mat);
	}
}
*/
void BeamLocalizer::ProcessChunk(const ChannelBuffer<float>* input,
		ChannelBuffer<float>* output) {
	DCHECK_EQ(input->num_channels(), num_input_channels_);
	DCHECK_EQ(input->num_frames_per_band(), chunk_length_);

	lapped_transform_->ProcessChunk(input->channels(0), output->channels(0));
}

static inline int GetRoundIndex(int idx, int len, int isVertical) {
    if (idx < 0) {
		if (isVertical)
			idx = -idx; // for horizontal same plane arrays
		else
			idx += len;
    }
	else if (idx >= len) {
		if (isVertical)
			idx = len - 1; // not precisely, should be 90+ angles
		else
			idx -= len;
    }
	return idx;
}
    
static inline int GetRoundDistance(int first, int second, int len, int isVertical) {
    int diff = abs(first-second);
    if (isVertical)
        return diff;
    else
        return std::min(diff,len-diff);
}

static inline bool CompareCandiPair(const std::pair<int,float> &first,
                                    const std::pair<int,float> &second) {
    return first.second > second.second;
}

static void CalcCandidates(std::vector<std::pair<int, float>> &candis,
		float *masks, int maskNum, int isVertical) {
	const int kWindowSize = 5; // must be odd
	const int kVWindowSize = 5; // must be odd
	const int kMinCandiSpace = 3;
	const int kMinVCandiSpace = 3;

	int nWindowSize = kWindowSize;
	if (isVertical)
		nWindowSize = kVWindowSize;

	// mask smoothing
	float *smasks = new float[maskNum];
	for (int i = 0; i < maskNum; i++) {
		float avgMask = 0.0f;
		for (int j = i - nWindowSize / 2; j <= i + nWindowSize / 2; j++) {
			int idx = GetRoundIndex(j, maskNum, isVertical);
			avgMask += masks[idx];
		}
		avgMask /= nWindowSize;
		smasks[i] = avgMask;
	}
    
    // find peak masks
	candis.clear();
	for (int i = 0; i < maskNum; i++) {
		if (smasks[i] < kMinCandiAvgMask)
			continue;
		int l2 = i - 2, l1 = i - 1, r1 = i + 1, r2 = i + 2;
		l2 = GetRoundIndex(l2, maskNum, isVertical);
		l1 = GetRoundIndex(l1, maskNum, isVertical);
		r1 = GetRoundIndex(r1, maskNum, isVertical);
		r2 = GetRoundIndex(r2, maskNum, isVertical);
		if (smasks[l2] <= smasks[l1] && smasks[l1] <= smasks[i]
				&& smasks[i] >= smasks[r1] && smasks[r1] >= smasks[r2]) {
            candis.push_back(std::pair<int, float>(i, smasks[i]));
		}
	}
    
    // larger to smaller
    std::sort(candis.begin(), candis.end(), CompareCandiPair);
    
    // remove candidates too close to each other
    int nMinCandiSpace = kMinCandiSpace;
    if (isVertical)
        nMinCandiSpace = kMinVCandiSpace;
    for (int i=1; i<candis.size(); i++) {
        for (int j=0; j<i; j++) {
            if (GetRoundDistance(candis[j].first,candis[i].first,maskNum,isVertical) < nMinCandiSpace) {
                candis.erase(candis.begin()+i);
                i --;
                break;
            }
        }
    }
    
    delete[] smasks;
}

static inline bool CompareCandiConfid(const SCandidate &first,
		const SCandidate &second) {
	return first.confidence > second.confidence;
}

void BeamLocalizer::ProcessAudioBlock(const complex_f* const * input,
		int num_input_channels, int num_freq_bins, int num_output_channels,
		complex_f* const * output) {
	CHECK_EQ(num_freq_bins, kNumFreqBins);
	CHECK_EQ(num_input_channels, num_input_channels_);
	CHECK_EQ(num_output_channels, 1);

	if (block_count_ % kBlockSkipStep) {
        block_count_++;
        return;
    }
    //printf("%d", block_count_);

	// horizontal masks
	CalculatePostfilterMasks(input, num_input_channels, 0, 0, 0);
	for (int i = 0; i < kNumInterfDirections; i++) {
		float avgMask = 0;
		for (int j = low_average_start_bin_; j < high_average_end_bin_; j++)
			avgMask += postfilter_masks_[i][j];
		avgMask /= high_average_end_bin_ - low_average_start_bin_;
		avg_masks_[i] = avgMask;
		//printf("%f ", avgMask);
		//printf("%d\t%d\t%3.2f\t", block_count_*8, i*5, avgMask);
		//for (int j=low_average_start_bin_; j<high_average_end_bin_; j++)
		//    printf("%3.2f ", postfilter_masks_[i][j]);
	}
	//printf("\n");

	std::vector<std::pair<int, float>> hcandis;
	CalcCandidates(hcandis, avg_masks_, kNumInterfDirections, 0);
    //printf("\thcandis: %lu", hcandis.size());

	candidates_.clear();
	for (int i = 0; i < hcandis.size(); i++) {
		// vertical masks
		CalculatePostfilterMasks(input, num_input_channels, 1,
				hcandis[i].first, i);
		for (int i = 0; i < kNumVInterfDirections; i++) {
			float avgMask = 0;
			for (int j = low_average_start_bin_; j < high_average_end_bin_; j++)
				avgMask += postfilter_vmasks_[i][j];
			avgMask /= high_average_end_bin_ - low_average_start_bin_;
			avg_vmasks_[i] = avgMask;
		}

		std::vector<std::pair<int, float>> vcandis;
		CalcCandidates(vcandis, avg_vmasks_, kNumVInterfDirections, 1);

		for (int j = 0; j < vcandis.size(); j++) {
			SCandidate scandi;
			scandi.azimuth = hcandis[i].first;
			scandi.elevation = vcandis[j].first;
			scandi.confidence = vcandis[j].second;
			candidates_.push_back(scandi);
		}
        //printf("\tvcandi: %lu", vcandis.size());
	}
    //printf("\n");

	/*std::sort(candidates_.begin(), candidates_.end(), CompareCandiConfid);
    for (int i=0; i<candidates_.size(); i++) {
        SCandidate &sc = candidates_[i];
        printf("\t%d,%d,%3.2f", sc.azimuth, sc.elevation, sc.confidence);
    }
    printf("\n");*/
    
    // add candidates to sound tracker
    for (int i=0; i<candidates_.size(); i++) {
        SCandidate &sc = candidates_[i];
        R2SoundCandidate    sc2;
        sc2.azimuth = sc.azimuth * kInterfStepRadians;
        sc2.elevation = -sc.elevation * kVInterfStepRadians;
        sc2.confidence = sc.confidence;
        sound_tracker_.AddCandidate(sc2);
    }
    sound_tracker_.Tick();
    sound_tracker_.GetCandidates(st_candidates_);

	block_count_++;
}

void BeamLocalizer::CalculatePostfilterMasks(const complex_f* const * input,
		int num_input_channels, int isVertical, int azimuthIdx, int hcandiIdx) {
	// Calculating the post-filter masks.
	for (int i = low_average_start_bin_; i < high_average_end_bin_; ++i) {
		eig_m_.CopyFromColumn(input, i, num_input_channels_);
		float eig_m_norm_factor = std::sqrt(SumSquares(eig_m_));
		if (eig_m_norm_factor != 0.f) {
			eig_m_.Scale(1.f / eig_m_norm_factor);
		}
        //printf("eig_m_norm_factor %d: %.5f\n", i, eig_m_norm_factor);

		if (isVertical) {
            // vertical 0 degree is equal to horizontal value
            new_vmasks_[0][i] = new_masks_[azimuthIdx][i];
            int vInterfNum = kNumVInterfDirections;
            if (hcandiIdx > 0) // vertical 90 degree has been calculated
                vInterfNum --;
			for (int j = 1; j < vInterfNum; j++) {
				float rxim = Norm(vtarget_cov_mats_[azimuthIdx][j][i], eig_m_);
				float ratio_rxiw_rxim = 0.f;
				if (rxim > 0.f) {
					ratio_rxiw_rxim = vrxiws_[azimuthIdx][j][i] / rxim;
				}

				new_vmasks_[j][i] = 0.f;
				if (ratio_rxiw_rxim > 0.f)
					new_vmasks_[j][i] = 1.f / ratio_rxiw_rxim;
                //new_vmasks_[j][i] *= eig_m_norm_factor * eig_m_norm_factor_scale_;
			}
		} else {
			for (int j = 0; j < kNumInterfDirections; j++) {
				float rxim = Norm(target_cov_mats_[j][i], eig_m_);
				float ratio_rxiw_rxim = 0.f;
				if (rxim > 0.f) {
					ratio_rxiw_rxim = rxiws_[j][i] / rxim;
				}

				//complex_f rmw = abs(ConjugateDotProduct(delay_sum_masks_[j][i], eig_m_));
				//rmw *= rmw;
				//float rmw_r = rmw.real();

				new_masks_[j][i] = 0.f;
				if (ratio_rxiw_rxim > 0.f)
					new_masks_[j][i] = 1.f / ratio_rxiw_rxim;
                //new_masks_[j][i] *= eig_m_norm_factor * eig_m_norm_factor_scale_;
				/*new_masks_[j][i] = CalculatePostfilterMask(interf_cov_mats_[j][i],
				 rpsiws_[j][i],
				 ratio_rxiw_rxim,
				 rmw_r,
				 mask_thresholds_[i]);
				 new_masks_[j][i] *= CalculatePostfilterMask(interf_cov_mats2_[j][i],
				 rpsiws2_[j][i],
				 ratio_rxiw_rxim,
				 rmw_r,
				 mask_thresholds_[i]);
				 new_masks_[j][i] *= CalculatePostfilterMask(interf_cov_mats3_[i],
				 rpsiws3_[j][i],
				 ratio_rxiw_rxim,
				 rmw_r,
				 mask_thresholds_[i]);*/
			}
		}
	}

	ApplyMaskSmoothing(isVertical);
	//ApplyLowFrequencyCorrection();
	//ApplyHighFrequencyCorrection();
	//ApplyMasks(input, output);

	//EstimateTargetPresence();
}
/*
float BeamLocalizer::CalculatePostfilterMask(
		const ComplexMatrixF& interf_cov_mat, float rpsiw,
		float ratio_rxiw_rxim, float rmw_r, float mask_threshold) {
	float rpsim = Norm(interf_cov_mat, eig_m_);

	// Find lambda.
	float ratio = 0.f;
	if (rpsim > 0.f) {
		ratio = rpsiw / rpsim;
	}
	float numerator = rmw_r - ratio;
	float denominator = ratio_rxiw_rxim - ratio;

	float mask = 1.f;
	if (denominator > mask_threshold) {
		float lambda = numerator / denominator;
		mask = std::max(lambda * ratio_rxiw_rxim / rmw_r, kMaskMinimum);
	}
	return mask;
}
*/
/*
 void BeamLocalizer::ApplyMasks(const complex_f* const* input,
 complex_f* const* output) {
 complex_f* output_channel = output[0];
 for (int f_ix = 0; f_ix < kNumFreqBins; ++f_ix) {
 output_channel[f_ix] = complex_f(0.f, 0.f);

 const complex_f* delay_sum_mask_els =
 normalized_delay_sum_masks_[f_ix].elements()[0];
 for (int c_ix = 0; c_ix < num_input_channels_; ++c_ix) {
 output_channel[f_ix] += input[c_ix][f_ix] * delay_sum_mask_els[c_ix];
 }

 output_channel[f_ix] *= postfilter_mask_[f_ix];
 }
 }
 */
void BeamLocalizer::ApplyMaskSmoothing(int isVertical) {
	for (int i = 0; i < kNumFreqBins; ++i) {
		if (isVertical) {
			for (int j = 0; j < kNumVInterfDirections; j++)
				/*postfilter_vmasks_[j][i] = kMaskSmoothAlpha * new_vmasks_[j][i]
						+ (1.f - kMaskSmoothAlpha) * postfilter_vmasks_[j][i];*/
                postfilter_vmasks_[j][i] = new_vmasks_[j][i];
		} else {
			for (int j = 0; j < kNumInterfDirections; j++)
				postfilter_masks_[j][i] = kMaskSmoothAlpha * new_masks_[j][i]
						+ (1.f - kMaskSmoothAlpha) * postfilter_masks_[j][i];
		}
	}
}
/*
 void BeamLocalizer::ApplyLowFrequencyCorrection() {
 float low_frequency_mask = 0.f;
 for (int i = low_average_start_bin_; i < low_average_end_bin_; ++i) {
 low_frequency_mask += postfilter_mask_[i];
 }

 low_frequency_mask /= low_average_end_bin_ - low_average_start_bin_;

 for (int i = 0; i < low_average_start_bin_; ++i) {
 postfilter_mask_[i] = low_frequency_mask;
 }
 }

 void BeamLocalizer::ApplyHighFrequencyCorrection() {
 high_pass_postfilter_mask_ = 0.f;
 for (int i = high_average_start_bin_; i < high_average_end_bin_; ++i) {
 high_pass_postfilter_mask_ += postfilter_mask_[i];
 }

 high_pass_postfilter_mask_ /= high_average_end_bin_ - high_average_start_bin_;

 for (int i = high_average_end_bin_; i < kNumFreqBins; ++i) {
 postfilter_mask_[i] = high_pass_postfilter_mask_;
 }
 }
 */
/*
void BeamLocalizer::EstimateTargetPresence(int azimuthIdx) {
	const int quantile = (1.f - kMaskQuantile) * high_average_end_bin_
			+ kMaskQuantile * low_average_start_bin_;
	std::nth_element(new_masks_[azimuthIdx] + low_average_start_bin_,
			new_masks_[azimuthIdx] + quantile,
			new_masks_[azimuthIdx] + high_average_end_bin_);
	if (new_masks_[azimuthIdx][quantile] > kMaskTargetThreshold) {
		is_target_present_ = true;
		interference_blocks_count_ = 0;
	} else {
		is_target_present_ = false; //interference_blocks_count_++ < hold_target_blocks_;
	}
}*/

void BeamLocalizer::Reset() {
	block_count_ = 0;
	candidates_.clear();
    sound_tracker_.Reset();
}

/*
int BeamLocalizer::GetCandidates(float *pCandidates, int nCandiNum) {
	if (nCandiNum > candidates_.size())
		nCandiNum = (int)candidates_.size();
	for (int i = 0; i < nCandiNum; i++) {
		pCandidates[i*3+0] = candidates_[i].azimuth * kInterfStepRadians;
		pCandidates[i*3+1] = candidates_[i].elevation * kVInterfStepRadians;
        //float confid = candidates_[i].confidence;
        //if (confid > kMaxCandiAvgMask)
        //    confid = kMaxCandiAvgMask;
		//pCandidates[i*3+2] = 100*(confid-kMinCandiAvgMask)/(kMaxCandiAvgMask-kMinCandiAvgMask);
        pCandidates[i*3+2] = candidates_[i].confidence;
	}
    //candidates_.clear();
 
    std::vector<R2SoundCandidate> candis;
    sound_tracker_.GetCandidates(candis);
    if (nCandiNum > candis.size())
        nCandiNum = (int)candis.size();
    for (int i = 0; i < nCandiNum; i++) {
        pCandidates[i*3+0] = candis[i].azimuth;
        pCandidates[i*3+1] = candis[i].elevation;
        pCandidates[i*3+2] = candis[i].confidence;
    }
    
	return nCandiNum;
}
*/

int BeamLocalizer::GetCandidate(int nCandi, R2SoundCandidate &sc) {
    if (nCandi < 0 || nCandi >= st_candidates_.size())
        return -1;
    sc = st_candidates_[nCandi];
    return 0;
}
    
}  // namespace webrtc
