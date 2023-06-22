//
//  r2sto2.h
//  r2ad
//
//  Created by hadoop on 10/22/15.
//  Copyright (c) 2015 hadoop. All rights reserved.
//

#ifndef __r2ad__r2sto2__
#define __r2ad__r2sto2__

#include "../r2math.h"
#include "../../../r2vt4/src/vt/zvtapi.h"
class r2sto2
{
public:
  r2sto2(const char* pWorkDir);
public:
  ~r2sto2(void);
  
protected:
  int InitPath(const char* pWorkDir);
  
public:
  std::string m_strVtNnetPath ;
  std::string m_strVtPhoneTablePath ;
  
  int m_iWordNum;
  WordInfo * m_pWordLst;
  
  //
  int m_iMicNum ;
  float* m_pMicPos ;
  float* m_pMicI2sDelay ;
  
  //vad
  int m_iMicId_Vad ;
  float m_fBaseRange ;
  float m_fMinDynaRange ;
  float m_fMaxDynaRange ;
  
  float m_fSilShield ;
  
  //Mic Info
  r2_mic_info* m_pMicInfo_In ;
  r2_mic_info* m_pMicInfo_Bf ;
  
  bool m_bCod ;
  
};

#endif /* defined(__r2ad__r2sto2__) */
