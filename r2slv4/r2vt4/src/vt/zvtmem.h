//
//  zvtmem.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zvtmem__
#define __r2vt4__zvtmem__

#include "../zmath.h"
#include "../dnn/znnet.h"
#include "../fea/zfea2.h"
#include "../fea/zfilter.h"
#include "../buff/zbuff.h"

#include "zvtapi.h"
#include "dep/zvtdet.h"
#include "zvtsto.h"

namespace __r2vt4__ {

  //ZVtMem--------------------------------------------------------------
  class ZVtMem
  {
  public:
    ZVtMem(ZVtSto* pVtSto, int iCn);
    
  public:
    ~ZVtMem(void);
    
  public:
    int SetWords(const WordInfo* pWordLst, int iWordNum) ;
    int GetWords(const WordInfo** pWordLst, int* iWordNum) ;
    int CheckWordsChanged();
    
  public:
    int Reset();
    int Detect(const float** pWavBuff, int iWavLen, int iFlag);
    int GetDnnOutput(const float** pWavBuff, int iWavLen, ZMat* &pDnnOutput, int &iFeaNum);
    int GetDetWordInfo(const WordInfo** pWordInfo, const WordDetInfo** pWordDetInfo, int iOffsetFrm);
    
  public:
    
    float GetBestScore(const float** pWavBuff, int iWavLen);
    
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
    
  public:
    
    //Fea
    int       m_iCn ;
    std::vector<ZFilter*>	m_FilterLst ;
    ZFea2*    m_pFea2 ;
    
    //Sto
    ZVtSto* m_pVtSto ;
    Nnet_Memory_Batch*  m_pNnetMem ;
    
    //VtDet
    bool    m_bNeedChangeWord ;
    int     m_iWordNum ;
    WordInfo* m_pWordLst ;
    std::vector<ZVtWord*> m_pVtWordLst ;
    
    
    int     m_iVtDetNum ;
    int*    m_pVtDetRt ;
    ZVtDet** m_VtDetLst ;
    
    int     m_iCurVtDetId ;
    
    //Buff
    //ZMatBuff*   m_pFeaBuff ;
    ZAudBuff*   m_pWavBuff ;
    
    
    //Debug File
    
    
  };

};


#endif /* __r2vt4__zvtmem__ */
