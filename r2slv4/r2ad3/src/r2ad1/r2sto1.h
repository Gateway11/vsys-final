//
//  r2sto1.h
//  r2ad
//
//  Created by hadoop on 10/22/15.
//  Copyright (c) 2015 hadoop. All rights reserved.
//

#ifndef __r2ad__r2sto1__
#define __r2ad__r2sto1__

#include "../r2math.h"

class r2sto1
{
public:
  r2sto1(const char* pWorkDir);
public:
  ~r2sto1(void);
  
  //resample
  int m_iMicNum ;
  int m_iSampleRate ;
  int m_iSampleBytes ;
  
  //Mic Info
  r2_mic_info* m_pMicInfo_In ;
  r2_mic_info* m_pMicInfo_Rs ;
  r2_mic_info* m_pMicInfo_Aec ;
  r2_mic_info* m_pMicInfo_AecRef ;
  
  
  bool    m_bAec ;
  float   m_fAecShield ;
  r2_mic_info*  m_pCpuInfo_Aec ;
  r2_mic_info*  m_pCpuInfo_Mat ;
};

#endif /* defined(__r2ad__r2sto1__) */
