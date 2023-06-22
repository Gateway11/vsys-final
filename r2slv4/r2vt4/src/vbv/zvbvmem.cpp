//
//  zvbvmem.cpp
//  r2vt4
//
//  Created by hadoop on 3/13/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zvbvmem.h"
#include "zvbvapi.h"
#include "../aud/zaudapi.h"

namespace __r2vt4__ {
  
  //ZVbvSto--------------------------------------------------------------
  ZVbvSto::ZVbvSto(int iMicNum, float* pMicPos, float* pMicDelay, const char* pVtNnetPath, const char* pVtPhoneTablePath){
    
    //mic
    m_iMicNum = iMicNum ;
    
    m_pMicPos = Z_SAFE_NEW_AR1(m_pMicPos, float, m_iMicNum * 3);
    memcpy(m_pMicPos, pMicPos, sizeof(float) * 3 * m_iMicNum) ;
    
    m_pMicDelay = Z_SAFE_NEW_AR1(m_pMicDelay, float, m_iMicNum) ;
    memcpy(m_pMicDelay, pMicDelay, sizeof(float) * m_iMicNum) ;
    
    //sl
    m_pMicInfo_Sl = z_getdefaultmicinfo(m_iMicNum);
    
    //bf
    m_pMicInfo_Bf = z_getdefaultmicinfo(m_iMicNum);
    
    //global init
    r2_vt_sysinit(pVtNnetPath, pVtPhoneTablePath);
    
    r2ssp_ssp_init();
    
#ifdef  _CB_VBV_AUDIO
    char cbpath[256];
    sprintf(cbpath,"%s/cb/",DEBUG_FILE_LOCATION);
    z_mkdir(cbpath);
#endif
    
  }
  
  
  ZVbvSto::~ZVbvSto(void){
    
    z_free_micinfo(m_pMicInfo_Sl);
    z_free_micinfo(m_pMicInfo_Bf);
    
    Z_SAFE_DEL_AR1(m_pMicDelay);
    
    Z_SAFE_DEL_AR1(m_pMicPos);
    
    r2ssp_ssp_exit();
    r2_vt_sysexit();
    
  }
  
  
  //ZVbvMem--------------------------------------------------------------
  ZVbvMem::ZVbvMem(ZVbvSto* pVbvSto){
    
    //mic
    m_pVbvSto = pVbvSto ;
    m_iMicNum = pVbvSto->m_iMicNum ;
    m_iFrmSize = AUDIO_SAMPLE_RATE / 1000 * AUDIO_FRAME_MS ;
    
    
    m_pNs = Z_SAFE_NEW(m_pNs, ZNs, m_iMicNum, pVbvSto->m_pMicInfo_Bf, 2);
    
    m_pSl = Z_SAFE_NEW(m_pSl, ZSl, m_pVbvSto->m_iMicNum, m_pVbvSto->m_pMicPos, m_pVbvSto->m_pMicDelay, m_pVbvSto->m_pMicInfo_Sl) ;
    
    m_pVt = Z_SAFE_NEW(m_pVt, ZVt, m_pVbvSto->m_pMicInfo_Bf) ;
    m_pVt->SetCallback_SecSl(dosecsl_callback_proxy, this);
    m_pVt->SetCallback_CheckDirty(checkdirty_callback_proxy, this) ;
    m_pVt->SetCallback_GetBfData(getdata_callback_proxy, this);
    
    m_pAudBuff = Z_SAFE_NEW(m_pAudBuff, ZAudBuff, m_pVbvSto->m_iMicNum, AUDIO_SAMPLE_RATE * 5) ;
    //m_pBf_Vt = Z_SAFE_NEW(m_pBf_Vt, ZBf, m_pVbvSto->m_iMicNum, m_pVbvSto->m_pMicPos, m_pVbvSto->m_pMicDelay, m_pVbvSto->m_pMicInfo_Bf) ;
    //m_pAudBuff_Vt = Z_SAFE_NEW(m_pAudBuff_Vt, ZAudBuff_S, AUDIO_SAMPLE_RATE * 10 );
    

    
    m_pAudBuff_Ns = Z_SAFE_NEW(m_pAudBuff_Ns, ZAudBuff, m_pVbvSto->m_iMicNum, AUDIO_SAMPLE_RATE * 5) ;
    m_pBf_Vt_Ns = Z_SAFE_NEW(m_pBf_Vt_Ns, ZBf, m_pVbvSto->m_iMicNum, m_pVbvSto->m_pMicPos, m_pVbvSto->m_pMicDelay, m_pVbvSto->m_pMicInfo_Bf) ;
    m_pAudBuff_Vt_Ns = Z_SAFE_NEW(m_pAudBuff_Vt_Ns, ZAudBuff_S, AUDIO_SAMPLE_RATE * 5 );
    
    //m_pBf_Vt2_Ns = Z_SAFE_NEW(m_pBf_Vt_Ns, ZBf, m_pVbvSto->m_iMicNum, m_pVbvSto->m_pMicPos, m_pVbvSto->m_pMicDelay, m_pVbvSto->m_pMicInfo_Bf) ;
    //m_pAudBuff_Vt2_Ns = Z_SAFE_NEW(m_pAudBuff_Vt2_Ns, ZAudBuff_S, AUDIO_SAMPLE_RATE * 10 );
    
    //dirty
    unsigned int dMax = 6 ;
    time_t tDirtyTimeOut = 60 ;
    time_t tTryTimeOut = 10 ;
    time_t tDirtySuccessTimeOut = 20 ;
    //time_t tDirtySuccessTimeOut = 0 ;
    
    m_pDirty = Z_SAFE_NEW(m_pDirty,  ZDirty, true, dMax, tDirtyTimeOut, tTryTimeOut, tDirtySuccessTimeOut);
    m_pVad = Z_SAFE_NEW(m_pVad, ZVad);
    
    m_bDirtySet_Aec = false ;
    m_bDirtySet_NoAec = true ;
    
    
    m_iCol_NoNew = AUDIO_SAMPLE_RATE * 5  ;
    m_pData_NoNew = Z_SAFE_NEW_AR2(m_pData_NoNew, float, m_iMicNum, m_iCol_NoNew) ;
    
    //Avg Sil Shield
    m_bSilOver = false ;
    m_iSilTotal = 10 ;
    m_iSilCur = 0  ;
    m_pSilShield = Z_SAFE_NEW_AR1(m_pSilShield, float, m_iSilTotal)
  }
  
  ZVbvMem::~ZVbvMem(void){
    
    
    Z_SAFE_DEL(m_pNs) ;
    Z_SAFE_DEL(m_pSl) ;
    Z_SAFE_DEL(m_pVt) ;
    
    Z_SAFE_DEL(m_pAudBuff) ;
    //Z_SAFE_DEL(m_pBf_Vt) ;
    //Z_SAFE_DEL(m_pAudBuff_Vt) ;
    
    Z_SAFE_DEL(m_pAudBuff_Ns) ;
    Z_SAFE_DEL(m_pBf_Vt_Ns) ;
    Z_SAFE_DEL(m_pAudBuff_Vt_Ns) ;
    
    Z_SAFE_DEL(m_pDirty) ;
    Z_SAFE_DEL(m_pVad) ;
    
    Z_SAFE_DEL_AR2(m_pData_NoNew) ;
    Z_SAFE_DEL_AR1(m_pSilShield) ;
    
  }
  
  int ZVbvMem::Process(const float** pWavBuff, int iWavLen, int iVtFlag, bool bDirtyReset){
    
    //Ns
    float ** pData_Ns = NULL; int iLen_Ns = 0 ;
    m_pNs->Process(pWavBuff, iWavLen, pData_Ns, iLen_Ns) ;
    
    //Sl
    m_pSl->PutData(pData_Ns, iLen_Ns);
    
    
    //Should before Vt
    m_pAudBuff_Ns->PutAudio((const float** )pData_Ns, iLen_Ns);
    if (m_pAudBuff_Vt_Ns->m_bWorking) {
      float * pData_VtBf = NULL ; int iLen_VtBf = 0 ;
      m_pBf_Vt_Ns->Process((const float**)pData_Ns, iLen_Ns, pData_VtBf, iLen_VtBf);
      m_pAudBuff_Vt_Ns->PutBuff(pData_VtBf, iLen_VtBf);
      
      //m_pBf_Vt2_Ns->Process((const float**)pData_Ns, iLen_Ns, pData_VtBf, iLen_VtBf);
      //m_pAudBuff_Vt2_Ns->PutBuff(pData_VtBf, iLen_VtBf);
    }
    
    //Should before Vt
    m_pAudBuff->PutAudio(pWavBuff, iWavLen) ;
//    if (m_pAudBuff_Vt->m_bWorking) {
//      float * pData_VtBf = NULL ; int iLen_VtBf = 0 ;
//      m_pBf_Vt->Process((const float**)pData_Ns, iLen_Ns, pData_VtBf, iLen_VtBf);
//      m_pAudBuff_Vt->PutBuff(pData_VtBf, iLen_VtBf);
//    }
    
    //dirty
    bool bAec = iVtFlag & R2_VT_FLAG_AEC ;
    int rt_vad = m_pVad->Process(pData_Ns[m_pSl->m_pMicInfo_Sl->pMicIdLst[0]], iLen_Ns, bDirtyReset) ;
    if (rt_vad == 1) {
      float fSlInfo[3] ;
      int iStart = 0 , iEnd = 0 ;
      m_pVad->GetVadFrmPos(iStart, iEnd);
      m_pSl->GetSl(iStart, iEnd, fSlInfo);
      int degree = DegreeChange(fSlInfo[0]) ;
      
      if (m_pVad->m_bDirtyReset) {
        ZLOG_INFO("Dirty Free %d Size:%d", degree, iStart / 160);
        m_pDirty->setFree2(ZDirty::getIndexByDegree(degree));
      }else {
        if (bAec && m_bDirtySet_Aec) {
          ZLOG_INFO("Dirty Set %d Size:%d", degree, iStart / 160);
          m_pDirty->dirty2(ZDirty::getIndexByDegree(degree));
        }else if(!bAec && m_bDirtySet_NoAec){
          ZLOG_INFO("Dirty Set %d Size:%d", degree, iStart / 160);
          m_pDirty->dirty2(ZDirty::getIndexByDegree(degree));
        }else{
          ZLOG_INFO("Skip Dirty Set %d Size", degree, iStart / 160);
        }
      }
      ZLOG_INFO("----------%f", m_pVad->getenergy_Threshold() ) ;
      SetSilShield(m_pVad->getenergy_Threshold());
    }
    
    //Vt
    int rt_vt =  m_pVt->Process((const float**)pData_Ns, iLen_Ns, iVtFlag);
    
    if (rt_vt & R2_VT_WORD_PRE) {

      assert(m_pAudBuff_Vt_Ns->m_bWorking == false) ;
      m_pAudBuff_Vt_Ns->m_bWorking = true ;
      //m_pAudBuff_Vt2_Ns->m_bWorking = true ;
//      assert(m_pAudBuff_Vt->m_bWorking == false) ;
//      m_pAudBuff_Vt->m_bWorking = true ;
      ZLOG_INFO("--------------r2mem_vbv_vt_awake_pre") ;
    }
    
    if (rt_vt & R2_VT_WORD_DET) {
      ZLOG_INFO("--------------r2mem_vbv_vt_awake");
    }
    
    if (rt_vt & R2_VT_WORD_CANCEL) {
      assert(m_pAudBuff_Vt_Ns->m_bWorking) ;
      m_pAudBuff_Vt_Ns->Reset();
      //m_pAudBuff_Vt2_Ns->m_bWorking = true ;
//      assert(m_pAudBuff_Vt->m_bWorking) ;
//      m_pAudBuff_Vt->Reset();
      ZLOG_INFO("--------------r2mem_vbv_vt_awake_cancel");
    }
    
    if (rt_vt & R2_VT_WORD_DET_CMD) {
      assert(m_pAudBuff_Vt_Ns->m_bWorking) ;
      m_pAudBuff_Vt_Ns->Reset();
      //m_pAudBuff_Vt2_Ns->m_bWorking = true ;
//      assert(m_pAudBuff_Vt->m_bWorking) ;
//      m_pAudBuff_Vt->Reset();
      ZLOG_INFO("--------------r2mem_vbv_vt_awake_cmd");
    }
    
    if (rt_vt & R2_VT_WORD_DET_NOCMD) {
      assert(m_pAudBuff_Vt_Ns->m_bWorking) ;
      m_pAudBuff_Vt_Ns->Reset();
      //m_pAudBuff_Vt2_Ns->m_bWorking = true ;
//      assert(m_pAudBuff_Vt->m_bWorking) ;
//      m_pAudBuff_Vt->Reset();
      ZLOG_INFO("--------------r2mem_vbv_vt_awake_nocmd");
    }
    
    return  rt_vt  ;
    
  }
  
  int ZVbvMem::Reset(){
    
    m_pVt->Reset() ;
    m_pAudBuff_Vt_Ns->Reset() ;
//    m_pAudBuff_Vt->Reset() ;
    return 0 ;
    
  }
  
  int ZVbvMem::SetParam(const int iParam, const void* pValue){
    
    if (iParam == R2_VBV_PARAM_INT_DIRTY_SET_NOAEC) {
      int value = * (int*) pValue ;
      if (value == 0 ) {
        m_bDirtySet_NoAec = false ;
      }else{
        m_bDirtySet_NoAec = true ;
      }
    }
    
    if (iParam == R2_VBV_PARAM_INT_DIRTY_SET_AEC) {
      int value = * (int*) pValue ;
      if (value == 0 ) {
        m_bDirtySet_Aec = false ;
      }else{
        m_bDirtySet_Aec = true ;
      }
    }
    
    return  0 ;
  }
  
  int ZVbvMem::dosecsl_callback_proxy(int nFrmStart_Sil, int nFrmEnd_Sil, int nFrmStart_Vt, int nFrmEnd_Vt, float* pSlInfo, float* pSilShield, void* param_cb ){
    
    ZVbvMem* pVbv2 = (ZVbvMem*) param_cb ;
    return pVbv2->my_dosecsl_callback(nFrmStart_Sil, nFrmEnd_Sil, nFrmStart_Vt, nFrmEnd_Vt, pSlInfo, pSilShield);
    
  }
  
  int ZVbvMem::my_dosecsl_callback(int nFrmStart_Sil, int nFrmEnd_Sil, int nFrmStart_Vt, int nFrmEnd_Vt, float* pSlInfo, float* pSilShield){
    
    assert(nFrmStart_Sil > nFrmEnd_Sil && nFrmStart_Vt > nFrmEnd_Vt) ;
    
    int iOffset = m_pVt->GetLeftPos() ;
    
    int iStart_Sil = ( nFrmStart_Sil + 10 ) * m_iFrmSize + iOffset ;
    int iEnd_Sil = nFrmEnd_Sil * m_iFrmSize + iOffset ;
    int iStart_Vt = nFrmStart_Vt * m_iFrmSize + iOffset ;
    int iEnd_Vt = nFrmEnd_Vt * m_iFrmSize + iOffset ;
    
    //float** pData1 = Z_SAFE_NEW_AR2(pData1, float, m_iMicNum, iStart_Sil);
    
    if (iStart_Sil > m_iCol_NoNew) {
      m_iCol_NoNew = iStart_Sil * 2 ;
      Z_SAFE_DEL_AR2(m_pData_NoNew);
      m_pData_NoNew = Z_SAFE_NEW_AR2(m_pData_NoNew, float, m_iMicNum, m_iCol_NoNew) ;
    }
    
    float * pData3 = NULL ;
    int iLen3 = 0 ;
    //m_pSl->GetSl(iStart_Vt, iEnd_Vt,pSlInfo) ;
    m_pSl->GetSl2(iStart_Vt, iEnd_Vt, pSlInfo) ;
    
    //ns
    m_pAudBuff_Ns->GetLastAudio(m_pData_NoNew, iStart_Sil , 0) ;
    m_pBf_Vt_Ns->Reset() ;
    m_pBf_Vt_Ns->Steer(pSlInfo[0], pSlInfo[1]);
    m_pBf_Vt_Ns->Process((const float**)m_pData_NoNew, iStart_Sil, pData3, iLen3);
    
    assert(!m_pAudBuff_Vt_Ns->m_bWorking) ;
    m_pAudBuff_Vt_Ns->Reset() ;
    m_pAudBuff_Vt_Ns->PutBuff(pData3, iLen3);
    
    //m_pBf_Vt2_Ns->Reset() ;
    //m_pBf_Vt2_Ns->Steer(pSlInfo[3], pSlInfo[4]);
    //m_pBf_Vt2_Ns->Process((const float**)m_pData_NoNew, iStart_Sil, pData3, iLen3);
    //
    //assert(!m_pAudBuff_Vt2_Ns->m_bWorking) ;
    //m_pAudBuff_Vt2_Ns->Reset() ;
    //m_pAudBuff_Vt2_Ns->PutBuff(pData3, iLen3);
    
    //no ns
//    m_pAudBuff->GetLastAudio(m_pData_NoNew, iStart_Sil , 0) ;
//    m_pBf_Vt->Reset() ;
//    m_pBf_Vt->Steer(pSlInfo[0], pSlInfo[1]);
//    m_pBf_Vt->Process((const float**)m_pData_NoNew, iStart_Sil, pData3, iLen3);
//    
//    assert(!m_pAudBuff_Vt->m_bWorking) ;
//    m_pAudBuff_Vt->Reset() ;
//    m_pAudBuff_Vt->PutBuff(pData3, iLen3);
    
    
    if (m_bSilOver) {
      float fSilShield = 0.0f ;
      for (int i = 0 ; i < m_iSilTotal ; i ++) {
        fSilShield += m_pSilShield[i] ;
      }
      fSilShield = fSilShield / m_iSilTotal ;
      *pSilShield = fSilShield ;
    }else{
      if (m_iSilCur == 0) {
        *pSilShield = m_pVad->getenergy_Threshold() ;
      }else{
        float fSilShield = 0.0f ;
        for (int i = 0 ; i < m_iSilCur ; i ++) {
          fSilShield += m_pSilShield[i] ;
        }
        fSilShield = fSilShield / m_iSilCur ;
        *pSilShield = fSilShield ;
      }
    }
    
    
    ZLOG_INFO("--------------Second BF: %s", m_pBf_Vt_Ns->GetSlInfo());
    
    return  0 ;
    
  }
  
  bool ZVbvMem::checkdirty_callback_proxy(void* param_cb, float fSlInfo[3]){
    
    ZVbvMem* pVbv2 = (ZVbvMem*) param_cb ;
    return  pVbv2->my_checkdirty_callback(fSlInfo);
    
  }
  
  bool ZVbvMem::my_checkdirty_callback(float fSlInfo[3]){
    
    int degree = DegreeChange(fSlInfo[0]) ;
    ZLOG_INFO("Dirty Check %d", degree);
    
    return m_pDirty->check(ZDirty::getIndexByDegree(degree));
    
    
  }
  
  int ZVbvMem::getdata_callback_proxy(int nFrmStart, int nFrmEnd, float* pData, void* param_cb){
    
    ZVbvMem* pVbv2 = (ZVbvMem*) param_cb ;
    return  pVbv2->my_getdata_callback(nFrmStart, nFrmEnd, pData );
    
  }
  
  int ZVbvMem::my_getdata_callback(int nFrmStart, int nFrmEnd, float* pData){
    
    if (nFrmStart > 0) {
      int iOffset = m_pVt->GetLeftPos() ;
      int iStart = nFrmStart * m_iFrmSize + iOffset ;
      int iEnd = nFrmEnd * m_iFrmSize  + iOffset ;
      
      m_pAudBuff_Vt_Ns->GetBuff(pData, iStart , iEnd) ;
      
    }else{
      
      int iOffset = m_pVt->GetLeftPos() ;
      int iStart = -nFrmStart * m_iFrmSize + iOffset ;
      int iEnd = nFrmEnd * m_iFrmSize  + iOffset ;
      
      //m_pAudBuff_Vt2_Ns->GetBuff(pData, iStart , iEnd) ;
      assert(0);
    }
    
    
    return  0 ;
    
  }
  
  int ZVbvMem::DegreeChange(float fAzimuth){
    
    int iDegree = (fAzimuth - 3.1415936f) * 180 / 3.1415936f + 0.5f  ;
    //  if (m_iMicNum == 8) {
    //    iDegree = 90 - iDegree ;
    //  }
    while (iDegree < 0) {
      iDegree += 360 ;
    }
    while (iDegree > 360) {
      iDegree -= 360 ;
    }
    
    return iDegree ;
  }
  
  int ZVbvMem::CheckDirty(float fAzimuth){
    
    int degree = DegreeChange(fAzimuth) ;
    ZLOG_INFO("Dirty Check %d", degree);
    
    return m_pDirty->check(ZDirty::getIndexByDegree(degree)) ;
  }
  
  int ZVbvMem::SetSilShield(float fSilShield){
    
    m_pSilShield[m_iSilCur] = fSilShield ;
    m_iSilCur ++ ;
    if (m_iSilCur == m_iSilTotal) {
      m_bSilOver = true ;
    }
    m_iSilCur = m_iSilCur % m_iSilTotal ;
    return 0 ;
  }
  
};




