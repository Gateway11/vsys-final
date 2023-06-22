//
//  zvtmem.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zvtmem.h"

namespace __r2vt4__ {

  //ZVtMem--------------------------------------------------------------
  ZVtMem::ZVtMem(ZVtSto* pVtSto, int iCn){
    
    m_pVtSto = pVtSto ;
    m_iCn = iCn ;
    
    //Fea
    m_pFea2 = Z_SAFE_NEW(m_pFea2,ZFea2,m_pVtSto->m_iSr,m_pVtSto->m_iBat, iCn) ;
    
    
    ZFilter* pFilter = NULL ;
    pFilter = Z_SAFE_NEW(pFilter,ZFilter_Delt,-2,2,m_pFea2->m_iFbDim,m_pFea2->m_iFbDim * 2) ;
    m_FilterLst.push_back(pFilter);
    pFilter = Z_SAFE_NEW(pFilter,ZFilter_Delt,-2,2,m_pFea2->m_iFbDim * 2,m_pFea2->m_iFbDim * 3);
    m_FilterLst.push_back(pFilter);
    pFilter = Z_SAFE_NEW(pFilter,ZFilter_Norm,0,0,m_pFea2->m_iFbDim * 3,m_pFea2->m_iFbDim * 3) ;
    m_FilterLst.push_back(pFilter);
    pFilter = Z_SAFE_NEW(pFilter,ZFilter_Cmb,-5,5,m_pFea2->m_iFbDim * 3,m_pFea2->m_iFbDim * 3 * 11) ;
    m_FilterLst.push_back(pFilter);
    
    
    //Dnn
    m_pNnetMem = Z_SAFE_NEW(m_pNnetMem, Nnet_Memory_Batch, m_pVtSto->m_pNnet, 4, 3);
    
    
    //Words
    m_iWordNum = 0 ;
    m_pWordLst = NULL ;
    m_bNeedChangeWord = false ;
    
    //CB
    m_Fun_Ds_Cb = NULL ;
    m_Param_Ds_Cb = NULL ;
    
    m_Fun_Cd_Cb = NULL ;
    m_Param_Cd_Cb = NULL ;
    
    m_Fun_Gd_Cb = NULL ;
    m_Param_Gd_Cb = NULL  ;
    
    //VtDet
    m_iVtDetNum = 0 ;
    m_VtDetLst = NULL ;
    m_pVtDetRt = NULL ;
    m_iCurVtDetId = -1 ;
    
    //Buff
    //m_pFeaBuff = Z_SAFE_NEW(m_pFeaBuff,ZMatBuff,100,m_pVtSto->m_pNnet->m_iOutputDim) ;
    m_pWavBuff = Z_SAFE_NEW(m_pWavBuff,ZAudBuff, iCn,m_pVtSto->m_iSr * 10);
    
    //Debug
#ifdef  _VT_AUDIO_
    char vtpath[256];
    sprintf(vtpath,"%s/realvt/",DEBUG_FILE_LOCATION);
    z_mkdir(vtpath);
#endif
    
  }
  
  ZVtMem::~ZVtMem(void){
    
    Z_SAFE_DEL(m_pFea2);
    for (int i = 0 ; i < m_FilterLst.size() ; i ++) {
      Z_SAFE_DEL(m_FilterLst[i]);
    }
    Z_SAFE_DEL(m_pNnetMem);
    
    Z_SAFE_DEL_AR1(m_pWordLst) ;
    for (int i = 0 ; i < m_iWordNum ; i ++) {
      Z_SAFE_DEL(m_pVtWordLst[i]) ;
    }
    
    for (int i = 0  ; i < m_iVtDetNum ; i ++) {
      Z_SAFE_DEL(m_VtDetLst[i]);
    }
    Z_SAFE_DEL_AR1(m_pVtDetRt);
    Z_SAFE_DEL_AR1(m_VtDetLst);
    
    //Z_SAFE_DEL(m_pFeaBuff);
    Z_SAFE_DEL(m_pWavBuff);
  }
  
  int ZVtMem::SetWords(const WordInfo* pWordLst, int iWordNum){
  
    if (iWordNum > 0) {
      std::vector<ZVtWord*> pVtWordLst = m_pVtSto->ParseWordTriPhoLst(pWordLst, iWordNum) ;
      if (pVtWordLst.size() == iWordNum) {
        
        m_bNeedChangeWord = true ;
        if (m_pWordLst != NULL) {
          Z_SAFE_DEL_AR1(m_pWordLst) ;
          for (int i = 0 ; i < m_iWordNum ; i ++) {
            Z_SAFE_DEL(m_pVtWordLst[i]) ;
          }
          m_iWordNum = 0 ;
        }
        
        m_iWordNum  = iWordNum ;
        m_pWordLst = Z_SAFE_NEW_AR1(m_pWordLst, WordInfo, m_iWordNum) ;
        memcpy(m_pWordLst, pWordLst, sizeof(WordInfo) * m_iWordNum) ;
        m_pVtWordLst = pVtWordLst ;
        
      }else{
        for (int i = 0 ; i < pVtWordLst.size() ; i ++) {
          Z_SAFE_DEL(pVtWordLst[i]) ;
        }
      }
      
    }
    
    
    
    if (this->m_bNeedChangeWord ) {
      return  0 ;
    }else{
      return 1 ;
    }
  }
  
  int ZVtMem::GetWords(const WordInfo** pWordLst, int* iWordNum){
    
    *pWordLst = m_pWordLst ;
    *iWordNum = m_iWordNum ;
    
    return  0 ;
    
  }
  
  int ZVtMem::CheckWordsChanged(){
    
    if (m_iCurVtDetId == -1 && m_bNeedChangeWord) {
      
      assert(m_iWordNum > 0);
      
      m_bNeedChangeWord = false ;
      
      if (m_iVtDetNum > 0) {
        for (int i = 0  ; i < m_iVtDetNum ; i ++) {
          Z_SAFE_DEL(m_VtDetLst[i]);
        }
        Z_SAFE_DEL_AR1(m_pVtDetRt);
        Z_SAFE_DEL_AR1(m_VtDetLst);
        m_iVtDetNum = 0 ;
      }
      
      if (m_iWordNum > 0) {
        m_iVtDetNum = m_iWordNum ;
        m_pVtDetRt = Z_SAFE_NEW_AR1(m_pVtDetRt,int, m_iVtDetNum);
        m_VtDetLst = Z_SAFE_NEW_AR1(m_VtDetLst,ZVtDet*, m_iVtDetNum);
        for (int i = 0 ; i < m_iVtDetNum ; i ++) {
          m_VtDetLst[i] = Z_SAFE_NEW(m_VtDetLst[i],ZVtDet,m_pVtWordLst[i], m_pVtSto->m_iFrmSize, m_pVtSto->m_iFrmOffset, m_iCn);
          m_VtDetLst[i]->SetDoSecSlCallback(m_Fun_Ds_Cb, m_Param_Ds_Cb) ;
          m_VtDetLst[i]->SetCheckDirtyCallback(m_Fun_Cd_Cb, m_Param_Cd_Cb) ;
          m_VtDetLst[i]->SetGetDataCallback(m_Fun_Gd_Cb, m_Param_Gd_Cb) ;

        }
        m_iCurVtDetId = -1 ;
      }
      
      
    }
    
    
    
    return  0 ;
    
  }
  
  int ZVtMem::Reset(){
    
    m_iCurVtDetId = -1 ;
    
    m_pFea2->ResetFb() ;
    for (int i = 0 ; i < m_FilterLst.size() ; i ++) {
      m_FilterLst[i]->ResetFilter() ;
    }
  
    m_pNnetMem->Reset() ;
    
    for (int i = 0 ; i < m_iVtDetNum ; i ++) {
      m_VtDetLst[i]->Reset() ;
    }
    
    //m_pFeaBuff->Reset() ;
    m_pWavBuff->Reset() ;
    
    return  0 ;
  }
  
  int ZVtMem::Detect(const float** pWavBuff, int iWavLen, int iVtFlag){
    
    CheckWordsChanged() ;
    
    if (iWavLen <= 0) {
      return  0 ;
    }
    
    ZMat* pDnnOutput = NULL ;
    int iOutputNum = 0 ;
    GetDnnOutput(pWavBuff, iWavLen, pDnnOutput, iOutputNum);
    
    //Store Fea
    m_pWavBuff->PutAudio(pWavBuff, iWavLen);
    //m_pFeaBuff->PutBuff((const float**)pDnnOutput->data, iOutputNum);
    
    //Check Output
    if(m_iCurVtDetId != -1){
      if (m_VtDetLst[m_iCurVtDetId]->IsNeedReset()) {
        m_iCurVtDetId = -1 ;
      }
    }
    for (int i = 0 ; i < m_iVtDetNum ; i ++) {
      if (m_VtDetLst[i]->IsNeedReset()) {
        m_VtDetLst[i]->Reset() ;
      }
    }
    
    //Do
    memset(m_pVtDetRt, 0, sizeof(int) * m_iVtDetNum);
    for (int i = 0 ; i < m_iVtDetNum ; i ++) {
      m_pVtDetRt[i] = m_VtDetLst[i]->ProcessScore(iOutputNum, pDnnOutput, m_pWavBuff, iVtFlag);
    }
    
    if (m_iCurVtDetId == -1) {
      for (int i = 0 ; i < m_iVtDetNum ; i ++) {
        if (m_pVtDetRt[i] & R2_VT_WORD_PRE) {
          m_iCurVtDetId = i ;
          break ;
        }
      }
    }
    
#ifdef  _VT_AUDIO_
    if (m_iCurVtDetId != -1) {
      if ((m_pVtDetRt[m_iCurVtDetId] & R2_VT_WORD_DET_CMD ) ||  (m_pVtDetRt[m_iCurVtDetId] & R2_VT_WORD_DET_NOCMD)  ) {
        //
        char VtAudioPath[512], VtFeaPath[512], AllAudioPath[512];
        std::string datatime = z_getdatatime();
        if (m_VtDetLst[m_iCurVtDetId]->m_pVtWord->m_iWordId == 0) {
          if (m_pVtDetRt[m_iCurVtDetId] & R2_VT_WORD_DET_CMD) {
            sprintf(VtAudioPath, "%s/realvt/%s.awake.cmd.pcm",DEBUG_FILE_LOCATION,datatime.c_str());
            sprintf(VtFeaPath, "%s/realvt/%s.awake.cmd.fea",DEBUG_FILE_LOCATION,datatime.c_str());
          }else{
            sprintf(VtAudioPath, "%s/realvt/%s.awake.nocmd.pcm",DEBUG_FILE_LOCATION,datatime.c_str());
            sprintf(VtFeaPath, "%s/realvt/%s.awake.nocmd.fea",DEBUG_FILE_LOCATION,datatime.c_str());
          }
          sprintf(AllAudioPath, "%s/realvt/%s.awake.all.pcm",DEBUG_FILE_LOCATION,datatime.c_str());
        }else{
          if (m_pVtDetRt[m_iCurVtDetId] & R2_VT_WORD_DET_CMD) {
            sprintf(VtAudioPath, "%s/realvt/%s.sleep.cmd.pcm",DEBUG_FILE_LOCATION,datatime.c_str());
            sprintf(VtFeaPath, "%s/realvt/%s.sleep.cmd.fea",DEBUG_FILE_LOCATION,datatime.c_str());
          }else{
            sprintf(VtAudioPath, "%s/realvt/%s.sleep.nocmd.pcm",DEBUG_FILE_LOCATION,datatime.c_str());
            sprintf(VtFeaPath, "%s/realvt/%s.sleep.nocmd.fea",DEBUG_FILE_LOCATION,datatime.c_str());
          }
          sprintf(AllAudioPath, "%s/realvt/%s.sleep.all.pcm",DEBUG_FILE_LOCATION,datatime.c_str());
        }
        
        
        int iVtAudioLen = m_VtDetLst[m_iCurVtDetId]->m_iFrmId_Cur - m_VtDetLst[m_iCurVtDetId]->m_iFrmId_Start
        + m_pVtSto->m_iFrmOffset + m_VtDetLst[m_iCurVtDetId]->m_iFrmOffset_LeftSilDet + 20 ;
        iVtAudioLen = iVtAudioLen * m_pVtSto->m_iFrmSize ;
        m_pWavBuff->StoreFile(VtAudioPath, iVtAudioLen, 0);
        
        //m_pWavBuff->StoreFile(AllAudioPath, m_pWavBuff->m_iCurPos, 0);
        
        //                int iVtFeaLen = m_VtDetLst[m_iCurVtDetId]->m_iFrmId_Cur - m_VtDetLst[m_iCurVtDetId]->m_iFrmId_Start ;
        //                m_pFeaBuff->StoreFile(VtFeaPath, iVtFeaLen, 0);
        
      }
    }
#endif
    
    
    if (m_iCurVtDetId != -1) {
      return m_pVtDetRt[m_iCurVtDetId];
    }else{
      return  0 ;
    }
  }
  
  int ZVtMem::GetDnnOutput(const float** pWavBuff, int iWavLen, ZMat* &pDnnOutput, int &iFeaNum){
    
    
    //Fb
    m_pFea2->ContinueFb(pWavBuff,iWavLen,pDnnOutput,iFeaNum);
    if (iFeaNum < 1){
      return 0 ;
    }
    
    //Filter
    for (int i = 0 ; i < m_FilterLst.size() ; i ++){
      m_FilterLst[i]->FilterFea(pDnnOutput,iFeaNum,pDnnOutput,iFeaNum);
      if (iFeaNum < 1){
        return 0 ;
      }
    }
    
    //z_storedata_mat("/Users/hadoop/Documents/compare/16.dat", pDnnOutput->data, iFeaNum, pDnnOutput->col);
    
    //Dnn need by socre
    m_pNnetMem->GetOutPut(pDnnOutput, iFeaNum, pDnnOutput, iFeaNum);
    
    return  0 ;
    
  }
  
  int ZVtMem::GetDetWordInfo(const WordInfo** pWordInfo, const WordDetInfo** pWordDetInfo, int iOffsetFrm){
  
    if (m_iCurVtDetId == -1) {
      *pWordInfo = NULL ;
      *pWordDetInfo = NULL ;
      return  1 ;
    }else{
      *pWordInfo = m_pWordLst + m_iCurVtDetId ;
      *pWordDetInfo = m_VtDetLst[m_iCurVtDetId]->GetDetWordInfo(iOffsetFrm);
      return  0 ;
    }
    
  }
  
  
  float ZVtMem::GetBestScore(const float** pWavBuff, int iWavLen){
    
    //assert(m_iVtDetNum == 1 );
    CheckWordsChanged() ;
    
    m_iCurVtDetId = 0 ;
    
    ZMat* pDnnOutput = NULL ;
    int iOutputNum = 0 ;
    GetDnnOutput(pWavBuff, iWavLen, pDnnOutput, iOutputNum);
    
    //Store Fea
    m_pWavBuff->PutAudio(pWavBuff, iWavLen);
    
    
    //Do
    return m_VtDetLst[m_iCurVtDetId]->GetBestScore(iOutputNum, pDnnOutput, m_pWavBuff) ;
  }

  
  int ZVtMem::SetDoSecSlCallback(dosecsl_callback fun_cb, void* param_cb){
    
    m_Fun_Ds_Cb = fun_cb ;
    m_Param_Ds_Cb = param_cb ;
    
    for (int i = 0 ; i < m_iVtDetNum ; i ++) {
      m_VtDetLst[i]->SetDoSecSlCallback(fun_cb, param_cb) ;
    }
    return 0 ;
    
  }
  
  int ZVtMem::SetCheckDirtyCallback(checkdirty_callback fun_cb, void* param_cb){
    
    m_Fun_Cd_Cb = fun_cb ;
    m_Param_Cd_Cb = param_cb ;
    
    for (int i = 0 ; i < m_iVtDetNum ; i ++) {
      m_VtDetLst[i]->SetCheckDirtyCallback(fun_cb, param_cb) ;
    }
    return 0 ;
    
  }
  
  int ZVtMem::SetGetDataCallback(getdata_callback fun_cb, void* param_cb){
    
    m_Fun_Gd_Cb = fun_cb ;
    m_Param_Gd_Cb = param_cb ;
    
    for (int i = 0 ; i < m_iVtDetNum ; i ++) {
      m_VtDetLst[i]->SetGetDataCallback(fun_cb, param_cb) ;
    }
    return 0 ;
  }
  
};




