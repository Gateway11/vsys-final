//
//  zvad.cpp
//  r2vt4
//
//  Created by hadoop on 5/15/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zvad.h"

namespace __r2vt4__ {
  
  
  ZVad::ZVad(void){
    
    m_iFrmSize = AUDIO_FRAME_MS * AUDIO_SAMPLE_RATE / 1000 ;
    m_hEngine_Vad = VD_NewVad(1) ;
    
    
    m_iVadState = 0 ;
    
    m_iVadFrmPos = 0 ;
    m_iVadStart = 0 ;
    m_iVadEnd = 0 ;
    m_bDirtyReset = false ;
    
    m_bNeedRestart = false ;
    
    int MaxSpeechFrameNum = 1000 ;
    VD_SetVadParam(m_hEngine_Vad, VD_PARAM_MAXSPEECHFRAMENUM, &MaxSpeechFrameNum) ;
    
    m_iLen_In = 0 ;
    m_iLen_In_Total = AUDIO_SAMPLE_RATE * 5 ;
    m_pData_In = Z_SAFE_NEW_AR1(m_pData_In, float, m_iLen_In_Total);
    
    m_pData_NoNew = Z_SAFE_NEW(m_pData_NoNew, ZVec, AUDIO_SAMPLE_RATE);
    
  }
  
  ZVad::~ZVad(void){
    
    Z_SAFE_DEL_AR1(m_pData_In);
    
    VD_DelVad(m_hEngine_Vad);
    
    Z_SAFE_DEL(m_pData_NoNew) ;
    
  }
  
  int ZVad::AddInData(const float* pData_In, int iLen_In){
    
    if (iLen_In + m_iLen_In > m_iLen_In_Total) {
      m_iLen_In_Total = (iLen_In + m_iLen_In) * 2 ;
      float* pTmp = m_pData_In ;
      m_pData_In = Z_SAFE_NEW_AR1(m_pData_In, float, m_iLen_In_Total);
      memcpy(m_pData_In, pTmp, m_iLen_In * sizeof(float));
      Z_SAFE_DEL_AR1(pTmp);
    }
    
    memcpy(m_pData_In + m_iLen_In, pData_In, iLen_In * sizeof(float));
    m_iLen_In += iLen_In ;
    
    return  0 ;
  }
  
  int ZVad::Process(const float* pData_In, int iLen_In, bool bDirtyReset){
    
    if (m_bNeedRestart) {
      m_bDirtyReset = false ;
      m_iVadState = 0 ;
      m_iVadFrmPos = 0 ;
      m_iVadStart = 0 ;
      m_iVadEnd = 0 ;
      VD_RestartVad(m_hEngine_Vad);
      m_bNeedRestart = false ;
    }
    
    AddInData(pData_In, iLen_In);
    int iFrmNum = m_iLen_In / m_iFrmSize ;
    
    m_iLen_In = 0 ;
    int rt = 0 ;
    for (int i = 0 ; i < iFrmNum ; i ++) {
      m_iVadFrmPos ++ ;
      
      if (m_iVadState == 0){
        VD_InputFloatWave(m_hEngine_Vad, m_pData_In + i * m_iFrmSize, m_iFrmSize, false , false);
        
        int iStartFrame_b = VD_GetVoiceStartFrame(m_hEngine_Vad) ;
        if (iStartFrame_b >= 0){
          m_iVadState = 1 ;
        }
        
      }else{
        
        if (bDirtyReset) {
          m_bDirtyReset = true ;
        }
        
        VD_InputFloatWave(m_hEngine_Vad, m_pData_In + i * m_iFrmSize, m_iFrmSize, false , false);
        
        int iStopFrame_b = VD_GetVoiceStopFrame(m_hEngine_Vad);
        if (iStopFrame_b > 0){
          m_iVadEnd = m_iVadFrmPos -  iStopFrame_b ;
          if (m_iVadEnd < 0) {
            m_iVadEnd = 0 ;
          }
          m_iVadStart = m_iVadEnd + VD_GetVoiceFrameNum(m_hEngine_Vad) ;
          
          rt = 1 ;
          m_bNeedRestart = true ;
          
          i ++ ;
          if (i < iFrmNum) {
            m_iLen_In = (iFrmNum - i) * m_iFrmSize ;
            m_pData_NoNew->CheckMaxSize(m_iLen_In) ;
            memcpy(m_pData_NoNew->data, m_pData_In + i * m_iFrmSize , sizeof(float) * m_iLen_In);
            memcpy(m_pData_In, m_pData_NoNew->data, sizeof(float) * m_iLen_In);
          }
          break ;
        }
      }
    }
    
    return  rt ;
  }
  
  int ZVad::GetVadFrmPos(int& iStart, int &iEnd){
    
    assert(m_iVadState == 1);
    
    iStart = m_iVadStart * m_iFrmSize + m_iLen_In ;
    iEnd = m_iVadEnd * m_iFrmSize + m_iLen_In ;
    
    return  0 ;
    
  }
  
  float  ZVad::getenergy_Lastframe(){
    
    return VD_GetLastFrameEnergy(m_hEngine_Vad);
  }
  
  float  ZVad::getenergy_Threshold(){
    
    return VD_GetThresholdEnergy(m_hEngine_Vad);
  }
  
};




