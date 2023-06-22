//
//  zrsmem2.cpp
//  r2vt4
//
//  Created by hadoop on 6/1/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zrsmem2.h"

namespace __r2vt4__ {

  ZRsMem2::ZRsMem2(int iCn, int iSrIn, int iSrOut){
    
    m_iCn = iCn ;
    
    m_iSr_In = iSrIn ;
    m_iFrmSize_In = iSrIn * 10 / 1000 ;
    m_iLen_In = 0 ;
    m_pMat_In = Z_SAFE_NEW(m_pMat_In, ZMat, m_iCn, m_iFrmSize_In) ;
    
    m_iSr_Out = iSrOut ;
    m_iFrmSize_Out = iSrOut * 10 / 1000 ;
    m_iLen_Out = 0 ;
    m_pMat_Out = Z_SAFE_NEW(m_pMat_Out, ZMat, m_iCn, m_iSr_Out * 5) ;
    
    
    
    m_hEngine_Rs = Z_SAFE_NEW_AR1(m_hEngine_Rs,r2ssp_handle,m_iCn);
    for (int i = 0 ; i < m_iCn ; i ++){
      m_hEngine_Rs[i] = r2ssp_rs_create();
      r2ssp_rs_init(m_hEngine_Rs[i],m_iSr_In,m_iSr_Out);
      
    }
    
  }
  
  ZRsMem2::~ZRsMem2(void){
    
    
    Z_SAFE_DEL(m_pMat_In) ;
    Z_SAFE_DEL(m_pMat_Out) ;
    
    for (int i = 0 ; i < m_iCn ; i ++) {
      r2ssp_rs_free(m_hEngine_Rs[i]) ;
    }
    Z_SAFE_DEL_AR1(m_hEngine_Rs);
  }
  

  int ZRsMem2::Reset(){
    
    m_iLen_In = 0 ;
    return  0 ;
    
  }
  
  int ZRsMem2::Process(const float** pWavIn, const int iLenIn, float** &pWavOut, int &iLenOut){
    
    int iFrmNum = (iLenIn + m_iLen_In) / m_iFrmSize_In ;
    
    if (iFrmNum * m_iFrmSize_Out > m_pMat_Out->col) {
      Z_SAFE_DEL(m_pMat_Out) ;
      m_pMat_Out = Z_SAFE_NEW(m_pMat_Out, ZMat, m_iCn, iFrmNum * m_iFrmSize_Out * 2) ;
    }
    
    int cur = 0 ;
    while (cur < iLenIn) {
      
      int ll = zmin(iLenIn - cur, m_iFrmSize_In - m_iLen_In) ;
      for (int i = 0 ; i < m_iCn ; i ++) {
        memcpy(m_pMat_In->data[i] + m_iLen_In, pWavIn[i] + cur , sizeof(float) * ll) ;
      }
      
      cur += ll ;
      m_iLen_In += ll ;
      
      if (m_iLen_In == m_iFrmSize_In) {
        
        for (int i = 0 ; i < m_iCn ; i ++) {
          r2ssp_rs_process(m_hEngine_Rs[i], m_pMat_In->data[i],
                           m_iFrmSize_In , m_pMat_Out->data[i] + m_iLen_Out , m_iFrmSize_Out);
        }
        m_iLen_Out += m_iFrmSize_Out ;
        
        m_iLen_In = 0 ;
      }      
    }
    
    pWavOut = m_pMat_Out->data ;
    iLenOut = m_iLen_Out ;
    
    return  0 ;
    
    
  }


};




