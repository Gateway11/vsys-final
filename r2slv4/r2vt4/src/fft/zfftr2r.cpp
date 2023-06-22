//
//  zfftr2r.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zfftr2r.h"

namespace __r2vt4__ {
  
  ZFFT::ZFFT(int iFFT, int iBat){
    
    //fftwf_plan fftwf_plan_many_dft_r2c(int rank, const int *n, int howmany,
    //float* in, const int *inembed, int istride, int idist,
    //fftwf_complex *out, const int *onembed, int ostride, int odist, unsigned flags);
    m_iFFT = iFFT ;
    m_iBat = iBat ;
    m_in = (float*)fftwf_malloc(sizeof(float)*m_iFFT*m_iBat);
    m_out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*m_iFFT*m_iBat);
    m_fftw3 = fftwf_plan_many_dft_r2c(1,&m_iFFT,m_iBat,
                                      m_in,&m_iFFT,1,m_iFFT,m_out,&m_iFFT,1,m_iFFT,FFTW_ESTIMATE);
  }
  
  ZFFT::~ZFFT(void)
  {
    fftwf_free(m_in);
    fftwf_free(m_out);
    fftwf_destroy_plan(m_fftw3);
  }
  
  int ZFFT::Execute(ZMat* pIn, ZMat* pOut){
    
    assert(pIn->row == m_iBat);
    assert(pOut->row == m_iBat);
    assert(pIn->col == m_iFFT);
    
    for (int i = 0 ; i < pIn->row ; i ++){
      memcpy(m_in + m_iFFT * i, pIn->data[i],sizeof(float)*m_iFFT);
    }
    fftwf_execute(m_fftw3);
    for (int i = 0 ; i < pOut->row ; i ++){
      for (int j = 0 ; j < m_iFFT / 2 + 1 ; j ++){
        pOut->data[i][j] = m_out[i*m_iFFT+j][0] * m_out[i*m_iFFT+j][0] + m_out[i*m_iFFT+j][1] * m_out[i*m_iFFT+j][1];
      }
    }
    return 0 ;
  }
  
  int ZFFT::Execute(ZMat* pIn, ZVec* pOut){
    
    assert(pIn->row == m_iBat);
    assert(pIn->col <= m_iFFT);
    
    memset(m_in, 0, sizeof(float) * m_iBat * m_iFFT);
    for (int i = 0 ; i < pIn->row ; i ++){
      memcpy(m_in + m_iFFT * i, pIn->data[i],sizeof(float)*pIn->col);
    }
    
    fftwf_execute(m_fftw3);
    
    pOut->Clean() ;
    
    for (int i = 0 ; i < pIn->row ; i ++){
      for (int j = 0 ; j < m_iFFT / 2 + 1 ; j ++){
        pOut->data[j] += m_out[i*m_iFFT+j][0] * m_out[i*m_iFFT+j][0] + m_out[i*m_iFFT+j][1] * m_out[i*m_iFFT+j][1];
      }
    }
    
    for (int j = 0 ; j < m_iFFT / 2 + 1; j ++) {
      pOut->data[j] = pOut->data[j] / pIn->row ;
    }
    return 0 ;
    
  }
  
  
  ZFFTS::ZFFTS(int iFFT){
    
    //fftwf_complex *out, const int *onembed, int ostride, int odist, unsigned flags);
    m_iFFT = iFFT ;
    m_in = (float*)fftwf_malloc(sizeof(float)*m_iFFT);
    m_out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*m_iFFT);
    m_fftw3 = fftwf_plan_dft_r2c_1d(m_iFFT, m_in, m_out, FFTW_ESTIMATE);
  }
  
  ZFFTS::~ZFFTS(void)
  {
    fftwf_free(m_in);
    fftwf_free(m_out);
    fftwf_destroy_plan(m_fftw3);
  }
  
  int ZFFTS::Execute(ZVec* pIn, ZVec* pOut){
    
    assert(pIn->size <= m_iFFT);
    assert(pOut->size == m_iFFT / 2 + 1);
    
    memcpy(m_in, pIn->data,sizeof(float)*pIn->size);
    memset(m_in + pIn->size , 0 , sizeof(float) * (m_iFFT - pIn->size));
    
    
    fftwf_execute(m_fftw3);
    
    for (int j = 0 ; j < m_iFFT / 2 + 1 ; j ++){
      pOut->data[j] = m_out[j][0] * m_out[j][0] + m_out[j][1] * m_out[j][1];
    }
    
    return 0 ;
  }
  
  
};



