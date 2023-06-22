//
//  r2mem3.cpp
//  r2ad2
//
//  Created by hadoop on 8/4/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

#include "r2mem3.h"

r2mem3::r2mem3(r2sto3* pSto){
  
  m_pSto = pSto ;
  
  //in
  m_pMem_In = R2_SAFE_NEW(m_pMem_In, r2mem_i, m_pSto->m_iMicNum, r2_in_float_32, m_pSto->m_pMicInfo_In);
  
  //bf
  m_pMem_Bf = R2_SAFE_NEW(m_pMem_Bf, r2mem_bf, m_pSto->m_iMicNum, m_pSto->m_pMicPos, m_pSto->m_pMicI2sDelay, m_pSto->m_pMicInfo_Bf);
  m_pMem_Bf->steer(0.0f, 0.0f);
  
  //agc
  m_pMem_Agc = R2_SAFE_NEW(m_pMem_Agc, r2mem_agc);
  
  //out
  m_pMem_Out = R2_SAFE_NEW(m_pMem_Out, r2mem_o, 1, r2_out_short_16, m_pSto->m_pMicInfo_Out);
  
  //Buff
  m_pMem_Buff = R2_SAFE_NEW(m_pMem_Buff, r2mem_buff);
  
  //fix mic
  m_bFirstFrm = true ;
}

r2mem3::~r2mem3(void){
  
  R2_SAFE_DEL(m_pMem_In);
  R2_SAFE_DEL(m_pMem_Bf);
  R2_SAFE_DEL(m_pMem_Agc);
  R2_SAFE_DEL(m_pMem_Out);
  
  R2_SAFE_DEL(m_pMem_Buff);
}

int r2mem3::ProcessData(char* pData_In, int iLen_In, char*& pData_Out, int &iLen_Out){
  
  float** pData_Mul = NULL, *pData_Sig = NULL;
  int iLen_Mul = 0 , iLen_Sig = 0 ;
  
  //In
  m_pMem_In->process(pData_In, iLen_In, pData_Mul, iLen_Mul);
  
  //fix mic
  if (m_bFirstFrm && iLen_Mul > 0) {
    m_bFirstFrm = false ;
    r2_mic_info* pMicErr = geterrorinfo(pData_Mul, iLen_Mul) ;
    if (r2_fixerrmix(m_pSto->m_pMicInfo_Bf, pMicErr)) {
      R2_SAFE_DEL(m_pMem_Bf);
      m_pMem_Bf = R2_SAFE_NEW(m_pMem_Bf, r2mem_bf, m_pSto->m_iMicNum, m_pSto->m_pMicPos, m_pSto->m_pMicI2sDelay, m_pSto->m_pMicInfo_Bf);
      m_pMem_Bf->steer(0.0f, 0.0f);

    }
    r2_free_micinfo(pMicErr);
  }
  
  //Bf
  if (m_pSto->m_bBf) {
    m_pMem_Bf->process(pData_Mul, iLen_Mul, pData_Sig, iLen_Sig);
  }else{
    pData_Sig = pData_Mul[m_pSto->m_iNoBfMicId];
    iLen_Sig = iLen_Mul ;
  }
  
  
  //agc
  if (m_pSto->m_bAgc) {
    m_pMem_Agc->process(pData_Sig, iLen_Sig, pData_Sig, iLen_Sig);
  }
  
  //out
  m_pMem_Out->process(&pData_Sig, iLen_Sig, pData_Out, iLen_Out);
  
  return  0 ;
  
}

int r2mem3::ProcessData(char* pData_In, int iLen_In){
  
  char * pData_Out = NULL ;
  int iLen_Out = 0 ;
  
  ProcessData(pData_In, iLen_In, pData_Out, iLen_Out);
  
  m_pMem_Buff->put(pData_Out, iLen_Out);
  
  return 0 ;
  
}

int r2mem3::reset(){
  
  m_pMem_Bf->reset() ;
  m_pMem_Bf->steer(0.0f, 0.0f);
  
  m_pMem_Agc->reset() ;
  m_pMem_Buff->reset() ;
  
  return 0 ;
}

r2_mic_info* r2mem3::geterrorinfo(float** pData_Mul, int iLen_Mul){
  
  
  std::vector<int> ErrorMic ;
  for (int i = 0 ; i < m_pSto->m_pMicInfo_In->iMicNum ; i ++) {
    int iMicId = m_pSto->m_pMicInfo_In->pMicIdLst[i] ;
    if (fabs(pData_Mul[iMicId][0]) < 0.5f) {
      ErrorMic.push_back(iMicId);
    }
  }
  
  if (ErrorMic.size() > 0) {
    r2_mic_info* pMicErr = r2_getdefaultmicinfo(ErrorMic.size());
    for (int i = 0 ; i < ErrorMic.size() ; i ++) {
      pMicErr->pMicIdLst[i] = ErrorMic[i] ;
      ZLOG_INFO("Detect Err Mic %d", ErrorMic[i]);
    }
    return  pMicErr ;
  }
  
  return NULL ;
  
}

