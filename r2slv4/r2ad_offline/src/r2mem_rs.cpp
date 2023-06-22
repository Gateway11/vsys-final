#include "r2mem_rs.h"

namespace __r2ad_offline__ {
  
  r2mem_rs::r2mem_rs(int iMicNum, int iSampleRate, r2_mic_info* pMicInfo_Rs){
    
    m_iMicNum = iMicNum ;
    m_pMicInfo_Rs = pMicInfo_Rs ;
    
    //data
    m_iSampleRate_In = iSampleRate ;
    m_iFrmSize_In = R2_AUDIO_FRAME_MS * m_iSampleRate_In / 1000 ;
    m_iLen_In = 0 ;
    m_iLen_In_Total = m_iFrmSize_In ;
    m_pData_In = R2_SAFE_NEW_AR2(m_pData_In, float, m_iMicNum, m_iLen_In_Total);
    
    m_iSampleRate_Out = R2_AUDIO_SAMPLE_RATE ;
    m_iFrmSize_Out = R2_AUDIO_FRAME_MS * m_iSampleRate_Out / 1000 ;
    m_iLen_Out = 0 ;
    m_iLen_Out_Total = R2_AUDIO_SAMPLE_RATE ;
    m_pData_Out = R2_SAFE_NEW_AR2(m_pData_Out, float, m_iMicNum, m_iLen_Out_Total);
    
    //rs
    m_hEngine_Rs = R2_SAFE_NEW_AR1(m_hEngine_Rs,r2ssp_handle,m_iMicNum);
    if (m_iSampleRate_In != m_iSampleRate_Out) {
      for (int i = 0 ; i < m_pMicInfo_Rs->iMicNum ; i ++){
        int iMicId = m_pMicInfo_Rs->pMicIdLst[i] ;
        m_hEngine_Rs[iMicId] = r2ssp_rs_create();
        r2ssp_rs_init(m_hEngine_Rs[iMicId],m_iSampleRate_In,m_iSampleRate_Out);
      }
    }
  }
  
  r2mem_rs::~r2mem_rs(void)
  {
    for (int i = 0 ; i < m_iMicNum ; i ++){
      if (m_hEngine_Rs[i] != 0) {
        r2ssp_rs_free(m_hEngine_Rs[i]);
      }
    }
    R2_SAFE_DEL_AR1(m_hEngine_Rs);
    
    R2_SAFE_DEL_AR2(m_pData_In);
    R2_SAFE_DEL_AR2(m_pData_Out);
    
  }
  
  int r2mem_rs::process(float** pData_In, int iLen_In, float**& pData_Out, int& iLen_Out){
    
    assert(iLen_In == 0 || (iLen_In > 0 && pData_In != NULL)) ;
    R2_MEM_ASSERT(this,0);
    
    int iFrmNum = (iLen_In + m_iLen_In) / m_iFrmSize_In ;
    if (iFrmNum * m_iFrmSize_Out > m_iLen_Out_Total) {
      m_iLen_Out_Total = iFrmNum * m_iFrmSize_Out * 2 ;
      R2_SAFE_DEL_AR2(m_pData_Out);
      m_pData_Out = R2_SAFE_NEW_AR2(m_pData_Out, float, m_iMicNum, m_iLen_Out_Total);
    }
    
    m_iLen_Out = 0 ;
    
    int iCur = 0 ;
    while (iCur < iLen_In) {
      int ll = r2_min(iLen_In - iCur , m_iLen_In_Total - m_iLen_In) ;
      for (int i = 0 ; i < m_pMicInfo_Rs->iMicNum ; i ++) {
        int iMicId = m_pMicInfo_Rs->pMicIdLst[i] ;
        memcpy(m_pData_In[iMicId] + m_iLen_In, pData_In[iMicId] + iCur, sizeof(float) * ll) ;
      }
      iCur += ll ;
      m_iLen_In += ll ;
      if (m_iLen_In == m_iLen_In_Total) {
        
        if (m_iSampleRate_In == m_iSampleRate_Out) {
          for (int i = 0 ; i < m_pMicInfo_Rs->iMicNum ; i ++) {
            int iMicId = m_pMicInfo_Rs->pMicIdLst[i] ;
            memcpy(m_pData_Out[iMicId] + m_iLen_Out, m_pData_In[iMicId], sizeof(float) * m_iFrmSize_Out) ;
          }
        }else{
          for (int i = 0 ; i < m_pMicInfo_Rs->iMicNum ; i ++) {
            int iMicId = m_pMicInfo_Rs->pMicIdLst[i] ;
            r2ssp_rs_process(m_hEngine_Rs[iMicId], m_pData_In[iMicId],
                             m_iFrmSize_In , m_pData_Out[iMicId] + m_iLen_Out , m_iFrmSize_Out);
          }
        }
        m_iLen_Out += m_iFrmSize_Out ;
        m_iLen_In = 0 ;
      }
    }
    
    pData_Out = m_pData_Out ;
    iLen_Out = m_iLen_Out ;
    
    return 0 ;
  }
  
  int r2mem_rs::reset(){
    
    m_iLen_In = 0 ;
    m_iLen_Out = 0 ;
    return 0 ;
  }
  
};