//
//  zvtdet.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zvtdet__
#define __r2vt4__zvtdet__

#include "../../zmath.h"

#include "../zvtapi.h"


#include "../../cf/zcfapi.h"
#include "../../buff/zbuff.h"
#include "zphosto.h"
#include "zsildet3.h"

#ifndef __ARM_ARCH_ARM__
#define _CB_AUDIO
#else
//#define _CB_AUDIO
#endif


namespace __r2vt4__ {
  
  struct ZVtCand{
    int     pre ;
    float   score ;
    float   framescore ;
    float   blockscore ;
    float   totalblockscore_avg ;
  };
  
  enum ZVtDetStatus{
    vtdet_ready = 100, //need data,fea,dnn
    vtdet_process, //need data,fea,dnn,asr
    vtdet_finish //
  };
  
  class ZVtDet
  {
  public:
    ZVtDet(ZVtWord* pVtWord, int iFrmSize, int iFrmOffset, int iCn);
    ~ZVtDet(void);
    
  public:
    int Reset();
    int ProcessScore(int iFrmSize, ZMat* pFeaScore, ZAudBuff* pWavBuff, int iVtFlag);
    int CheckStatus(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec);
    int GetBlockScore(float* pScore, int iMaxScoreLen);
    bool IsNeedReset();
    bool IsNeedDnn();
    
  public:
    const WordDetInfo* GetDetWordInfo(int offset);
    float GetBestScore(int iFrmSize, ZMat* pFeaScore, ZAudBuff* pWavBuff);
    
    //do second sl callback
    int SetDoSecSlCallback(dosecsl_callback fun_cb, void* param_cb);
    dosecsl_callback m_Fun_Ds_Cb ;
    void* m_Param_Ds_Cb ;
    
    //dirty callback
    int SetCheckDirtyCallback(checkdirty_callback fun_cb, void* param_cb);
    checkdirty_callback m_Fun_Cd_Cb;
    void* m_Param_Cd_Cb ;
    
    //get data callback
    int SetGetDataCallback(getdata_callback fun_cb, void* param_cb);
    getdata_callback m_Fun_Gd_Cb;
    void* m_Param_Gd_Cb ;
    
  protected:
    int GetBestLen(int iFrmId);
    int DynamicProgramming(float* pFrmScore);
    int UpdateFrmScore(float* pFrmScore);
    inline int SetInValidCand(ZVtCand &pCand);
    
  protected:
    int ProcessMsg_Pre(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec);
    int ProcessMsg_Best(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec);
    int ProcessMsg_Energy(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec);
    int ProcessMsg_LeftSil(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec);
    int ProcessMsg_CfCheck(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec);
    int ProcessMsg_RightSil(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec);
    
  public:
    int m_iFrmSize ;
    int m_iFrmOffset ;
    ZVtWord*    m_pVtWord ;
    
    //SilCheck
    ZSilDet3*   m_pSilDet3 ;
    
    //CfCheck
    r2_cf_htask m_hCfTask ;
    
    //Cand Info
    int m_iCandLen_Total ;
    ZVtCand**   m_pCandLst ;
    float*      m_pCandScore_Best ;
    int*        m_pCandId_Best ;
    
    //Status
    //0-pre-1-detet-2-check-reset-0
    ZVtDetStatus m_iVtDetStatus ;
    
    //Pos
    int m_iFrmId_Cur ;
    int m_iFrmId_Start ;
    int m_iFrmId_Pre ;
    int m_iFrmId_Best ;
    int m_iFrmId_Energy ;
    int m_iFrmId_LeftSil ;
    int m_iFrmId_CfCheck ;
    int m_iFrmId_RightSil ;
    
    float m_fBestScore_30 ;
    int m_iFrmId_Best_30 ;
    
    int m_iFlag_Sl ;
    
    
    float m_fBestScore ;
    
    WordDetInfo m_WordDetInfo ;
    
    //Default Cand
    ZVtCand m_pDefaultCand ;
    
    //Debug
    //float* m_pCandDebugScore ;
    
  };

};


#endif /* __r2vt4__zvtdet__ */
