//
//  zoutput.cpp
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zoutput.h"

namespace __r2vt4__ {

  ZOutput::ZOutput(ZOutputType iOutputType){
    
    m_iOutputType = iOutputType ;
  }
  
  ZOutput::~ZOutput(void){
    
    
  }
  
  int ZOutput::PutChar(char c){
    
    WriteRawData(&c, 1);
    return 0 ;
  }
  
  void ZOutput::WriteToken(const char* pToken){
    
    WriteRawData(pToken, strlen(pToken));
    PutChar(' ');
  }
  
  //-------------------------------------------------------------------
  
  ZBuffOutput::ZBuffOutput(char* &pDataBuff, int &iDataLen, ZOutputType iOutputType)
  :ZOutput(iOutputType),m_pDataBuff(pDataBuff),m_iWritePos(iDataLen){
    
    m_iTotalLen = 1024 ;
    m_pDataBuff = Z_SAFE_NEW_AR1(m_pDataBuff, char, m_iTotalLen);
  }
  
  ZBuffOutput::~ZBuffOutput(void){
    
    
  }
  
  int ZBuffOutput::WriteRawData(const char* pDataBuff, size_t iDataLen){
    
    if (m_iWritePos + iDataLen > m_iTotalLen) {
      m_iTotalLen += m_iWritePos + iDataLen ;
      char* pTmp = Z_SAFE_NEW_AR1(pTmp,char, m_iTotalLen);
      memcpy(pTmp, m_pDataBuff, sizeof(char) * m_iWritePos);
      Z_SAFE_DEL_AR1(m_pDataBuff);
      m_pDataBuff = pTmp ;
    }
    
    memcpy(m_pDataBuff + m_iWritePos , pDataBuff, sizeof(char) * iDataLen);
    m_iWritePos += iDataLen ;
    
    return  0 ;
  }
  
  //-------------------------------------------------------------------
  ZFileOutput::ZFileOutput(const char* pFilePath, ZOutputType iOutputType)
  :ZOutput(iOutputType){
    
    m_pFile = fopen(pFilePath, "wb");
    if (m_pFile != NULL && m_iOutputType == ZOUT_KALDI_BINERY) {
      PutChar('\0');
      PutChar('B');
    }
    
  }
  
  
  ZFileOutput::~ZFileOutput(void){
    
    if (m_pFile != NULL) {
      fclose(m_pFile);
      m_pFile = NULL ;
    }
  }
  
  
  bool ZFileOutput::isValid(){
    
    if (m_pFile != NULL) {
      return  true ;
    }else {
      return  false ;
    }
  }
  
  
  int ZFileOutput::WriteRawData(const char* pDataBuff, size_t iDataLen){
    
    assert(m_pFile != NULL);
    
    fwrite(pDataBuff, sizeof(char), iDataLen, m_pFile);
    
    return  0 ;
  }
  
};




