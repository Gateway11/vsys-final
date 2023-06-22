//
//  zbf.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zbf__
#define __r2vt4__zbf__

#include "../../zmath.h"
#include "r2ssp.h"

namespace __r2vt4__ {
  
  class ZBf
  {
  public:
    ZBf(int iMicNum, float* pMicPosLst, float* pMicDelay, z_mic_info* pMicInfo_Bf);
    
  public:
    ~ZBf(void);
    
  public:
    int Reset();
    int Process(const float** pData_In, int iLen_In, float*& pData_Out, int& iLen_Out);
    
    int Steer(float fAzimuth, float fElevation, int bSteer = 1);
    const char* GetSlInfo();
    
    //bool CheckSl(float fAzimuth, float fElevation);
    
  public:
    
    //in
    int m_iMicNum ;
    int m_iFrmSize ;
    
    float m_fSlInfo[3] ;
    
    //pos
    z_mic_info* m_pMicInfo_Bf ;
    float* m_pMics_Bf ;
    float* m_pMicI2sDelay ;
    
    float * m_pData_Bf ;
    
    //out
    int m_iLen_Out_Total ;
    float* m_pData_Out ;
    
    r2ssp_handle m_hEngine_Bf ;
    
    //output
    std::string m_strSlInfo ;
    
  };
  
};


#endif /* __r2vt4__zbf__ */
