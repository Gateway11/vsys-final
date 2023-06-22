//
//  zvbvmem.h
//  r2vt4
//
//  Created by hadoop on 3/13/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zvbvmem__
#define __r2vt4__zvbvmem__

#include "../zmath.h"

#include "dep/zbf.h"
#include "dep/zsl.h"
#include "dep/zns.h"
#include "dep/zvt.h"
#include "dep/zvad.h"
#include "dep/zdirty.h"

#include "../buff/zbuff.h"


#ifndef __ARM_ARCH_ARM__
#define _CB_VBV_AUDIO
#else
//#define _CB_VBV_AUDIO
#endif


namespace __r2vt4__ {
  
  //ZVbvSto--------------------------------------------------------------
  class ZVbvSto
  {
  public:
    ZVbvSto(int iMicNum, float* pMicPos, float* pMicDelay, const char* pVtNnetPath, const char* pVtPhoneTablePath);
    
  public:
    ~ZVbvSto(void);
    
  public:
    int m_iMicNum ;
    float* m_pMicPos ;
    float* m_pMicDelay ;
    
    z_mic_info* m_pMicInfo_Sl ;
    z_mic_info* m_pMicInfo_Bf ;
    
  };
  
  
  //ZVbvMem--------------------------------------------------------------
  class ZVbvMem
  {
  public:
    ZVbvMem(ZVbvSto* pVbvSto);
    
  public:
    ~ZVbvMem(void);
    
  public:
    int Process(const float** pWavBuff, int iWavLen, int iVtFlag, bool bDirtyReset);
    int Reset();
    
    int SetParam(const int iParam, const void* pValue);
    
    
  protected:
    static bool checkdirty_callback_proxy(void* param_cb, float fSlInfo[3]);
    bool my_checkdirty_callback(float fSlInfo[3]);
    
    static int getdata_callback_proxy(int nFrmStart, int nFrmEnd, float* pData, void* param_cb);
    int my_getdata_callback(int nFrmStart, int nFrmEnd, float* pData);
    
    static int dosecsl_callback_proxy(int nFrmStart_Sil, int nFrmEnd_Sil, int nFrmStart_Vt, int nFrmEnd_Vt, float* pSlInfo, float* pSilShield, void* param_cb );
    int my_dosecsl_callback(int nFrmStart_Sil, int nFrmEnd_Sil, int nFrmStart_Vt, int nFrmEnd_Vt, float* pSlInfo, float* pSilShield);
    
  public:
    
    int m_iMicNum ;
    int m_iFrmSize ;
    ZVbvSto* m_pVbvSto ;
    
    // vt sl
    
    
    ZNs*    m_pNs ;
    ZSl*    m_pSl ;
    ZVt*    m_pVt ;
    
    ZAudBuff*   m_pAudBuff ;
    //ZBf*  m_pBf_Vt ;
    //ZAudBuff_S* m_pAudBuff_Vt ;
    
    ZBf*  m_pBf_Vt_Ns ;
    ZAudBuff*   m_pAudBuff_Ns ;
    ZAudBuff_S* m_pAudBuff_Vt_Ns ;
    
    //ZBf*  m_pBf_Vt2_Ns ;
    //ZAudBuff_S* m_pAudBuff_Vt2_Ns ;
    
    
    //dirty
    ZVad* m_pVad ;
    ZDirty* m_pDirty ;
    int DegreeChange(float fAzimuth);
    int CheckDirty(float fAzimuth);
    bool m_bDirtySet_Aec ;
    bool m_bDirtySet_NoAec ;
    
    
    //No New
    int m_iCol_NoNew ;
    float** m_pData_NoNew ;
    
    
    //Avg Sil Shield
    bool m_bSilOver ;
    int m_iSilTotal ;
    int m_iSilCur ;
    float* m_pSilShield ;
    int SetSilShield(float fSilShield) ;
    
    
    
  };
  
};


#endif /* __r2vt4__zvbvmem__ */
