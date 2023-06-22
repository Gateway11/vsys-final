//
//  zinput.cpp
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zinput.h"

namespace __r2vt4__ {

  ZInput::ZInput(ZInputType iInputType){
    
    m_iInputType = iInputType ;
  }
  
  ZInput::~ZInput(void){
    
    
  }
  
  std::string ZInput::ReadToken(){
    
    std::string res ;
    char c = GetChar();
    while (c != ' ' && res.length() < 100) {
      res += c ;
      ReadRawData(&c, 1);
    }
    return  res ;
  }
  
  bool ZInput::ExpectToken(const char* pExpectToken){
    
    std::string token = ReadToken();
    
    if (strcasecmp(pExpectToken,token.c_str()) == 0) {
      return  true ;
    }else{
      assert(0);
      return  false ;
    }
  }
  
  char ZInput::GetChar(){
    
    char c = 0 ;
    ReadRawData(&c, 1);
    return  c ;
  }
  
  
  //-------------------------------------------------------------------
  
  ZBuffInput::ZBuffInput(char* pDataBuff, int iDataLen, ZInputType iInputType)
  :ZInput(iInputType){
    
    m_iReadPos = 0 ;
    m_iDataLen = iDataLen ;
    m_pDataBuff = Z_SAFE_NEW_AR1(m_pDataBuff, char, m_iDataLen);
    memcpy(m_pDataBuff, pDataBuff, m_iDataLen);
    
  }
  
  ZBuffInput::~ZBuffInput(void){
    
    Z_SAFE_DEL_AR1(m_pDataBuff);
  }
  
  int ZBuffInput::ReadRawData(char* pDataBuff, size_t iDataLen){
    
    assert(m_iReadPos + iDataLen <= m_iDataLen) ;
    
    memcpy(pDataBuff, m_pDataBuff + m_iReadPos , iDataLen);
    m_iReadPos += iDataLen ;
    
    return 0 ;
  }
  
  //-------------------------------------------------------------------
  
  ZFileInput::ZFileInput(const char* pFilePath, ZInputType iInputType)
  :ZInput(iInputType){
    
    m_pFile = fopen(pFilePath, "rb");
    if(m_pFile != NULL && m_iInputType == ZIN_KALDI_BINERY){
      assert(getc(m_pFile) == '\0') ;
      assert(getc(m_pFile) == 'B') ;
    }
    
  }
  
  ZFileInput::~ZFileInput(void){
    
    if (m_pFile != NULL) {
      fclose(m_pFile);
      m_pFile = NULL ;
    }
  }
  
  bool ZFileInput::isValid(){
    
    if (m_pFile != NULL) {
      return  true ;
    }else {
      return  false ;
    }
  }
  
  int ZFileInput::ReadRawData(char* pDataBuff, size_t iDataLen){
    
    assert(m_pFile != NULL);
    fread(pDataBuff, sizeof(char), iDataLen, m_pFile);
    
    return  0 ;
  }

};




