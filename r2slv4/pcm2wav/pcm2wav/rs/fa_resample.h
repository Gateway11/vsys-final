#ifndef _FA_RESAMPLE_H
#define _FA_RESAMPLE_H

#include "fa_fir.h"

#ifdef __cplusplus
extern "C"
{
#endif
  
  namespace ZNS_LIBRESAMPLE {
    
#define FA_DEFAULT_FRAMELEN     1024            /* default num of sample in */
#define FA_FRAMELEN_MAX         (160*147+8192)  /* 8192 no meaning, I just set here to safe regarding */
    
#define FA_RATIO_MAX            16
    
    
    
#define MAXV(a, b)	(((a) > (b)) ? (a) : (b))
#define MINV(a, b)	(((a) < (b)) ? (a) : (b))
    
    typedef	struct _fa_polyphase_filter_t {
      float *h;           /* the proto-type lpf confficients */
      float **p;          /* the ployphase cofficients matrix */
      int n;              /* the total number of the proto-type lpf cofficients */
      int m;              /* the number of the phase */
      int k;              /* the number of the subfilter cofficients, k = n/m + 1*/
      
      float gain;
      
    } fa_polyphase_filter_t;
    
    typedef struct _fa_timevary_filter_t {
      float *h;           /* the proto-type lpf confficients */
      float **g;          /* the ployphase cofficients matrix */
      int n;              /* N ,the total number of the proto-type lpf cofficients */
      int l;              /* L ,the number of the phase */
      int m;              /* M ,the deciminator of fractional ratio */
      int k;              /* Q ,the number of the subfilter cofficients, k = n/m + 1*/
      
      float gain;
      
    } fa_timevary_filter_t;
    
    typedef struct _fa_resample_filter_t {
      int L;                  /* interp factor */
      int M;                  /* decimate factor */
      float fc;               /* cutoff freqency, normalized */
      
      fa_polyphase_filter_t ppflt;
      fa_timevary_filter_t  tvflt;
      float gain;
      
      int bytes_per_sample;
      int num_in;             /* number of sample in */
      int num_out;            /* number of sample out */
      int out_index;
      int bytes_in;
      int bytes_out;
      
      int buf_len;
      unsigned char *buf;
      
    } fa_resample_filter_t;
    
    
    
    uintptr_t fa_decimate_init(int M, float gain, win_t win_type);
    void      fa_decimate_uninit(uintptr_t handle);
    
    uintptr_t fa_interp_init(int L, float gain, win_t win_type);
    void      fa_interp_uninit(uintptr_t handle);
    
    uintptr_t fa_resample_filter_init(int L, int M, float gain, win_t win_type);
    void      fa_resample_filter_uninit(uintptr_t handle);
    
    int fa_get_resample_framelen_bytes(uintptr_t handle);
    int fa_decimate(uintptr_t handle, unsigned char *sample_in, int sample_in_size,
                    unsigned char *sample_out, int *sample_out_size);
    int fa_interp(uintptr_t handle, unsigned char *sample_in, int sample_in_size,
                  unsigned char *sample_out, int *sample_out_size);
    int fa_resample(uintptr_t handle, unsigned char *sample_in, int sample_in_size,
                    unsigned char *sample_out, int *sample_out_size);
    
  }
  
#ifdef __cplusplus
}
#endif



#endif
