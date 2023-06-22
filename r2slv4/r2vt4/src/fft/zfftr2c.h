//
//  zfftr2c.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zfftr2c__
#define __r2vt4__zfftr2c__

#include "../zmath.h"
#include "../mat/zmat.h"
#include "../mat/zcmat.h"

namespace __r2vt4__ {
  
  class ZFFT_R2C
  {
  public:
    ZFFT_R2C(int iFFT, int iBat);
  public:
    ~ZFFT_R2C(void);
    
    int ExecuteR2C(ZMat* pIn, ZCMat* pOut);
    
  protected:
    
    int m_iFFT ;
    int m_iBat ;
    
    float* m_in ;
    fftwf_complex * m_out ;
    
    fftwf_plan m_fftw3 ;
    
  };
  
  class ZFFT_C2R
  {
  public:
    ZFFT_C2R(int iFFT, int iBat);
  public:
    ~ZFFT_C2R(void);
    
    int ExecuteC2R(ZCMat* pIn, ZMat* pOut);
    
  protected:
    
    int m_iFFT ;
    int m_iBat ;
    
    float* m_out ;
    fftwf_complex * m_in ;
    
    fftwf_plan m_fftw3 ;
    
  };
  
};

#endif /* __r2vt4__zfftr2c__ */
