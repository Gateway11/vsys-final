//
//  zfftr2c.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zfftr2c.h"

namespace __r2vt4__ {
  
  ZFFT_R2C::ZFFT_R2C(int iFFT, int iBat){
    
    m_iFFT = iFFT ;
    m_iBat = iBat ;
    m_in = (float*)fftwf_malloc(sizeof(float)*m_iFFT*m_iBat);
    m_out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*m_iFFT*m_iBat);
    m_fftw3 = fftwf_plan_many_dft_r2c(1,&m_iFFT,m_iBat,
                                      m_in,&m_iFFT,1,m_iFFT,m_out,&m_iFFT,1,m_iFFT,FFTW_ESTIMATE);
  }
  
  ZFFT_R2C::~ZFFT_R2C(void){
    
    fftwf_free(m_in);
    fftwf_free(m_out);
    fftwf_destroy_plan(m_fftw3);
    
  }
  
  int ZFFT_R2C::ExecuteR2C(ZMat* pIn, ZCMat* pOut){
    
    assert(pIn->row == m_iBat);
    assert(pOut->row == m_iBat);
    assert(pIn->col == m_iFFT);
    
    for (int i = 0 ; i < pIn->row ; i ++){
      memcpy(m_in + m_iFFT * i, pIn->data[i],sizeof(float)*m_iFFT);
    }
    fftwf_execute(m_fftw3);
    for (int i = 0 ; i < pOut->row ; i ++){
      memcpy(pOut->data[i] , m_out + i * m_iFFT, sizeof(fftwf_complex) * m_iFFT);
    }
    for (int i = 0 ; i < pOut->row ; i ++) {
      for (int j = 0 ; j < pOut->col; j ++) {
        pOut->data[i][j][0] += 0.000001f ;
      }
    }
    return 0 ;
  }
  
  ZFFT_C2R::ZFFT_C2R(int iFFT, int iBat){
    
    m_iFFT = iFFT ;
    m_iBat = iBat ;
    m_in = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*m_iFFT*m_iBat);
    m_out = (float*)fftwf_malloc(sizeof(float)*m_iFFT*m_iBat);
    m_fftw3 = fftwf_plan_many_dft_c2r(1,&m_iFFT,m_iBat,
                                      m_in,&m_iFFT,1,m_iFFT,m_out,&m_iFFT,1,m_iFFT,FFTW_ESTIMATE);
  }
  
  ZFFT_C2R::~ZFFT_C2R(void){
    
    fftwf_free(m_in);
    fftwf_free(m_out);
    fftwf_destroy_plan(m_fftw3);
  }
  
  int ZFFT_C2R::ExecuteC2R(ZCMat* pIn, ZMat* pOut){
    
    assert(pIn->row == m_iBat);
    assert(pOut->row == m_iBat);
    assert(pIn->col <= m_iFFT);
    
    for (int i = 0 ; i < pIn->row ; i ++){
      memcpy(m_in + m_iFFT * i, pIn->data[i],sizeof(fftwf_complex)*pIn->col);
      memset(m_in + m_iFFT * i + pIn->col, 0, sizeof(fftwf_complex) * (m_iFFT - pIn->col));
    }
    fftwf_execute(m_fftw3);
    for (int i = 0 ; i < pOut->row ; i ++){
      memcpy(pOut->data[i] , m_out + i * m_iFFT, sizeof(float) * m_iFFT);
    }
    return 0 ;
  }
  
};




