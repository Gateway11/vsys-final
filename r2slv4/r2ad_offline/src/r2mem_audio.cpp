//
//  r2mem_audio.cpp
//  r2ad_offline
//
//  Created by hadoop on 7/29/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

#include "r2mem_audio.h"

namespace __r2ad_offline__ {
  
  
  r2mem_audio::r2mem_audio(int iCn, int iSr){
    
    m_iCn = iCn ;
    m_iSr = iSr ;
    
    m_iDataLen = 0 ;
    m_iDataTotalLen = iSr ;
    m_pData = R2_SAFE_NEW_AR2(m_pData, float, m_iCn, m_iDataTotalLen) ;
    
  }
  
  r2mem_audio::~r2mem_audio(void){
    
    R2_SAFE_DEL_AR2(m_pData) ;
    
  }
  
  int r2mem_audio::ReadFormMultiFiles(std::vector<std::string> FilePathLst, r2_in_type iType){
    
    int iFileNum = FilePathLst.size() ;
    assert(m_iCn == iFileNum);
    int iBit = 0 ;
    switch (iType) {
      case r2_in_int_24:
        iBit = 3 ;
        break;
      case r2_in_int_32:
        iBit = 4 ;
        break;
      case r2_in_int_32_10:
        iBit = 4 ;
        break;
      case r2_in_float_32:
        iBit = 4 ;
        break;
      default:
        return  - 1;
    }
    
    FILE* pFile = fopen(FilePathLst[0].c_str(), "rb");
    fseek(pFile, 0, SEEK_END);
    int iDataLen = ftell(pFile) / iBit;
    fclose(pFile);
    if (iDataLen < m_iSr * 0.5) {
      return 1 ;
    }
    
    float** pData = R2_SAFE_NEW_AR2(pData, float, m_iCn, iDataLen);
    char* pRawData = R2_SAFE_NEW_AR1(pRawData, char, iDataLen * iBit) ;
    
    for (int i = 0 ; i < iFileNum ; i ++) {
      pFile = fopen(FilePathLst[i].c_str(), "rb");
      fseek(pFile, 0, SEEK_END);
      if (ftell(pFile) != iDataLen * iBit) {
        fclose(pFile);
        R2_SAFE_DEL_AR2(pData);
        R2_SAFE_DEL_AR1(pRawData);
        return  1 ;
      }
      fseek(pFile, 0, SEEK_SET);
      fread(pRawData, sizeof(char), iDataLen * iBit, pFile);
      fclose(pFile) ;
      
      if(iType == r2_in_int_24){
        r2_int24* pData_Int24 = (r2_int24*) pRawData ;
        for (int j = 0 ; j < iDataLen ; j ++) {
          pData[i][j] = pData_Int24[j].toint() / 4.0f ;
        }
      }else if(iType == r2_in_int_32){
        int* pData_Int32 = (int*) pRawData ;
        for (int j = 0 ; j < iDataLen ; j ++) {
          pData[i][j] = pData_Int32[j] / 1024.0f ;
        }
      }else if(iType == r2_in_int_32_10){
        int* pData_Int32 = (int*) pRawData ;
        for (int j = 0 ; j < iDataLen ; j ++) {
          pData[i][j] = pData_Int32[j] / 4.0f ;
        }
      }else {
        memcpy(pData[i], pRawData, iDataLen * sizeof(float));
      }
    }
    
    AddData(pData, iDataLen);
    
    R2_SAFE_DEL_AR2(pData);
    R2_SAFE_DEL_AR1(pRawData);
    
    return  0 ;
    
  }
  
  int r2mem_audio::ReadFormSingleFile(const char* pFilePath, r2_in_type iType, int iHeadOffset){
    
    int iBit = 0 ;
    switch (iType) {
      case r2_in_int_24:
        iBit = 3 ;
        break;
      case r2_in_int_32:
        iBit = 4 ;
        break;
      case r2_in_int_32_10:
        iBit = 4 ;
        break;
      case r2_in_float_32:
        iBit = 4 ;
        break;
      default:
        return  - 1;
    }
    
    FILE* pFile = fopen(pFilePath, "rb");
    if (pFile == NULL) {
      return  1 ;
    }
    fseek(pFile, 0, SEEK_END);
    int iDataLen = ftell(pFile) - iHeadOffset;
    if (iDataLen < iHeadOffset) {
      fclose(pFile);
      return 1 ;
    }
    fseek(pFile, iHeadOffset, SEEK_SET);
    char* pRawData = R2_SAFE_NEW_AR1(pRawData, char, iDataLen) ;
    fread(pRawData, sizeof(char), iDataLen, pFile) ;
    fclose(pFile);
    
    iDataLen = iDataLen / m_iCn / iBit;
    
    if (iDataLen < m_iSr * 0.5) {
      return 1 ;
    }
    
    float** pData = R2_SAFE_NEW_AR2(pData, float, m_iCn, iDataLen) ;
    if(iType == r2_in_int_24){
      r2_int24* pData_Int24 = (r2_int24*) pRawData ;
      for (int j = 0 ; j < iDataLen ; j ++) {
        for (int i = 0 ; i < m_iCn ; i ++) {
          pData[i][j] = pData_Int24[j*m_iCn+i].toint() / 4.0f ;
        }
      }
    }else if(iType == r2_in_int_32){
      int* pData_Int32 = (int*) pRawData ;
      for (int j = 0 ; j < iDataLen ; j ++) {
        for (int i = 0 ; i < m_iCn ; i ++) {
          pData[i][j] = pData_Int32[j*m_iCn+i] / 1024.0f ;
        }
      }
    }else if(iType == r2_in_int_32_10){
      int* pData_Int32 = (int*) pRawData ;
      for (int j = 0 ; j < iDataLen ; j ++) {
        for (int i = 0 ; i < m_iCn ; i ++) {
          pData[i][j] = pData_Int32[j*m_iCn+i] / 4.0f ;
          //printf("%d\n", pData_Int32[j*m_iCn+i]) ;
        }
      }
    }else if(iType == r2_in_float_32){
      float* pData_Float = (float*) pRawData ;
      for (int j = 0 ; j < iDataLen ; j ++) {
        for (int i = 0 ; i < m_iCn ; i ++) {
          pData[i][j] = pData_Float[j*m_iCn+i];
        }
      }

    }
    
    AddData(pData, iDataLen) ;
    
    R2_SAFE_DEL_AR2(pData) ;
    R2_SAFE_DEL_AR1(pRawData) ;
    
    return  0 ;
    
  }
  
  int r2mem_audio::AddData(float** pData, int iDataLen){
    
    
    if (m_iDataLen + iDataLen > m_iDataTotalLen) {
      float** pTmpData = m_pData ;
      m_iDataTotalLen = (m_iDataLen + iDataLen) * 2 ;
      m_pData = R2_SAFE_NEW_AR2(m_pData, float, m_iCn, m_iDataTotalLen) ;
      for (int i = 0 ; i < m_iCn ; i ++) {
        memcpy(m_pData[i], pTmpData[i], m_iDataLen * sizeof(float));
      }
      R2_SAFE_DEL_AR2(pTmpData) ;
    }
    
    for (int i = 0 ; i < m_iCn ; i ++) {
      memcpy(m_pData[i] + m_iDataLen, pData[i], iDataLen * sizeof(float));
    }
    
    m_iDataLen += iDataLen ;
    
    return  0 ;
  }
  
  int r2mem_audio::AddData(r2mem_audio* pData){
    
    assert(pData->m_iCn == m_iCn) ;
    AddData(pData->m_pData, pData->m_iDataLen);
    
    return  0 ;
    
  }
  
  int r2mem_audio::Reset(){
    
    m_iDataLen = 0 ;
    
    return 0 ;
  }
  
  int r2mem_audio::RemoveDc(int iCn){
    
    float total = 0.0f ;
    for (int i = 0 ; i < m_iDataLen ; i ++) {
      total += m_pData[iCn][i] ;
    }
    total = total / m_iDataLen ;
    for (int i = 0 ; i < m_iDataLen ; i ++) {
      m_pData[iCn][i] -= total ;
    }
    return  0 ;
  }
  
  int r2mem_audio::WriteToWavFile(const char* pFilePath){
    
    FILE * pFile = fopen(pFilePath,"wb");
    if(pFile == NULL) {
      ZLOG_INFO("Error in WriteToWavFile : %s", pFilePath);
      return -1 ;
    }
    
    RIFF riff ;
    FMT fmt ;
    DATA data ;
    
    int iDataSize = m_iCn * m_iDataLen * sizeof(short) ;
    short* pData = R2_SAFE_NEW_AR1(pData, short, m_iCn * m_iDataLen) ;
    
    for (int i = 0 ; i < m_iDataLen ; i ++) {
      for (int j = 0 ; j < m_iCn ; j ++) {
        pData[i * m_iCn + j ] = m_pData[j][i] ;
      }
    }
    
    strncpy(riff.riffID, "RIFF", 4);
    riff.riffSIZE = sizeof(RIFF) + sizeof(FMT) + sizeof(DATA) + iDataSize - 8;
    strncpy(riff.riffFORMAT, "WAVE", 4);
    fwrite(&riff,sizeof(RIFF),1,pFile);
    
    //FMT
    strncpy(fmt.fmtID, "fmt ", 4);
    fmt.fmtSIZE = sizeof(WAVEFORM);
    fmt.fmtFORMAT.wFormatTag = 0x0001;       //0x0001	Microsoft PCM ; 0x0003	IEEE Float
    fmt.fmtFORMAT.nChannels = m_iCn;
    fmt.fmtFORMAT.nSamplesPerSec = m_iSr;
    fmt.fmtFORMAT.nBlockAlign = m_iCn * sizeof(short);
    fmt.fmtFORMAT.nAvgBytesPerSec = fmt.fmtFORMAT.nBlockAlign * m_iSr;
    fmt.fmtFORMAT.wBitsPerSample = 16;
    fwrite(&fmt,sizeof(FMT),1,pFile);
    
    //DATA
    strncpy(data.dataID, "data", 4);
    data.dataSIZE = iDataSize ;
    fwrite(&data,sizeof(DATA),1,pFile);
    fwrite(pData,1,iDataSize,pFile);
    
    fclose(pFile);
    
    R2_SAFE_DEL_AR1(pData);
    
    return 0 ;
  }
  
  
};




