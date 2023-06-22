//
//  zsildet3.h
//  r2vt3
//
//  Created by hadoop on 5/24/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

#ifndef __r2vt2__zsildet3__
#define __r2vt2__zsildet3__

#include "../../zmath.h"
#include "../../mat/zvec.h"

namespace __r2vt4__ {
  
  
  class ZSilDet3
  {
  public:
    ZSilDet3(int iFrmSize, int iFrmNum);
  public:
    ~ZSilDet3(void);
    
  public:
    int     SetSilShield(float fSilShield) ;
    float   SetVtEnergy(float* pData, int iLen) ;
    
    int     Reset();
    bool    CheckLeftSilence(const float* pData, int iLen);
    bool    CheckRightSilence(const float* pData, int iLen);
    
  protected:
    float   GetFrmEnergy(const float* pData);
    float   NormEnergy(float fEnergy);
    void    PrintEnergy();
    
  public:
    int     m_iFrmSize ;
    int     m_iFrmNum ;
    int     m_iFFTSize ;
    float*  m_pHamming ;
    
    
    float   m_fStartShield ;
    float   m_fEndShield ;
    
    float   m_fSilEnergy ;
    float   m_fVtEnergy ;
    float   m_fLeftAvg  ;
    float   m_fLeftMax ;
    float   m_fRightMax ;
    float   m_fSilShield ;
    
    float   m_fLeftSilShield ;
    float   m_fRightSilShield ;
    
    float*  m_pFrmEnergy ;
    
    float*  m_pFFTIn ;
    fftwf_complex * m_pFFTOut ;
    double* m_pFrmEn ;
    fftwf_plan m_fftw_plan ;
    
  };
  
};


#endif /* __r2vt2__zsildet3__ */
