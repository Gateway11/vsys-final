//
//  zvt.h
//  r2vt4
//
//  Created by hadoop on 3/19/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zvt__
#define __r2vt4__zvt__

#include "../../vt/zvtapi.h"
#include "../../zmath.h"

namespace __r2vt4__ {
  
  class ZVt
  {
  public:
    ZVt(z_mic_info* pMicInfo_Vt);
  public:
    ~ZVt(void);
    
    int SetWords(const WordInfo* pWordLst, int iWordNum);
    int GetWords(const WordInfo** pWordLst, int* iWordNum);
    
    int Process(const float** pWavBuff, int iWavLen, int iVtFlag);
    int GetDetWordInfo(const WordInfo** pWordInfo, const WordDetInfo** pWordDetInfo);
    
    int Reset();
    
    int GetLeftPos() ;
    
  public:
    
    int SetCallback_SecSl(dosecsl_callback fun_cb, void* param_cb);
    int SetCallback_CheckDirty(checkdirty_callback fun_cb, void* param_cb);
    int SetCallback_GetBfData(getdata_callback fun_cb, void* param_cb);
    
    r2_vt_htask m_hEngine_Vt ;
    z_mic_info* m_pMicInfo_Vt ;
    
    int m_iBatchSize ;
    int m_iFrmSize ;
    int m_iFrmBatchNum ;
    
    float** m_pData_In ;
    int m_iLen_In ;
    int m_iLen_In_Total ;
    
    WordInfo m_WordInfo ;
    WordDetInfo m_WordDetInfo ;
    
  };
  
};


#endif /* __r2vt4__zvt__ */
