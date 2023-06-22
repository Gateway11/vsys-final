//
//  zvt.cpp
//  r2vt4
//
//  Created by hadoop on 3/19/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zvt.h"

namespace __r2vt4__ {
  
  ZVt::ZVt(z_mic_info* pMicInfo_Vt){
    
    m_pMicInfo_Vt = z_copymicinfo(pMicInfo_Vt);
    
    m_hEngine_Vt = r2_vt_create(m_pMicInfo_Vt->iMicNum);
    
    m_iFrmSize = AUDIO_SAMPLE_RATE / 1000 * AUDIO_FRAME_MS ;
    m_iFrmBatchNum = 12 ;
    m_iBatchSize = m_iFrmBatchNum * m_iFrmSize ;
    
    m_iLen_In_Total = m_iBatchSize * 10 ;
    m_iLen_In = 0 ;
    m_pData_In = Z_SAFE_NEW_AR2(m_pData_In, float, m_pMicInfo_Vt->iMicNum, m_iLen_In_Total) ;
  }

  ZVt::~ZVt(void){
    
    r2_vt_free(m_hEngine_Vt) ;
    
    Z_SAFE_DEL_AR2(m_pData_In) ;
    
    z_free_micinfo(m_pMicInfo_Vt) ;
    
  }
  
  int ZVt::SetWords(const WordInfo* pWordLst, int iWordNum){
    
    return  r2_vt_setwords(m_hEngine_Vt, pWordLst, iWordNum) ;
  }
  
  int ZVt::GetWords(const WordInfo** pWordLst, int* iWordNum){
    
    return  r2_vt_getwords(m_hEngine_Vt, pWordLst, iWordNum) ;
  }

  int ZVt::Process(const float** pWavBuff, int iWavLen, int iVtFlag){
    
    if (iWavLen == 0) {
      return  0 ;
    }
    
    if (m_iLen_In >= m_iBatchSize) {
      int iLeftSize = m_iLen_In % m_iBatchSize ;
      if (iLeftSize > 0) {
        for (int i = 0 ; i < m_pMicInfo_Vt->iMicNum; i ++) {
          memcpy(m_pData_In[i], m_pData_In[i] + m_iLen_In - iLeftSize, iLeftSize * sizeof(float)) ;
        }
      }
      m_iLen_In = iLeftSize ;
    }
    
    if (m_iLen_In + iWavLen >  m_iLen_In_Total ) {
      m_iLen_In_Total = (m_iLen_In + iWavLen) * 2 ;
      float** pTmp = Z_SAFE_NEW_AR2(pTmp, float, m_pMicInfo_Vt->iMicNum, m_iLen_In_Total) ;
      if (m_iLen_In > 0) {
        for (int i = 0 ; i < m_pMicInfo_Vt->iMicNum; i ++) {
          memcpy(pTmp[i], m_pData_In[i], m_iLen_In * sizeof(float)) ;
        }
      }
      Z_SAFE_DEL_AR2(m_pData_In) ;
      m_pData_In = pTmp ;
    }
    
    for (int i = 0 ; i < m_pMicInfo_Vt->iMicNum; i ++) {
      memcpy(m_pData_In[i] + m_iLen_In, pWavBuff[m_pMicInfo_Vt->pMicIdLst[i]], iWavLen * sizeof(float)) ;
    }
    m_iLen_In += iWavLen ;
    
    return r2_vt_process(m_hEngine_Vt, (const float**)m_pData_In, m_iLen_In / m_iBatchSize * m_iBatchSize, iVtFlag);
    
  }

  int ZVt::GetDetWordInfo(const WordInfo** pWordInfo, const WordDetInfo** pWordDetInfo){

    return r2_vt_getdetwordinfo(m_hEngine_Vt, pWordInfo, pWordDetInfo, (m_iLen_In % m_iBatchSize) / m_iFrmSize ) ;
  }
  
  int ZVt::Reset(){
    
    return  r2_vt_reset(m_hEngine_Vt) ;
  }

  
  int ZVt::GetLeftPos(){
    
    return m_iLen_In % m_iBatchSize ;
  }
  
  int ZVt::SetCallback_SecSl(dosecsl_callback fun_cb, void* param_cb){
    
    return  r2_vt_set_dosecsl_cb(m_hEngine_Vt, fun_cb, param_cb) ;
  }

  int ZVt::SetCallback_CheckDirty(checkdirty_callback fun_cb, void* param_cb){
    
    return  r2_vt_set_checkdirty_cb(m_hEngine_Vt, fun_cb, param_cb) ;
  }
  
  int ZVt::SetCallback_GetBfData(getdata_callback fun_cb, void* param_cb){
    
    return  r2_vt_set_getdata_cb(m_hEngine_Vt, fun_cb, param_cb) ;
  }

};




