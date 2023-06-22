//
//  r2mem_audio.h
//  r2ad_offline
//
//  Created by hadoop on 7/29/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

#ifndef __r2ad_offline__r2mem_audio__
#define __r2ad_offline__r2mem_audio__

#include "r2math.h"

namespace __r2ad_offline__ {
  
  enum r2_in_type{
    r2_in_int_24 = 1 ,
    r2_in_int_32 ,
    r2_in_int_32_10 ,
    r2_in_float_32
  };
  
  struct r2_int24 {
    unsigned char m_Internal[3] ;
    
    int toint(){
      if ((m_Internal[2] & 0x80) != 0){
        return  ((m_Internal[0] & 0xff) | (m_Internal[1] & 0xff) << 8 | (m_Internal[2] & 0xff) << 16 | (-1 & 0xff) << 24);
      }else{
        return  ((m_Internal[0] & 0xff) | (m_Internal[1] & 0xff) << 8 | (m_Internal[2] & 0xff) << 16 | (0 & 0xff) << 24);
      }
    }
  };
  
  struct WAVEFORM {
    unsigned short wFormatTag;
    unsigned short nChannels;
    unsigned int nSamplesPerSec;
    unsigned int nAvgBytesPerSec;
    unsigned short nBlockAlign;
    unsigned short wBitsPerSample;
  };
  
  struct RIFF {
    char riffID[4];
    unsigned int riffSIZE;
    char riffFORMAT[4];
  };
  
  struct FMT {
    char fmtID[4];
    unsigned int fmtSIZE;
    WAVEFORM fmtFORMAT;
  };
  
  struct DATA {
    char dataID[4];
    unsigned int dataSIZE;
  };
  
  class r2mem_audio {
    
  public:
    r2mem_audio(int iCn, int iSr);
  public:
    ~r2mem_audio(void);
    
  public:
    int ReadFormMultiFiles(std::vector<std::string> FilePathLst, r2_in_type iType);
    int ReadFormSingleFile(const char* pFilePath, r2_in_type iType, int iHeadOffset);
    int AddData(float** pData, int iDataLen);
    int AddData(r2mem_audio* pData);
    int Reset();
    
    int RemoveDc(int iCn);
    
    int WriteToWavFile(const char* pFilePath);
    
  public:
    
    int  m_iCn ;
    int  m_iSr ;
    
    int  m_iDataLen ;
    int  m_iDataTotalLen ;
    float ** m_pData ;
    
  };
  
};


#endif /* __r2ad_offline__r2mem_audio__ */
