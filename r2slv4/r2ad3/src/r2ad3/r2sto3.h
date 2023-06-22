//
//  r2sto3.h
//  r2ad2
//
//  Created by hadoop on 8/4/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

#ifndef __r2ad2__r2sto3__
#define __r2ad2__r2sto3__

#include "../r2math.h"
class r2sto3
{
public:
  r2sto3(const char* pWorkDir);
public:
  ~r2sto3(void);
  
  //resample
  int m_iMicNum ;
  float* m_pMicPos ;
  float* m_pMicI2sDelay ;
  
  //Mic Info
  bool m_bBf ;
  bool m_bAgc ;
  
  int m_iNoBfMicId ;
  
  
  r2_mic_info* m_pMicInfo_In ;
  r2_mic_info* m_pMicInfo_Bf ;
  r2_mic_info* m_pMicInfo_Out ;
  
};


#endif /* __r2ad2__r2sto3__ */
