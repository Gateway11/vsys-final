//
//  zbuff.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zbuff.h"


namespace __r2vt4__ {

  //ZVecBuff--------------------------------------------------------------
  ZVecBuff::ZVecBuff(int size){
    
    m_iCurPos = 0 ;
    m_pVec = Z_SAFE_NEW(m_pVec,ZVec,size);
    
  }
  
  
  ZVecBuff::~ZVecBuff(void){
    
    Z_SAFE_DEL(m_pVec);
  }
  
  
  int ZVecBuff::PutBuff(const float* pData , int iDataNum){
    
    int iCur = 0 ;
    
    while (iCur < iDataNum) {
      int ll = zmin(iDataNum - iCur , m_pVec->size - m_iCurPos);
      memcpy(m_pVec->data + m_iCurPos , pData + iCur, sizeof(float) * ll);
      iCur += ll ;
      m_iCurPos += ll ;
      if (m_iCurPos == m_pVec->size) {
        m_iCurPos = 0 ;
      }
    }
    
    return  0 ;
  }
  
  int ZVecBuff::GetBuff(float* pData, int start, int end){
    
    assert(start < m_pVec->size);
    assert(end < m_pVec->size);
    assert(start > end);
    
    int iPos1 = m_iCurPos - start ;
    if (iPos1 < 0) {
      iPos1 += m_pVec->size ;
    }
    int iDataNum = start - end ;
    
    int iCur = 0 ;
    while (iCur < iDataNum) {
      int ll = zmin(iDataNum - iCur , m_pVec->size - iPos1);
      memcpy(pData + iCur, m_pVec->data + iPos1 ,  sizeof(float) * ll);
      iCur += ll ;
      iPos1 += ll ;
      if (iPos1 == m_pVec->size) {
        iPos1 = 0 ;
      }
    }
    
    return  0 ;
  }
  
  int ZVecBuff::Reset(){
    
    m_iCurPos = 0 ;
    m_pVec->Clean() ;
    
    return  0 ;
  }
  
  
  int ZVecBuff::StoreFile(const char* pFilePath, int start, int end){
    
    FILE* pFile = fopen(pFilePath, "wb");
    for (int i = 0 ; i <  start - end ; i ++) {
      int pos = m_iCurPos - start + i ;
      if (pos < 0) {
        pos += m_pVec->size ;
      }
      fwrite(m_pVec->data + pos, sizeof(float), 1, pFile);
    }
    fclose(pFile);
    
    return  0 ;
    
  }

  //ZAudBuff_F--------------------------------------------------------------
  ZAudBuff_S::ZAudBuff_S(int size){
    
    m_bWorking = false  ;
    m_iCurSize = 0 ;
    m_iTotalSize = size ;
    m_pData = Z_SAFE_NEW_AR1(m_pData, float, m_iTotalSize);
    
  }
  
  ZAudBuff_S::~ZAudBuff_S(void){
    
    Z_SAFE_DEL_AR1(m_pData) ;
  }

  int ZAudBuff_S::PutBuff(const float* pData , int iDataNum){
    
    if (m_iCurSize + iDataNum > m_iTotalSize) {
      m_iTotalSize = (m_iCurSize + iDataNum) * 2 ;
      float* pTmp = Z_SAFE_NEW_AR1(pTmp, float, m_iTotalSize);
      memcpy(pTmp, m_pData, sizeof(float) * m_iCurSize) ;
      Z_SAFE_DEL_AR1(m_pData) ;
      m_pData = pTmp ;
    }
    
    memcpy(m_pData + m_iCurSize , pData, sizeof(float) * iDataNum);
    m_iCurSize += iDataNum ;
    
    return 0 ;
  }
  
  int ZAudBuff_S::GetBuff(float* pData, int start, int end){
    
    if (start <= m_iCurSize) {
      memcpy(pData, m_pData + m_iCurSize - start , sizeof(float) * (start - end)) ;
    }else{
      memset(m_pData, 0, sizeof(float) * (start - m_iCurSize));
      memcpy(pData + start - m_iCurSize , m_pData, sizeof(float) * (m_iCurSize - end)) ;
      ZLOG_INFO("xfffffffffffxfffffffffffxfffffffffffxfffffffffffxfffffffffffxfffffffffff");
    }
    
    return 0 ;
    
  }
  
  int ZAudBuff_S::Reset(){
    
    m_bWorking = false  ;
    m_iCurSize = 0 ;
    
    return 0 ;
  }
  
  
  
  //ZMatBuff--------------------------------------------------------------
  ZMatBuff::ZMatBuff(int row , int col){
    
    m_iCurPos = 0  ;
    m_pMat = Z_SAFE_NEW(m_pMat,ZMat,row,col);
  }
  
  ZMatBuff::~ZMatBuff(void){
    
    Z_SAFE_DEL(m_pMat);
  }
  
  int ZMatBuff::PutBuff(const float** pDataBuff, int iDataNum){
    
    for (int i = 0 ; i < iDataNum ; i ++) {
      memcpy(m_pMat->data[m_iCurPos], pDataBuff[i], sizeof(float) * m_pMat->col);
      m_iCurPos ++ ;
      if (m_iCurPos == m_pMat->row) {
        m_iCurPos = 0 ;
      }
    }
    return  0 ;
  }
  
  float*  ZMatBuff::GetBuff(int offset){
    
    
    assert(m_iCurPos >= 0 );
    assert(offset <= m_pMat->row) ;
    
    int pos = m_iCurPos - offset ;
    if (pos < 0) {
      pos += m_pMat->row ;
    }
    return m_pMat->data[pos];
  }
  
  int ZMatBuff::StoreFile(const char* pFilePath, int start, int end){
    
    FILE* pFile = fopen(pFilePath, "wb");
    int row = start - end ;
    int col = m_pMat->col ;
    fwrite(&row, sizeof(int), 1, pFile);
    fwrite(&col, sizeof(int), 1, pFile);
    for (int i = 0 ; i <  start - end ; i ++) {
      int pos = m_iCurPos - start + i ;
      if (pos < 0) {
        pos += m_pMat->row ;
      }
      fwrite(m_pMat->data[pos], sizeof(float), m_pMat->col, pFile);
    }
    fclose(pFile);
    return  0 ;
  }
  
  int ZMatBuff::Reset(){
    
    m_iCurPos = 0 ;
    m_pMat->Clean() ;
    return  0 ;
  }

  
  //ZAudBuff--------------------------------------------------------------
  ZAudBuff::ZAudBuff(int iMicNum , int iMaxLen){
    
    m_iMicNum = iMicNum ;
    m_iMaxLen = iMaxLen ;
    m_iCurPos = 0 ;
    
    m_pAudio = Z_SAFE_NEW(m_pAudio, ZMat, m_iMicNum ,  m_iMaxLen) ;
  }
  
  ZAudBuff::~ZAudBuff(void){
    
    Z_SAFE_DEL(m_pAudio) ;
  }
  
  int ZAudBuff::PutAudio(const float** pAudBuff, int iLen){
    
    if (iLen >= m_iMaxLen) {
      m_iCurPos = 0 ;
      for (int i = 0 ; i < m_iMicNum ; i ++) {
        memcpy(m_pAudio->data[i], pAudBuff[i] + iLen - m_iMicNum, sizeof(float) * m_iMaxLen);
      }
    }else{
      int len1 = zmin(iLen, m_iMaxLen - m_iCurPos) ;
      for (int i = 0 ; i < m_iMicNum ; i ++) {
        memcpy(m_pAudio->data[i] + m_iCurPos , pAudBuff[i], sizeof(float) * len1);
      }
      m_iCurPos += len1 ;
      if (m_iCurPos == m_iMaxLen) {
        m_iCurPos = 0 ;
      }
      if (iLen > len1) {
        int len2 = iLen - len1 ;
        for (int i = 0 ; i < m_iMicNum ; i ++) {
          memcpy(m_pAudio->data[i] + m_iCurPos , pAudBuff[i] + len1, sizeof(float) * len2);
        }
        m_iCurPos += len2 ;
      }
    }
    return  0 ;
  }
  
  int ZAudBuff::GetLastAudio(float** pAudBuff, int iStart, int iEnd){
    
    assert(iStart < m_iMaxLen);
    assert(iEnd < m_iMaxLen);
    
    if (iStart < m_iCurPos) {
      int iLen = iStart  - iEnd ;
      for (int i = 0 ; i < m_iMicNum  ; i ++) {
        memcpy(pAudBuff[i], m_pAudio->data[i] + m_iCurPos - iStart, sizeof(float) * iLen);
      }
    }else{
      if (iEnd > m_iCurPos) {
        int iLen = iStart  - iEnd ;
        for (int i = 0 ; i < m_iMicNum  ; i ++) {
          memcpy(pAudBuff[i], m_pAudio->data[i] + m_iMaxLen + m_iCurPos - iStart, sizeof(float) * iLen);
        }
      }else{
        int iLen1 = iStart - m_iCurPos ;
        int iLen2 = m_iCurPos - iEnd ;
        for (int i = 0 ; i < m_iMicNum ; i ++) {
          memcpy(pAudBuff[i], m_pAudio->data[i] + m_iMaxLen - iLen1, sizeof(float) * iLen1);
          memcpy(pAudBuff[i] + iLen1, m_pAudio->data[i], sizeof(float) * iLen2) ;
        }
      }
    }
    return  0 ;
  }
  
  int ZAudBuff::GetLastAudio(float* pAudBuff, int iStart, int iEnd, int iCn){
    
    assert(iStart < m_iMaxLen);
    assert(iEnd < m_iMaxLen);
    
    if (iStart < m_iCurPos) {
      int iLen = iStart  - iEnd ;
      memcpy(pAudBuff, m_pAudio->data[iCn] + m_iCurPos - iStart, sizeof(float) * iLen);
    }else{
      if (iEnd > m_iCurPos) {
        int iLen = iStart  - iEnd ;
        memcpy(pAudBuff, m_pAudio->data[iCn] + m_iMaxLen + m_iCurPos - iStart, sizeof(float) * iLen);
      }else{
        int iLen1 = iStart - m_iCurPos ;
        int iLen2 = m_iCurPos - iEnd ;
        memcpy(pAudBuff, m_pAudio->data[iCn] + m_iMaxLen - iLen1, sizeof(float) * iLen1);
        memcpy(pAudBuff + iLen1, m_pAudio->data[iCn], sizeof(float) * iLen2) ;
      }
    }
    return  0 ;
    
  }
  
  int ZAudBuff::Reset(){
    
    m_pAudio->Clean() ;
    m_iCurPos = 0 ;
    return 0 ;
  }
  
};




