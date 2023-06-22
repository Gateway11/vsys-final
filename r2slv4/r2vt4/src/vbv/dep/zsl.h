//
//  zsl.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zsl__
#define __r2vt4__zsl__

#include "../../zmath.h"
#include "zns.h"

#define USE_LZHU_SL

#ifdef USE_LZHU_SL
#include "../../sl/zslapi.h"
#else
#include "zsourcelocationapi.h"
#endif



namespace __r2vt4__ {

  class ZSl
  {
  public:
    ZSl(int iMicNum, float* pMicPosLst, float* pMicDelay, z_mic_info* pMicInfo_Sl);
    
  public:
    ~ZSl(void);
    
  public:
    int PutData(float** pfDataBuff, int iDataLen);
    int GetSl(int iStartPos, int iEndPos, float pSlInfo[3]);
    int GetSl2(int iStartPos, int iEndPos, float* pSlInfo);
    int Reset();
    
  public:
    
    //mic
    int m_iMicNum ;
    float* m_pMics_Sl ;
    float* m_pMicI2sDelay ;
    z_mic_info* m_pMicInfo_Sl ;
    
    //engine
#ifdef USE_LZHU_SL
    r2_sl_htask m_hEngine_Sl ;
#else
    r2_sourcelocation_htask m_hEngine_Sl ;
#endif
    
    float** m_pData_Sl ;
    
  };
  
};


#endif /* __r2vt4__zsl__ */
