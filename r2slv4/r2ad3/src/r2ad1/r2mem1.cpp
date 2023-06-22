//
//  r2mem11.cpp
//  r2ad
//
//  Created by hadoop on 10/22/15.
//  Copyright (c) 2015 hadoop. All rights reserved.
//

#include "r2mem1.h"

#include <unistd.h>

r2mem1::r2mem1(r2sto1* pSto){
  
  m_pSto = pSto ;
  
  //In
  if (m_pSto->m_iSampleBytes == 3) {
    m_pMem_In = R2_SAFE_NEW(m_pMem_In,r2mem_i, m_pSto->m_iMicNum,r2_in_int_24,m_pSto->m_pMicInfo_In);
  }else if(m_pSto->m_iMicNum == 10){
    m_pMem_In = R2_SAFE_NEW(m_pMem_In, r2mem_i, m_pSto->m_iMicNum,r2_in_int_32_10,m_pSto->m_pMicInfo_In);
  }else{
    m_pMem_In = R2_SAFE_NEW(m_pMem_In, r2mem_i, m_pSto->m_iMicNum,r2_in_int_32,m_pSto->m_pMicInfo_In);
  }
  
  //rs
  m_pMem_Rs = R2_SAFE_NEW(m_pMem_Rs, r2mem_rs2, m_pSto->m_iMicNum,m_pSto->m_iSampleRate, m_pSto->m_pMicInfo_Rs, false);
  //m_pMem_Rs = R2_SAFE_NEW(m_pMem_Rs, r2mem_rs, m_pSto->m_iMicNum,m_pSto->m_iSampleRate, m_pSto->m_pMicInfo_Rs, false);
  
  //rdcsd
  m_pMem_RDc = R2_SAFE_NEW(m_pMem_RDc, r2mem_rdc, m_pSto->m_pMicInfo_Aec, m_pSto->m_pMicInfo_AecRef, 16000) ;
  
  //aec
  m_pMem_Aec = R2_SAFE_NEW(m_pMem_Aec, r2mem_aec, m_pSto->m_iMicNum, m_pSto->m_pMicInfo_Aec, m_pSto->m_pMicInfo_AecRef, m_pSto->m_pCpuInfo_Aec);
  
  //Out
  m_pMem_Out = R2_SAFE_NEW(m_pMem_Out, r2mem_o, m_pSto->m_iMicNum, r2_out_float_32, m_pSto->m_pMicInfo_Aec);
  
  //Buff
  m_pMem_Buff = R2_SAFE_NEW(m_pMem_Buff, r2mem_buff);
  
#ifdef __ARM_ARCH_ARM__
  m_bFirstData = true ;
#endif
  
  m_bFirstFrm = true ;
  
  //debug
  m_strDebugFolder =  DEBUG_FILE_LOCATION;
  r2_mkdir(m_strDebugFolder.c_str());
  
#ifdef _RAW_AUDIO
  char rawpath[256];
  sprintf(rawpath, "%s/raw.pcm",m_strDebugFolder.c_str());
  m_pRawFile = fopen(rawpath, "wb");
  m_iRecordLen_Max = 100 * 1024 * 1024 ;
  m_iRecordLen_Cur = 0 ;
#endif
  
#ifdef  _RS_AUDIO
  char rspath[256];
  sprintf(rspath,"%s/rs/",m_strDebugFolder.c_str());
  r2_mkdir(rspath);
  m_pRsFile = R2_SAFE_NEW_AR1(m_pRsFile,FILE*,m_pSto->m_iMicNum);
  for (int i = 0 ; i < m_pSto->m_iMicNum ; i ++) {
    sprintf(rspath,"%s/rs/%d.pcm",m_strDebugFolder.c_str(),i);
    m_pRsFile[i] = fopen(rspath, "wb");
  }
#endif
  
#ifdef  _AEC_AUDIO
  char aecpath[256];
  sprintf(aecpath,"%s/aec/",m_strDebugFolder.c_str());
  r2_mkdir(aecpath);
  m_pAecFile = R2_SAFE_NEW_AR1(m_pAecFile,FILE*,m_pSto->m_iMicNum);
  for (int i = 0 ; i < m_pSto->m_iMicNum ; i ++) {
    sprintf(aecpath,"%s/aec/%d.pcm",m_strDebugFolder.c_str(),i);
    m_pAecFile[i] = fopen(aecpath, "wb");
  }
#endif
  
}

r2mem1::~r2mem1(void)
{
  R2_SAFE_DEL(m_pMem_In);
  
  R2_SAFE_DEL(m_pMem_Rs);
  
  R2_SAFE_DEL(m_pMem_RDc);
  
  R2_SAFE_DEL(m_pMem_Aec);
  
  R2_SAFE_DEL(m_pMem_Out);
  
  R2_SAFE_DEL(m_pMem_Buff);
  
#ifdef  _AEC_AUDIO
  for (int i = 0 ; i < m_pSto->m_iMicNum ; i ++) {
    fclose(m_pAecFile[i]);
  }
  R2_SAFE_DEL_AR1(m_pAecFile);
#endif
  
#ifdef  _RS_AUDIO
  for (int i = 0 ; i < m_pSto->m_iMicNum ; i ++) {
    fclose(m_pRsFile[i]);
  }
  R2_SAFE_DEL_AR1(m_pRsFile);
#endif
  
#ifdef _RAW_AUDIO
  fclose(m_pRawFile);
#endif
  
}

int r2mem1::ProcessData(char* pData_In, int iLen_In, char*& pData_Out, int &iLen_Out){
  
  assert(iLen_In == 0 || (iLen_In > 0 && pData_In != NULL)) ;
  
  
#ifdef __ARM_ARCH_ARM__
  if (m_bFirstData) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    int cpuid = m_pSto->m_pCpuInfo_Mat->pMicIdLst[1] ;
    CPU_SET(cpuid, &mask);
    setCurrentThreadAffinityMask(mask);
    m_bFirstData = false ;
  }
#endif
  
  
  pData_Out = NULL ;
  iLen_Out = 0 ;
  
  //prepare data
  float** pData_Mul = NULL;
  int iLen_Mul = 0;
  
#ifdef _RAW_AUDIO
  if (m_iRecordLen_Cur < m_iRecordLen_Max) {
    fwrite(pData_In, sizeof(char), iLen_In, m_pRawFile);
    m_iRecordLen_Cur += iLen_In ;
  }
#endif
  
  //in
  m_pMem_In->process(pData_In, iLen_In, pData_Mul, iLen_Mul);
  
  //rs
  m_pMem_Rs->process(pData_Mul,iLen_Mul,pData_Mul,iLen_Mul);

  //fix aec
  //m_pMem_Aec_fix->process(pData_Mul,iLen_Mul);
  
  //remove dc
  m_pMem_RDc->process(pData_Mul, iLen_Mul);

#ifdef  _RS_AUDIO
  for (int i = 0 ; i < m_pSto->m_iMicNum ; i ++) {
    fwrite(pData_Mul[i], sizeof(float), iLen_Mul, m_pRsFile[i]);
  }
#endif
  
  //aec
  int rt = 0 ;
  if (m_pSto->m_bAec) {
    rt = m_pMem_Aec->process(pData_Mul,iLen_Mul,pData_Mul,iLen_Mul);
#ifdef  _AEC_AUDIO
    for (int i = 0 ; i < m_pSto->m_iMicNum ; i ++) {
      fwrite(pData_Mul[i], sizeof(float), iLen_Mul, m_pAecFile[i]);
    }
#endif
  }
  
  //fix mic
  if (iLen_Mul > 0 && m_bFirstFrm) {
    m_bFirstFrm = false ;
    for (int i = 0 ; i < m_pSto->m_pMicInfo_Aec->iMicNum ; i ++) {
      int iMicId = m_pSto->m_pMicInfo_Aec->pMicIdLst[i] ;
      if (m_pMem_RDc->m_bMicOk[i] == 1) {
        pData_Mul[iMicId][0] = 1.0f ;
      }else{
        pData_Mul[iMicId][0] = 0.0f ;
      }
    }
    
  }
  
  //out
  m_pMem_Out->process(pData_Mul,iLen_Mul, pData_Out, iLen_Out) ;
  
  
  usleep(10);
  
  return rt ;
}

int r2mem1::ProcessData(char* pData_In, int iLen_In){
  
  char * pData_Out = NULL ;
  int iLen_Out = 0 ;
  int rt = ProcessData(pData_In, iLen_In, pData_Out, iLen_Out);
  
  m_pMem_Buff->put(pData_Out, iLen_Out);
  
  return rt ;
}

int r2mem1::reset(){
  
  //in
  m_pMem_In->reset() ;
  
  //rs
  m_pMem_Rs->reset() ;
  
  //aec
  m_pMem_Aec->reset() ;
  
  //out
  m_pMem_Out->reset() ;
  
  //buff
  m_pMem_Buff->reset() ;
  
  return 0 ;
}