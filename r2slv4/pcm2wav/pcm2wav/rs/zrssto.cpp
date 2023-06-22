//
//  zrssto.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zrssto.h"

#include "fa_fir.h"
#include "fa_resample.h"

namespace __r2vt4__ {
  
  ZRsMem::ZRsMem(int iCn, int iSrIn, int iSrOut, int iFrmOut){
    
    m_iCn = iCn ;
    m_iSrIn = iSrIn ;
    m_iSrOut = iSrOut ;
    m_iFrmOut = iFrmOut ;
    
    int sr_gcd = z_gcd(iSrIn, iSrOut) ;
    
    iSrIn = iSrIn / sr_gcd ;
    iSrOut = iSrOut / sr_gcd ;
    while (iSrOut % 4 != 0) {
      iSrIn = iSrIn * 2 ;
      iSrOut = iSrOut * 2 ;
    }
    
    //Filter
    m_pResample = ZNS_LIBRESAMPLE::fa_resample_filter_init(iSrOut, iSrIn , 1.0f, ZNS_LIBRESAMPLE::HAMMING);
    
    ZNS_LIBRESAMPLE::fa_resample_filter_t *resflt = (ZNS_LIBRESAMPLE::fa_resample_filter_t *)m_pResample;
    
    m_iL = resflt->L;
    m_iM = resflt->M;
    m_iQ = resflt->tvflt.k;
    
    assert(m_iL % 4 == 0 ) ;
    
    //Filter
    m_pFileter = Z_SAFE_NEW(m_pFileter, ZMat, m_iL, m_iM + m_iQ - 1 ) ;
    for (int i = 0 ; i < m_iL ; i ++) {
      int iStartPos = (i * m_iM)/m_iL ;
      for (int j = 0 ; j < m_iQ ; j ++) {
        m_pFileter->data[i][j + iStartPos] = resflt->tvflt.g[i][m_iQ - 1 - j] ;
      }
    }
    
    m_pFileter_Neon = Z_SAFE_NEW(m_pFileter_Neon, ZMat_Neon, m_iL, m_iM + m_iQ - 1, false) ;
    m_pFileter_Neon->CopyFrom(m_pFileter);
    
    
    
    //BatchSize
    int sss = z_lcm(m_iL, m_iFrmOut) ;
    m_iBatchSize = sss / m_iL ;
    
    while ( (m_iBatchSize * m_iCn) % 4 != 0) {
      m_iBatchSize = m_iBatchSize * 2 ;
    }
    
    
    m_iLen_In = m_iQ - 1 ;
    m_iLen_In_Total = m_iQ - 1 + m_iM * m_iBatchSize  ;
    m_pAud_In = Z_SAFE_NEW(m_pAud_In, ZMat, m_iCn, m_iLen_In_Total) ;
    
    
    //
    m_pBatchAud_In = Z_SAFE_NEW(m_pBatchAud_In, ZVec, m_iQ + m_iM - 1);
    
    m_pBatchAud_Out = Z_SAFE_NEW(m_pBatchAud_Out, ZVec, m_iL) ;
    
    
    m_iLen_Out_Total = 1024 ;
    m_pAud_Out = Z_SAFE_NEW(m_pAud_Out, ZMat, m_iCn, m_iLen_Out_Total) ;
    
  }
  
  ZRsMem::~ZRsMem(void){
    
    Z_SAFE_DEL(m_pFileter);
    
    Z_SAFE_DEL(m_pFileter_Neon) ;
    
    Z_SAFE_DEL(m_pAud_In) ;
    
    Z_SAFE_DEL(m_pBatchAud_In) ;
    
    Z_SAFE_DEL(m_pBatchAud_Out) ;
    
    Z_SAFE_DEL(m_pAud_Out) ;
    
    
  }
  
  int ZRsMem::Reset(){
    
    m_iLen_In = m_iQ ;
    m_pAud_In->Clean() ;
    
    return 0 ;
  }
  
  int ZRsMem::Process(const float** pWavIn, const int iLenIn, float** &pWavOut, int &iLenOut){
    
    
    m_iLen_Out = 0 ;
    m_pAud_Out->Clean() ;
    
    int iCur = 0 ;
    while (iCur < iLenIn) {
      int ll = zmin(m_iLen_In_Total - m_iLen_In, iLenIn - iCur);
      for (int i = 0 ; i < m_iCn ; i ++) {
        memcpy(m_pAud_In->data[i] + m_iLen_In, pWavIn[i] + iCur, sizeof(float) * ll) ;
      }
      iCur += ll ;
      m_iLen_In += ll ;
      
      if (m_iLen_In == m_iLen_In_Total) {
        //process
        ProcessBatch();
        
      for (int i = 0 ; i < m_iCn ; i ++) {
        memcpy(m_pAud_In->data[i], m_pAud_In->data[i] + m_iM * m_iBatchSize , sizeof(float) * (m_iQ - 1)) ;
      }
      m_iLen_In = m_iQ - 1  ;
      }

      
    }
    
    pWavOut = m_pAud_Out->data ;
    iLenOut = m_iLen_Out ;
    
    return  0 ;
  }
  
  
  int ZRsMem::ProcessBatch(){
    
    if (m_iLen_Out_Total < m_iLen_Out + m_iBatchSize * m_iL) {
      m_iLen_Out_Total = (m_iLen_Out + m_iBatchSize * m_iL) * 2 ;
      ZMat* pTmp = Z_SAFE_NEW(pTmp, ZMat, m_iCn, m_iLen_Out_Total);
      for (int i = 0 ; i < m_iCn ; i ++) {
        memcpy(pTmp->data[i], m_pAud_Out->data[i], sizeof(float) * m_iLen_Out) ;
      }
      Z_SAFE_DEL(m_pAud_Out);
      m_pAud_Out = pTmp ;
    }
    
    float* pTmp1 = m_pBatchAud_In->data ;
    float* pTmp2 = m_pBatchAud_Out->data ;
    
    for (int i = 0 ; i < m_iCn  ; i ++ ) {
      for (int j = 0 ; j < m_iBatchSize ; j ++) {
        
        memcpy(m_pBatchAud_In->data, m_pAud_In->data[i] + j * m_iM , sizeof(float) * (m_iQ + m_iM - 1)) ;
        
        m_pBatchAud_Out->Clean() ;
        
        m_pBatchAud_Out->Add_aAX_NENO(m_pFileter_Neon, m_pBatchAud_In) ;
        
        memcpy(m_pAud_Out->data[i] + m_iLen_Out + j * m_iL , m_pBatchAud_Out->data , sizeof(float) * m_iL) ;
        
//        m_pBatchAud_In->data = m_pAud_In->data[i] + j * m_iM ;
//        m_pBatchAud_Out->data = m_pAud_Out->data[i] + m_iLen_Out + j * m_iL ;
        
//        m_pBatchAud_Out->Add_aAX_NENO(m_pFileter_Neon, m_pBatchAud_In) ;
        
      }
    }
    
    m_pBatchAud_In->data = pTmp1 ;
    m_pBatchAud_Out->data = pTmp2 ;
    
    m_iLen_Out += m_iL * m_iBatchSize ;
    
    return  0 ;
  
  }

};




