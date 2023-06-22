#ifndef R2_MEM_RS_H
#define R2_MEM_RS_H

#include "r2ssp.h"
#include "r2math.h"

namespace __r2ad_offline__ {
  
  class r2mem_rs
  {
  public:
    r2mem_rs(int iMicNum, int iSampleRate, r2_mic_info* pMicInfo_Rs);
  public:
    ~r2mem_rs(void);
    
  public:
    int reset();
    int process(float** pData_In, int iLen_In, float**& pData_Out, int& iLen_Out);
    
  public:
    
    int m_iMicNum ;
    r2_mic_info* m_pMicInfo_Rs ;
    
    int m_iSampleRate_In ;
    int m_iSampleRate_Out ;
    
    int m_iFrmSize_In ;
    int m_iLen_In;
    int m_iLen_In_Total ;
    float ** m_pData_In ;
    
    int m_iFrmSize_Out ;
    int m_iLen_Out ;
    int m_iLen_Out_Total ;
    float** m_pData_Out ;
    
    r2ssp_handle* m_hEngine_Rs ;
  };
  
};

#endif