//
//  zfftr2r.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zfftr2r__
#define __r2vt4__zfftr2r__

#include "../zmath.h"
#include "../mat/zvec.h"
#include "../mat/zmat.h"

namespace __r2vt4__ {
  
  class ZFFT
  {
  public:
    ZFFT(int iFFT, int iBat);
  public:
    ~ZFFT(void);
    
    int Execute(ZMat* pIn, ZMat* pOut);
    int Execute(ZMat* pIn, ZVec* pOut);
    
  protected:
    
    int m_iFFT ;
    int m_iBat ;
    float* m_in ;
    fftwf_complex * m_out ;
    fftwf_plan m_fftw3 ;
  };
  
  class ZFFTS
  {
  public:
    ZFFTS(int iFFT);
  public:
    ~ZFFTS(void);
    
    int Execute(ZVec* pIn, ZVec* pOut);
    
  protected:
    
    int m_iFFT ;
    float* m_in ;
    fftwf_complex * m_out ;
    
    fftwf_plan m_fftw3 ;
  };
  
};


#endif /* __r2vt4__zfftr2r__ */
