//
//  zns.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zns.h"

namespace __r2vt4__ {
  
  ZNs::ZNs(int iMicNum, z_mic_info* pMicInfo_Ns, int iNsMode){
    
    m_iMicNum = iMicNum ;
    m_pMicInfo_Ns = pMicInfo_Ns ;
    m_iFrmSize = 160 ;
    
    m_pHandle = Z_SAFE_NEW_AR1(m_pHandle, r2ssp_handle, pMicInfo_Ns->iMicNum);
    
    for (int i = 0 ; i < m_pMicInfo_Ns->iMicNum ; i ++) {
      m_pHandle[i] = r2ssp_ns_create();
      r2ssp_ns_init(m_pHandle[i], 16000);
      r2ssp_ns_set_mode(m_pHandle[i], iNsMode);
    }
    
    m_iLen_Out_Total = m_iFrmSize * 10 ;
    m_pData_Out = Z_SAFE_NEW_AR2(m_pData_Out, float, m_iMicNum, m_iLen_Out_Total);
    
  }
  
  ZNs::~ZNs(void){
    
    for (int i = 0 ; i < m_pMicInfo_Ns->iMicNum ; i ++) {
      r2ssp_ns_free(m_pHandle[i]);
    }
    
    Z_SAFE_DEL_AR1(m_pHandle);
    Z_SAFE_DEL_AR2(m_pData_Out);
    
  }
  
  int ZNs::Process(const float** pData_In, int iLen_In, float** &pData_Out, int &iLen_Out){
    
    assert(iLen_In % m_iFrmSize == 0) ;
    
    if (iLen_In > m_iLen_Out_Total ) {
      Z_SAFE_DEL_AR2(m_pData_Out);
      m_iLen_Out_Total = iLen_In * 2 ;
      m_pData_Out = Z_SAFE_NEW_AR2(m_pData_Out, float, m_iMicNum, m_iLen_Out_Total);
    }
    
    int iFrmNum = iLen_In / m_iFrmSize ;
    for (int i = 0 ; i < m_pMicInfo_Ns->iMicNum ; i ++) {
      int iMicId = m_pMicInfo_Ns->pMicIdLst[i] ;
      for (int j = 0 ; j < iFrmNum ; j ++) {
        r2ssp_ns_process(m_pHandle[i], pData_In[iMicId] + j * m_iFrmSize , m_pData_Out[iMicId] + j * m_iFrmSize);
      }
    }
    
    pData_Out = m_pData_Out ;
    iLen_Out = iLen_In ;
    
    return  0 ;
  }

};




