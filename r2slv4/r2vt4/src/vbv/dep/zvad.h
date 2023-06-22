//
//  zvad.h
//  r2vt4
//
//  Created by hadoop on 5/15/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zvad__
#define __r2vt4__zvad__

#include "NNVadIntf.h"
#include "../../zmath.h"
#include "../../mat/zvec.h"

namespace __r2vt4__ {
  
  
  class ZVad
  {
  public:
    ZVad(void);
  public:
    ~ZVad(void);
    
    int AddInData(const float* pData_In, int iLen_In);
    int Process(const float* pData_In, int iLen_In, bool bOutput);
    int GetVadFrmPos(int& iStart, int &iEnd);
    
    float  getenergy_Lastframe();
    float  getenergy_Threshold();
    
  public:
    int     m_iFrmSize ;
    
    int     m_iLen_In ;
    int     m_iLen_In_Total ;
    float * m_pData_In ;
    
    //vad
    VD_HANDLE m_hEngine_Vad ;
    int m_iVadState ;
    
    int m_iVadFrmPos ;
    int m_iVadStart ;
    int m_iVadEnd ;
    bool m_bDirtyReset ;
    bool m_bNeedRestart ;
    
    //No New
    ZVec* m_pData_NoNew ;
    
    
  };
  
  
};


#endif /* __r2vt4__zvad__ */
