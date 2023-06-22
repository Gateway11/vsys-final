//
//  r2mem_vad3.cpp
//  r2ad_offline
//
//  Created by hadoop on 8/1/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

#include "r2mem_vad3.h"
#include "r2math.h"

namespace __r2ad_offline__ {

  r2mem_vad3::r2mem_vad3(int iMicNum, int iMicId_Vad)
  {
    m_iMicNum = iMicNum ;
    m_iMicId_Vad = iMicId_Vad ;
    
    m_iFrmSize = R2_AUDIO_SAMPLE_RATE / 1000 * R2_AUDIO_FRAME_MS ;

    m_iLen_Out = 0 ;
    m_iLen_Out_Total = m_iFrmSize * 100 ;
    m_pData_Out = R2_SAFE_NEW_AR2(m_pData_Out,float,iMicNum,m_iLen_Out_Total);

    m_iVadState = 0 ;
    
    m_hEngine_Vad = VD_NewVad(2);
    
    //int nMinSilFrameNum = 80;
    //VD_SetVadParam(m_hEngine_Vad, VD_PARAM_MINSILFRAMENUM, &nMinSilFrameNum);
    
  }
  
  r2mem_vad3::~r2mem_vad3(void){
    
    R2_SAFE_DEL_AR2(m_pData_Out);
    VD_DelVad(m_hEngine_Vad);
  }
  
  int r2mem_vad3::process(float** pData_In, int iLen_In, float**& pData_Out, int& iLen_Out){
    
    assert(iLen_In == 0 || (iLen_In > 0 && pData_In != NULL)) ;
    R2_MEM_ASSERT(this,0);
    
    assert(iLen_In % m_iFrmSize == 0) ;
    
    int iFrmNum = iLen_In / m_iFrmSize ;
    
    m_iLen_Out = 0 ;
    int iStartFrame_b = -1 , iStopFrame_b = -1 ;
    
    for (int i = 0 ; i < iFrmNum ; i ++ ) {
      //Store to Block
      if (i == iFrmNum - 1) {
        VD_InputFloatWave(m_hEngine_Vad, pData_In[m_iMicId_Vad] + i * m_iFrmSize, m_iFrmSize, true, false) ;
      }else{
        VD_InputFloatWave(m_hEngine_Vad, pData_In[m_iMicId_Vad] + i * m_iFrmSize, m_iFrmSize, false, false) ;
      }
      
      if (m_iVadState == 0){
        iStartFrame_b = VD_GetVoiceStartFrame(m_hEngine_Vad);
        if (iStartFrame_b >= 0){
          m_iVadState = 1 ;
        }
      }else{
        iStopFrame_b = VD_GetVoiceStopFrame(m_hEngine_Vad);
        if (iStopFrame_b > 0){
          
          AddOutData(pData_In, m_iFrmSize * iStartFrame_b, (iStopFrame_b - iStartFrame_b) * m_iFrmSize) ;
          VD_RestartVad(m_hEngine_Vad);
          m_iVadState = 0 ;
          iStartFrame_b = -1 ;
          iStopFrame_b = -1 ;

        }
      }
    }
    
    pData_Out = m_pData_Out ;
    iLen_Out = m_iLen_Out ;
    
    return 0 ;
    
  }
  
  int r2mem_vad3::reset(){
    
    VD_RestartVad(m_hEngine_Vad);
    m_iVadState = 0 ;
    
    return 0 ;
  }
  

  int r2mem_vad3::AddOutData(float** pData_Out, int iOffset, int iLen_Out){
    
    if (m_iLen_Out + iLen_Out > m_iLen_Out_Total ) {
      m_iLen_Out_Total = (iLen_Out + m_iLen_Out) * 2 ;
      float** pTmp = m_pData_Out ;
      m_pData_Out = R2_SAFE_NEW_AR2(m_pData_Out, float, m_iMicNum, m_iLen_Out_Total);
      for (int i = 0 ; i < m_iMicNum; i ++) {
        memcpy(m_pData_Out[i], pTmp[i], sizeof(float) * m_iLen_Out);
      }
      R2_SAFE_DEL_AR2(pTmp);
    }
    
    for (int i = 0 ; i < m_iMicNum ; i ++) {
      memcpy(m_pData_Out[i] + m_iLen_Out, pData_Out[i] + iOffset, iLen_Out * sizeof(float));
    }
    m_iLen_Out += iLen_Out ;
    
    return  0 ;
    
    
  }
};




