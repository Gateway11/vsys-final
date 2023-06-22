//
//  zsildet3.cpp
//  r2vt3
//
//  Created by hadoop on 5/24/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

#include "zsildet3.h"
#include "../../zutil.h"

namespace __r2vt4__ {
  
  ZSilDet3::ZSilDet3(int iFrmSize, int iFrmNum){
    
    m_iFrmSize = iFrmSize ;
    m_iFrmNum = iFrmNum ;
    m_iFFTSize = 2 ;
    while (m_iFFTSize < m_iFrmSize) {
      m_iFFTSize = m_iFFTSize * 2 ;
    }
    
    m_pFrmEnergy = Z_SAFE_NEW_AR1(m_pFrmEnergy, float, m_iFrmNum);
    
    m_fStartShield = 1.25f ;
    m_fEndShield = 2.5f ;
    
    m_fVtEnergy = 0 ;
    m_fSilEnergy = 0 ;
    m_fLeftAvg = 0 ;
    m_fLeftMax = 0 ;
    m_fRightMax = 0 ;
    m_fSilShield = 0 ;
    
    float fFrmWin = 0.54f ;
    m_pHamming = Z_SAFE_NEW_AR1(m_pHamming, float, m_iFFTSize);
    for (int i = 0 ; i < m_iFFTSize ; i ++){
      m_pHamming[i] = (float)(fFrmWin - (1 - fFrmWin)*cos(2 * i * 3.14159265f / (m_iFFTSize-1)));
    }
    
    m_pFFTIn = (float*)fftwf_malloc(sizeof(float)*m_iFFTSize);
    m_pFFTOut = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*m_iFFTSize);
    memset(m_pFFTIn, 0, sizeof(float)*m_iFFTSize);
    
    m_fftw_plan = fftwf_plan_dft_r2c_1d(m_iFFTSize, m_pFFTIn, m_pFFTOut, FFTW_ESTIMATE);
    
    m_fLeftSilShield = 0.3f  ;
    m_fRightSilShield = 0.3f  ;
    
    
  }
  
  ZSilDet3::~ZSilDet3(void){
    
    Z_SAFE_DEL_AR1(m_pFrmEnergy) ;
    Z_SAFE_DEL_AR1(m_pHamming) ;
    
    fftwf_free(m_pFFTIn);
    fftwf_free(m_pFFTOut);
    fftwf_destroy_plan(m_fftw_plan);
  }
  
  int ZSilDet3::SetSilShield(float fSilShield){
    
//    if (fSilShield < 300) {
//      fSilShield = 300 ;
//    }
    
    m_fSilShield = fSilShield ;
    
    
    return  0 ;
    
  }
  
  float ZSilDet3::SetVtEnergy(float* pData, int iLen) {
    
    m_fSilEnergy = m_fSilShield * m_fStartShield ;
    m_fVtEnergy = m_fSilShield * m_fEndShield ;
    
    float fVtEnergy = 0.0f ;
    int iFrmNum = (iLen - m_iFFTSize) / m_iFrmSize + 1 ;
    for (int i = 0 ; i < iFrmNum ; i ++) {
      fVtEnergy += GetFrmEnergy(pData + i * m_iFrmSize);
    }
    
    fVtEnergy = fVtEnergy / iFrmNum ;
    
    if (fVtEnergy / 3  > m_fVtEnergy) {
      m_fVtEnergy = fVtEnergy / 3  ;
    }
    ZLOG_INFO("fVtEnergy %f", fVtEnergy);
    
    return  fVtEnergy ;
  }
  
  int ZSilDet3::Reset(){
    
    m_fVtEnergy = 0 ;
    m_fSilEnergy = 0 ;
    m_fLeftAvg = 0 ;
    m_fLeftMax = 0 ;
    m_fRightMax = 0 ;
    m_fSilShield = 0 ;
    memset(m_pFrmEnergy, 0, sizeof(float) * m_iFrmNum);
    
    return  0 ;
  }
  
  bool    ZSilDet3::CheckLeftSilence(const float* pData, int iLen){
    
    assert(iLen % m_iFrmSize == 0) ;
    
    int iFrmNum = (iLen - m_iFFTSize) / m_iFrmSize + 1 ;
    float* pEnergy = Z_SAFE_NEW_AR1(pEnergy, float, iFrmNum);
    float* pEnergyProb = Z_SAFE_NEW_AR1(pEnergyProb, float, iFrmNum);
    
    for (int i = 0 ; i < iFrmNum ; i ++) {
      pEnergy[i] = GetFrmEnergy(pData + i * m_iFrmSize);
      pEnergyProb[i] = NormEnergy(pEnergy[i]);
    }
    
    m_fLeftMax = 0.0f ;
    for (int i = 0 ; i < iFrmNum - m_iFrmNum ; i ++) {
      float fAvgEnergy = 0.0f ;
      for (int j = 0 ; j < m_iFrmNum ; j ++) {
        fAvgEnergy += pEnergyProb[i + j];
      }
      if (fAvgEnergy > m_fLeftMax) {
        m_fLeftMax = fAvgEnergy ;
      }
    }
    m_fLeftMax = m_fLeftMax / m_iFrmNum ;
    
    m_fLeftAvg = 0.0f ;
    for (int i = 0 ; i < iFrmNum ; i ++) {
      m_fLeftAvg += pEnergy[i] ;
    }
    m_fLeftAvg = m_fLeftAvg / iFrmNum ;
    
    PrintEnergy();
    
    Z_SAFE_DEL_AR1(pEnergy);
    Z_SAFE_DEL_AR1(pEnergyProb);
    
    if (m_fLeftMax < m_fLeftSilShield) {
      if (m_fLeftAvg < m_fSilShield) {
        m_fSilShield = m_fLeftAvg ;
        m_fSilEnergy = m_fLeftAvg * m_fStartShield ;
      }
      return  true ;
    }else{
      return  false ;
    }
    
  }
  
  bool  ZSilDet3::CheckRightSilence(const float* pData, int iLen){
    
    assert(iLen % m_iFrmSize == 0) ;
    
    int iFrmNum = (iLen - m_iFFTSize) / m_iFrmSize + 1 ;
    float* pEnergy = Z_SAFE_NEW_AR1(pEnergy, float, iFrmNum);
    float* pEnergyProb = Z_SAFE_NEW_AR1(pEnergyProb, float, iFrmNum);
    
    for (int i = 0 ; i < iFrmNum ; i ++) {
      pEnergy[i] = GetFrmEnergy(pData + i * m_iFrmSize);
      pEnergyProb[i] = NormEnergy(pEnergy[i]);
    }
    
    m_fRightMax = 0.0f ;
    if (iFrmNum > m_iFrmNum) {
      for (int i = 0 ; i < iFrmNum - m_iFrmNum ; i ++) {
        float fAvgEnergy = 0.0f ;
        for (int j = 0 ; j < m_iFrmNum ; j ++) {
          fAvgEnergy += pEnergyProb[i + j];
        }
        if (fAvgEnergy > m_fRightMax) {
          m_fRightMax = fAvgEnergy ;
        }
      }
      m_fRightMax = m_fRightMax / m_iFrmNum ;
      
    }else{
      for (int i = 0 ; i < iFrmNum ; i ++) {
        m_fRightMax += pEnergyProb[i];
      }
      m_fRightMax = m_fRightMax / iFrmNum ;
    }
    
    PrintEnergy() ;
    
    Z_SAFE_DEL_AR1(pEnergy);
    Z_SAFE_DEL_AR1(pEnergyProb);
    
    if (m_fRightMax < m_fRightSilShield ) {
      return  true ;
    }else{
      return  false ;
    }
    
  }
  
  float   ZSilDet3::GetFrmEnergy(const float* pData){
    
    for (int j = 0; j < m_iFFTSize ; j ++) {
      m_pFFTIn[j] = pData[j] * m_pHamming[j] ;
    }
    
    fftwf_execute(m_fftw_plan);
    
    // sub-band energy
    double energy = 0.0 ;
    int nStartBin = 2 ;
    int nStopBin = (m_iFFTSize/2) * 3/4 ; // use 125 ~ 6k (16khz sample rate)
    for (int i = nStartBin; i < nStopBin; i++)
      energy += m_pFFTOut[i][0] * m_pFFTOut[i][0] +
      m_pFFTOut[i][1] * m_pFFTOut[i][1] ;
    
    // use average power
    energy =  sqrtf(energy / m_iFFTSize) ;
    
    return energy ;
    
  }
  
  float ZSilDet3::NormEnergy(float fEnergy){
    
    if (fEnergy < m_fSilEnergy) {
      return  0.0f ;
    }else if(fEnergy > m_fVtEnergy){
      return 1.0f ;
    }else{
      return (fEnergy - m_fSilEnergy) / (m_fVtEnergy - m_fSilEnergy) ;
    }
    
  }
  
  void   ZSilDet3::PrintEnergy(){
    
    ZLOG_INFO("--------------fSilEnergy%f fVtEnergy%f fLeftAvg%f fLeftMax%f fSilShield%f fRightMax%f",
              m_fSilEnergy, m_fVtEnergy, m_fLeftAvg, m_fLeftMax, m_fSilShield, m_fRightMax);
  }
  
};




