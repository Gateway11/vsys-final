//
//  zbf.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zbf.h"

namespace __r2vt4__ {
  
  ZBf::ZBf(int iMicNum, float* pMicPosLst, float* pMicDelay, z_mic_info* pMicInfo_Bf){
    
    m_iMicNum = iMicNum ;
    m_iFrmSize = AUDIO_SAMPLE_RATE / 1000 * AUDIO_FRAME_MS ;
    m_pMicInfo_Bf = z_copymicinfo(pMicInfo_Bf) ;
    
    m_pMics_Bf = Z_SAFE_NEW_AR1(m_pMics_Bf,float,m_pMicInfo_Bf->iMicNum * 3);
    m_pMicI2sDelay = Z_SAFE_NEW_AR1(m_pMicI2sDelay, float, m_pMicInfo_Bf->iMicNum) ;
    
    for (int i = 0 ; i < m_pMicInfo_Bf->iMicNum ; i ++)	{
      int iMicId = m_pMicInfo_Bf->pMicIdLst[i] ;
      memcpy(m_pMics_Bf + i * 3 , pMicPosLst + iMicId * 3 , sizeof(float) * 3) ;
      m_pMicI2sDelay[i] = pMicDelay[iMicId];
    }
    
    
    
    if (m_pMicInfo_Bf->iMicNum > 1) {
      m_hEngine_Bf = r2ssp_bf_create(m_pMics_Bf, m_pMicInfo_Bf->iMicNum);
      r2ssp_bf_init(m_hEngine_Bf,AUDIO_FRAME_MS,AUDIO_SAMPLE_RATE);
      r2ssp_bf_set_mic_delays(m_hEngine_Bf, m_pMicI2sDelay, m_pMicInfo_Bf->iMicNum);
    }else{
      m_hEngine_Bf = NULL ;
    }
    
    memset(m_fSlInfo, 0, sizeof(float) * 3);
    Steer(m_fSlInfo[0], m_fSlInfo[1]);
    
    
    m_pData_Bf = Z_SAFE_NEW_AR1(m_pData_Bf,float,m_pMicInfo_Bf->iMicNum * m_iFrmSize);
    
    
    m_iLen_Out_Total = AUDIO_SAMPLE_RATE ;
    m_pData_Out = Z_SAFE_NEW_AR1(m_pData_Out, float, m_iLen_Out_Total);
  }
  
  ZBf::~ZBf(void)
  {
    
    Z_SAFE_DEL_AR1(m_pData_Out);
    
    Z_SAFE_DEL_AR1(m_pData_Bf);
    
    Z_SAFE_DEL_AR1(m_pMics_Bf);
    
    Z_SAFE_DEL_AR1(m_pMicI2sDelay);
    
    r2ssp_bf_free(m_hEngine_Bf);
    
    z_free_micinfo(m_pMicInfo_Bf);
  }
  
  int ZBf::Reset(){
    
    if (m_pMicInfo_Bf->iMicNum > 1) {
      r2ssp_bf_free(m_hEngine_Bf);
      m_hEngine_Bf = r2ssp_bf_create(m_pMics_Bf, m_pMicInfo_Bf->iMicNum);
      r2ssp_bf_init(m_hEngine_Bf,AUDIO_FRAME_MS,AUDIO_SAMPLE_RATE);
      r2ssp_bf_set_mic_delays(m_hEngine_Bf, m_pMicI2sDelay, m_pMicInfo_Bf->iMicNum);
    }
    memset(m_fSlInfo, 0, sizeof(float) * 3);
    Steer(m_fSlInfo[0], m_fSlInfo[1]);
    
    return 0 ;
  }
  
  int ZBf::Process(const float** pData_In, int iLen_In, float*& pData_Out, int& iLen_Out){
    
    assert(iLen_In == 0 || (iLen_In > 0 && pData_In != NULL)) ;
    
    if (iLen_In > m_iLen_Out_Total) {
      m_iLen_Out_Total = iLen_In * 2 ;
      Z_SAFE_DEL_AR1(m_pData_Out);
      m_pData_Out = Z_SAFE_NEW_AR1(m_pData_Out, float, m_iLen_Out_Total);
    }
    
    if (m_pMicInfo_Bf->iMicNum > 1) {
      for (int i = 0 ; i < iLen_In ; i += m_iFrmSize){
        for (int j = 0 ; j < m_pMicInfo_Bf->iMicNum ; j ++){
          int iMicId = m_pMicInfo_Bf->pMicIdLst[j];
          memcpy(m_pData_Bf + j * m_iFrmSize, pData_In[iMicId] + i, sizeof(float) * m_iFrmSize);
        }
        r2ssp_bf_process(m_hEngine_Bf, m_pData_Bf, m_iFrmSize * m_pMicInfo_Bf->iMicNum, m_pMicInfo_Bf->iMicNum, m_pData_Out + i);
      }
    }else{
      for (int i = 0 ; i < iLen_In ; i += m_iFrmSize){
        int iMicId = m_pMicInfo_Bf->pMicIdLst[0];
        memcpy(m_pData_Out + i, pData_In[iMicId] + i, sizeof(float) * m_iFrmSize);
      }
    }
    
    iLen_Out = iLen_In ;
    pData_Out = m_pData_Out ;
    
    return  0 ;
  }
  
  int ZBf::Steer(float fAzimuth, float fElevation, int bSteer){
    
    m_fSlInfo[0] = fAzimuth ;
    m_fSlInfo[1] = fElevation ;
    
    if (m_pMicInfo_Bf->iMicNum > 1 && bSteer > 0) {
      r2ssp_bf_steer(m_hEngine_Bf, m_fSlInfo[0], m_fSlInfo[1], m_fSlInfo[0]  + 3.1415926f, 0);
    }
    
    return 0 ;
  }
  
  const char* ZBf::GetSlInfo(){
    
    char info[256];
    int iAzimuth = (m_fSlInfo[0] - 3.1415936f) * 180 / 3.1415936f + 0.1f ;
    while (iAzimuth < 0) {
      iAzimuth += 360 ;
    }
    while (iAzimuth >= 360) {
      iAzimuth -= 360 ;
    }
    
    sprintf(info,"%5f %5f",(float)iAzimuth , m_fSlInfo[1] * 180 / 3.1415936f);
    m_strSlInfo = info ;
    return  m_strSlInfo.c_str() ;
    
  }
  
};




