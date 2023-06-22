//
//  zvtdet.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zvtdet.h"
#include "../../aud/zaudapi.h"

namespace __r2vt4__ {

  ZVtDet::ZVtDet(ZVtWord* pVtWord, int iFrmSize, int iFrmOffset, int iCn){
    
    m_iFrmSize = iFrmSize ;
    m_iFrmOffset = iFrmOffset ;
    
    m_pVtWord = pVtWord ;
    
    m_iCandLen_Total = 400 ;
    
    m_pCandLst = Z_SAFE_NEW_AR2(m_pCandLst,ZVtCand, m_iCandLen_Total, m_pVtWord->m_iVtStateNum * m_pVtWord->m_iBlockMaxFrmLen);
    m_pCandScore_Best = Z_SAFE_NEW_AR1(m_pCandScore_Best, float, m_iCandLen_Total);
    m_pCandId_Best = Z_SAFE_NEW_AR1(m_pCandId_Best, int, m_iCandLen_Total);
    
    m_iVtDetStatus = vtdet_ready ;
    
    m_iFrmId_Cur = 0 ;
    m_iFrmId_Start = -1 ;
    m_iFrmId_Pre = -1 ;
    m_iFrmId_Best = -1 ;
    m_iFrmId_RightSil = -1 ;
    
    
    //Check ASR
    m_hCfTask = NULL ;
    if (m_pVtWord->m_bClassifyCheck) {
      m_hCfTask = r2_cf_create(m_pVtWord->m_strClassifyNnetPath.c_str(), iCn);
    }
    
    //Check LeftSil, RightSil
    m_pSilDet3 = Z_SAFE_NEW(m_pSilDet3, ZSilDet3,m_iFrmSize,30);
    m_pSilDet3->m_fLeftSilShield = m_pVtWord->m_fLeftSilShield ;
    m_pSilDet3->m_fRightSilShield = m_pVtWord->m_fRightSilShield ;
    
    
    m_pDefaultCand.pre = -2 ;
    m_pDefaultCand.score = -Z_FLT_MAX ;
    //m_pDefaultCand.framescore = -Z_FLT_MAX ;
    m_pDefaultCand.blockscore = -Z_FLT_MAX ;
    m_pDefaultCand.totalblockscore_avg = -Z_FLT_MAX ;
    
    m_fBestScore = -Z_FLT_MAX ;
    
    Reset();
    
    m_Fun_Ds_Cb = NULL ;
    m_Param_Ds_Cb = NULL ;
    
    m_Fun_Cd_Cb = NULL ;
    m_Param_Cd_Cb = NULL ;
    
    m_Fun_Gd_Cb = NULL ;
    m_Param_Gd_Cb = NULL  ;
    
    
#ifdef  _CB_AUDIO
    char cbpath[256];
    sprintf(cbpath,"%s/cb/",DEBUG_FILE_LOCATION);
    z_mkdir(cbpath);
#endif
    
  }
  
  ZVtDet::~ZVtDet(void){
    
    Z_SAFE_DEL_AR2(m_pCandLst);
    Z_SAFE_DEL_AR1(m_pCandScore_Best);
    Z_SAFE_DEL_AR1(m_pCandId_Best);
    
    Z_SAFE_DEL(m_pSilDet3);
    
    if (m_hCfTask != 0) {
      r2_cf_free(m_hCfTask) ;
    }
    
  }
  
  int ZVtDet::ProcessScore(int iFrmSize, ZMat* pFeaScore, ZAudBuff* pWavBuff, int iVtFlag){
    
    
    int rt = 0 ;
    
    bool bVadEnd = iVtFlag & R2_VT_FLAG_VAD_END ;
    bool bAec = iVtFlag & R2_VT_FLAG_AEC ;
    
    for (int i = 0 ; i < iFrmSize ; i ++) {
      float* pFrmScore = pFeaScore->data[i];
      
      if (m_iFrmId_Best == -1 && (m_iVtDetStatus == vtdet_process || m_iVtDetStatus == vtdet_ready)) {
        UpdateFrmScore(pFrmScore);
      }
      
      rt |= CheckStatus(iFrmSize - i, pFeaScore,pWavBuff, bAec);
      
      m_iFrmId_Cur ++ ;
    }
    
    if (bVadEnd && m_iVtDetStatus == vtdet_process) {
      
      if (m_iFrmId_Best == -1) {
        m_iFrmId_Best = m_iFrmId_Cur ;
        rt |= CheckStatus(0, pFeaScore,pWavBuff, bAec);
      }
      
      if (m_iFrmId_Cur < m_iFrmId_RightSil) {
        if (m_pVtWord->CheckNoCmdChange(bAec)) {
          rt |= R2_VT_WORD_DET_CMD ;
        }else{
          rt |= R2_VT_WORD_DET_NOCMD ;
        }
        m_iVtDetStatus = vtdet_finish ;
      }
    }
    
    return rt ;
  }
  
  
  int ZVtDet::ProcessMsg_Pre(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec){
    
    int rt = 0 ;
    
    if (m_iVtDetStatus == vtdet_process && m_iFrmId_Cur == m_iFrmId_Best) {
      
      //Second sl
      if (m_Fun_Ds_Cb != NULL) {
        
        int nFrmStart_Vt = m_iFrmId_Cur + iFrmOffset - m_iFrmId_Start + m_iFrmOffset ;
        int nFrmEnd_Vt = iFrmOffset ;
        
        int iStartFrm_Sil = iFrmOffset + m_iFrmOffset + (m_iFrmId_Cur -  m_iFrmId_Start)
        + m_pVtWord->m_iLeftSilSkip   + m_pVtWord->m_iLeftSilOffset ;
        int iEndFrm_Sil = iStartFrm_Sil - m_pVtWord->m_iLeftSilOffset ;
        
        float fSilShield = 0.0f ;
        m_Fun_Ds_Cb(iStartFrm_Sil, iEndFrm_Sil, nFrmStart_Vt, nFrmEnd_Vt, m_WordDetInfo.fWordSlInfo, &fSilShield, m_Param_Ds_Cb);
        m_pSilDet3->SetSilShield(fSilShield) ;
      }else{
        m_pSilDet3->SetSilShield(1000.0f) ;
      }
      
      
      //m_iFrmId_Best = m_iFrmId_Cur ;
      
      //energy
      if (m_pVtWord->m_bLeftSilCheck || m_pVtWord->m_bRightSilCheck) {
        m_iFrmId_Energy = m_iFrmId_Cur ;
      }
      
      //generate pre msg
      m_iFrmId_LeftSil = m_iFrmId_Cur ;
      
      //generate vt det msg
      m_iFrmId_CfCheck = m_iFrmId_Cur ;
      
    }
    
    return  rt ;
  }
  
  int ZVtDet::ProcessMsg_Best(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec){
    
    int rt = 0 ;
    
    if (m_iVtDetStatus == vtdet_process && m_iFrmId_Cur == m_iFrmId_Best) {
      
      
      //generate cmd and no cmd msg
      if (m_pVtWord->m_bRightSilCheck) {
        assert(m_pVtWord->m_iRightSilSkip + m_pVtWord->m_iRightSilOffset >= m_iFrmOffset) ;
        
        m_iFrmId_RightSil = m_iFrmId_Best + m_pVtWord->m_iRightSilSkip + m_pVtWord->m_iRightSilOffset - m_iFrmOffset ;
        if (m_iFrmId_RightSil < m_iFrmId_Best) {
          m_iFrmId_RightSil = m_iFrmId_Best ;
        }
      }else{
        m_iFrmId_RightSil = m_iFrmId_Best;
      }
      
    }
    
    return  rt ;
  }
  
  int ZVtDet::ProcessMsg_Energy(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec){
    
    int rt = 0 ;
    
    if (m_iVtDetStatus == vtdet_process && m_iFrmId_Cur == m_iFrmId_Energy) {
      
      assert(m_pVtWord->m_bLeftSilCheck || m_pVtWord->m_bRightSilCheck) ;
      
      //Get Engery
      int iStartFrm = iFrmOffset + m_iFrmOffset + (m_iFrmId_Cur -  m_iFrmId_Start)  ;
      int iEndFrm = iFrmOffset ;
      
      float* pData = Z_SAFE_NEW_AR1(pData, float, (iStartFrm - iEndFrm) * m_iFrmSize) ;
      
      if (m_Fun_Gd_Cb != NULL) {
        m_Fun_Gd_Cb(m_iFlag_Sl * iStartFrm, iEndFrm, pData, m_Param_Gd_Cb);
      }else{
        pWavBuff->GetLastAudio(pData, iStartFrm * m_iFrmSize, iEndFrm * m_iFrmSize, 0) ;
      }
//#ifdef  _CB_AUDIO
//      char cbpath[256];
//      sprintf(cbpath,"%s/cb/%s_en_cb.pcm",DEBUG_FILE_LOCATION,z_getdatatime().c_str());
//      z_storedata_vec(cbpath, m_pData_NoNew->data, (iStartFrm - iEndFrm) * m_iFrmSize);
//      
//      pWavBuff->GetBuff(m_pData_NoNew->data, iStartFrm * m_iFrmSize, iEndFrm * m_iFrmSize) ;
//      sprintf(cbpath,"%s/cb/%s_en_buff.pcm",DEBUG_FILE_LOCATION,z_getdatatime().c_str());
//      z_storedata_vec(cbpath, m_pData_NoNew->data, (iStartFrm - iEndFrm) * m_iFrmSize);
//#endif
      float fVtEnergy = m_pSilDet3->SetVtEnergy(pData, (iStartFrm - iEndFrm) * m_iFrmSize) ;
      m_WordDetInfo.fEnergy = fVtEnergy ;
      
      if (fVtEnergy < 300.0f) {
        ZLOG_INFO("---------------------FAILED CHECK VT ENERGY");
        m_iVtDetStatus = vtdet_finish ;
      }
      
      Z_SAFE_DEL_AR1(pData) ;
      
    }
    
    return  rt ;
  }
  
  int ZVtDet::ProcessMsg_LeftSil(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec){
    
    int rt = 0 ;
    
    if (m_iVtDetStatus == vtdet_process && m_iFrmId_Cur == m_iFrmId_LeftSil) {
      
      
      if (m_pVtWord->m_bLeftSilCheck) {
        assert(m_pVtWord->m_bLeftSilCheck) ;
        assert(m_iFrmId_LeftSil >= m_iFrmId_Energy) ;
        
        
        int iStartFrm = iFrmOffset + m_iFrmOffset + (m_iFrmId_Cur -  m_iFrmId_Start)
        + m_pVtWord->m_iLeftSilSkip   + m_pVtWord->m_iLeftSilOffset  ;
        int iEndFrm = iStartFrm - m_pVtWord->m_iLeftSilOffset ;
        
        float* pData = Z_SAFE_NEW_AR1(pData, float, (iStartFrm - iEndFrm) * m_iFrmSize) ;
        if (m_Fun_Gd_Cb != NULL) {
          m_Fun_Gd_Cb(m_iFlag_Sl * iStartFrm, iEndFrm, pData, m_Param_Gd_Cb);
        }else{
          pWavBuff->GetLastAudio(pData, iStartFrm * m_iFrmSize, iEndFrm * m_iFrmSize, 0) ;
        }
        
        if (m_pSilDet3->CheckLeftSilence(pData, (iStartFrm - iEndFrm) * m_iFrmSize)) {
          rt |= R2_VT_WORD_PRE ;
        }else{
          if (bAec) {
            rt |= R2_VT_WORD_PRE ;
            ZLOG_INFO("---------------------REMOVE CHECK LEFT SILENCE IN AEC CONDITION");
          }else{
            ZLOG_INFO("---------------------FAILED CHECK LEFT SILENCE");
            m_iVtDetStatus = vtdet_finish ;
          }
          
        }
        
        Z_SAFE_DEL_AR1(pData);
      }else{
        rt |= R2_VT_WORD_PRE ;
      }
    }
    
    return  rt ;
  }
  
  
  int ZVtDet::ProcessMsg_CfCheck(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec){
    
    int rt = 0 ;
    
    if (m_iVtDetStatus == vtdet_process && m_iFrmId_Cur == m_iFrmId_CfCheck) {
      
      bool bCfCheck = true ;
      
      if (m_pVtWord->m_bClassifyCheck) {
        //add CF Check
        int iStartFrm = iFrmOffset + m_iFrmOffset + (m_iFrmId_Cur -  m_iFrmId_Start) + 10  ;
        int iEndFrm = iFrmOffset + m_iFrmOffset - 20  ;
        iEndFrm = zmax(iEndFrm, 0) ;
        
        r2_aud* pAud_Cb = r2_aud_malloc(pWavBuff->m_iMicNum + 2, AUDIO_SAMPLE_RATE, (iStartFrm - iEndFrm) * m_iFrmSize );
        pWavBuff->GetLastAudio(pAud_Cb->data, iStartFrm * m_iFrmSize, iEndFrm * m_iFrmSize) ;
        if( m_Fun_Gd_Cb != NULL){
          m_Fun_Gd_Cb(iStartFrm, iEndFrm, pAud_Cb->data[pWavBuff->m_iMicNum], m_Param_Gd_Cb);
          //m_Fun_Gd_Cb(-iStartFrm, iEndFrm, pAud_Cb->data[pWavBuff->m_iMicNum + 1], m_Param_Gd_Cb);
        }
#ifdef  _CB_AUDIO
        char cbpath[256];
        sprintf(cbpath,"%s/cb/%s_cf_cb.wav",DEBUG_FILE_LOCATION,z_getdatatime().c_str());
        r2_aud_out(cbpath, pAud_Cb) ;
#endif
        ZLOG_INFO("--------------Start cf");
        float score = r2_cf_check_buff(m_hCfTask, (const float**)pAud_Cb->data, (iStartFrm - iEndFrm) * m_iFrmSize, pWavBuff->m_iMicNum, 0) ;
        //float score = -100.0f ;
        m_iFlag_Sl = 1 ;
        if ( m_Fun_Gd_Cb != NULL) {
          float score1 = r2_cf_check_buff(m_hCfTask, (const float**)(&pAud_Cb->data[pWavBuff->m_iMicNum]), (iStartFrm - iEndFrm) * m_iFrmSize, 1, 0) ;
          ZLOG_INFO("--------------%s", z_getslinfo(m_WordDetInfo.fWordSlInfo).c_str());
//          float score2 = r2_cf_check_buff(m_hCfTask, (const float**)(&pAud_Cb->data[pWavBuff->m_iMicNum + 1]), (iStartFrm - iEndFrm) * m_iFrmSize, 1, 0) ;
//          ZLOG_INFO("--------------%s", z_getslinfo(m_WordDetInfo.fWordSlInfo + 3).c_str());
//          if (score2 > score1) {
//            m_WordDetInfo.fWordSlInfo[0] = m_WordDetInfo.fWordSlInfo[3] ;
//            m_WordDetInfo.fWordSlInfo[1] = m_WordDetInfo.fWordSlInfo[4] ;
//            m_WordDetInfo.fWordSlInfo[2] = m_WordDetInfo.fWordSlInfo[5] ;
//            m_iFlag_Sl = -1 ;
//          }
//          score = zmax(score1, score2);
          score = zmax(score, score1) ;
        }
        
        ZLOG_INFO("--------------End cf");
        r2_aud_free(pAud_Cb) ;
        
        if (score > m_pVtWord->m_fClassifyNnetShield) {
          bCfCheck = true ;
        }else{
          bCfCheck = false ;
        }
      }
      
      //dirty check
      if (bCfCheck) {
        bool bCheckDirty = true ;
        if (m_Fun_Cd_Cb != NULL) {
          bCheckDirty = m_Fun_Cd_Cb(m_Param_Cd_Cb, m_WordDetInfo.fWordSlInfo);
        }
        if (bCheckDirty) {
          ZLOG_INFO("--------------Dirty Check Successfully");
          rt |= R2_VT_WORD_DET ;
        }else{
          ZLOG_INFO("--------------Dirty Check Failed");
          m_iVtDetStatus = vtdet_finish ;
        }
      }else{
        m_iVtDetStatus = vtdet_finish ;
      }

      
    }
    
    return  rt ;
  }
  
  
  int ZVtDet::ProcessMsg_RightSil(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec){
    
    int rt = 0 ;
    
    //RightSil
    if ((m_iVtDetStatus == vtdet_process) && m_iFrmId_Cur == m_iFrmId_RightSil) {
      
      
      if (m_pVtWord->m_bRightSilCheck) {
        int iStartFrm = m_pVtWord->m_iRightSilOffset + iFrmOffset ;
        int iEndFrm = iFrmOffset ;
        
        float* pData = Z_SAFE_NEW_AR1(pData, float, (iStartFrm - iEndFrm) * m_iFrmSize) ;
        
        if (m_Fun_Gd_Cb != NULL) {
          m_Fun_Gd_Cb(m_iFlag_Sl * iStartFrm, iEndFrm, pData, m_Param_Gd_Cb);
        }else{
          pWavBuff->GetLastAudio(pData, iStartFrm * m_iFrmSize, iEndFrm * m_iFrmSize, 0) ;
        }
        
        if (m_pSilDet3->CheckRightSilence(pData, (iStartFrm - iEndFrm) * m_iFrmSize) ) {
          if (m_pVtWord->CheckNoCmdChange(bAec)) {
            rt |= R2_VT_WORD_DET_CMD ;
          }else{
            rt |= R2_VT_WORD_DET_NOCMD ;
          }
        }else{
          rt |= R2_VT_WORD_DET_CMD ;
        }
        
        Z_SAFE_DEL_AR1(pData);
      }else{
        if (m_pVtWord->CheckNoCmdChange(bAec)) {
          rt |= R2_VT_WORD_DET_CMD ;
        }else{
          rt |= R2_VT_WORD_DET_NOCMD ;
        }
      }
      m_iVtDetStatus = vtdet_finish ;
    }
    
    return  rt ;
  }
  
  
  int ZVtDet::CheckStatus(int iFrmOffset, ZMat* pFeaScore, ZAudBuff* pWavBuff, bool bAec){
    
    int rt = 0 ;
    
    rt |= ProcessMsg_Pre(iFrmOffset, pFeaScore, pWavBuff, bAec) ;
    rt |= ProcessMsg_Best(iFrmOffset, pFeaScore, pWavBuff, bAec) ;
    
    rt |= ProcessMsg_CfCheck(iFrmOffset, pFeaScore, pWavBuff, bAec) ;
    
    rt |= ProcessMsg_Energy(iFrmOffset, pFeaScore, pWavBuff, bAec);
    rt |= ProcessMsg_LeftSil(iFrmOffset, pFeaScore, pWavBuff, bAec) ;
    
    rt |= ProcessMsg_RightSil(iFrmOffset, pFeaScore, pWavBuff, bAec) ;
    
    return rt ;
  }
  
  int ZVtDet::DynamicProgramming(float* pFrmScore){
    
    int iMaxScoreId = 0 ;
    for (int i = 0 ; i < 5191 ; i ++) {
      if (pFrmScore[i] > pFrmScore[iMaxScoreId]) {
        iMaxScoreId = i ;
      }
    }
    
    //ZLOG_INFO("FrmId: %d; BestState: %d;", m_iFrmId_Cur, iMaxScoreId);
    
    
    int iCandId_Cur = (m_iFrmId_Cur) % m_iCandLen_Total ;
    int iCandId_Last = (m_iFrmId_Cur + m_iCandLen_Total - 1 ) % m_iCandLen_Total ;
    
    //ZLOG_INFO("%d %d", m_iFrmId_Count, m_iCandId_Cur) ;
    
    ZVtCand* pCurCandLst = m_pCandLst[iCandId_Cur] ;
    ZVtCand* pLastCandLst = m_pCandLst[iCandId_Last] ;
    
    for (int i = 0 ; i < m_pVtWord->m_iVtStateNum ; i ++){
      ZVtState* pVtState = m_pVtWord->m_pVtStateLst[i];
      float fStateScore = pFrmScore[pVtState->pCurStateLst[0] - 1];
      for (int j = 1 ; j < pVtState->iCurStateNum ; j ++){
        if (fStateScore < pFrmScore[pVtState->pCurStateLst[j] - 1]){
          fStateScore = pFrmScore[pVtState->pCurStateLst[j] - 1] ;
        }
      }
      
      for (int j = 0 ; j < m_pVtWord->m_iBlockMaxFrmLen ; j ++){
        
        int iCandId =  i * m_pVtWord->m_iBlockMaxFrmLen + j ;
        
        pCurCandLst[iCandId].framescore = fStateScore ;
        
        if (j == 0) {
          if (pVtState->iStateFlag & PHO_FLAG_BLOCK_BEGIN) {
            if (pVtState->iStateFlag & PHO_FLAG_WORD_BEGIN) {
              //First Frame
              pCurCandLst[iCandId].pre = -1 ;
              pCurCandLst[iCandId].score = fStateScore ;
              pCurCandLst[iCandId].blockscore = fStateScore ;
              pCurCandLst[iCandId].totalblockscore_avg = 0 ;
            }else{
              //Middle Frame
              int preid = -1 ;
              float maxscore = - Z_FLT_MAX ;
              //fist pre is self so ignore
              for (int m = 1 ; m < pVtState->iPreStateNum  ; m ++) {
                int pos1 = pVtState->pPreStateLst[m] * m_pVtWord->m_iBlockMaxFrmLen;
                for (int n = m_pVtWord->m_iBlockMinFrmLen ; n < m_pVtWord->m_iBlockMaxFrmLen; n ++) {
                  int pos2 = pos1 + n;
                  assert((pos2 >= 0) && (pos2 < (m_pVtWord->m_iVtStateNum * m_pVtWord->m_iBlockMaxFrmLen)) );
                  if (pLastCandLst[pos2].pre == -2) {
                    continue ;
                  }
                  if (pLastCandLst[pos2].blockscore > m_pVtWord->m_fBlockMinScore) {
                    float score = pLastCandLst[pos2].blockscore + pLastCandLst[pos2].totalblockscore_avg ;
                    if (score > maxscore){
                      maxscore = score ;
                      preid = pos2 ;
                    }
                  }
                }
              }
              
              if (preid == -1){
                SetInValidCand(pCurCandLst[iCandId]);
              }else{
                pCurCandLst[iCandId].pre = preid ;
                pCurCandLst[iCandId].score = fStateScore ;
                //pCurCandLst[iCandId].framescore = fStateScore ;
                pCurCandLst[iCandId].blockscore = fStateScore ;
                pCurCandLst[iCandId].totalblockscore_avg = maxscore ;
              }
            }
            
          }else{
            SetInValidCand(pCurCandLst[iCandId]);
          }
          
        }else{
          int preid = -1 ;
          float maxscore = - Z_FLT_MAX ;
          
          if (pVtState->iStateFlag & PHO_FLAG_BLOCK_BEGIN) {
            int pos1 = pVtState->pPreStateLst[0] * m_pVtWord->m_iBlockMaxFrmLen + j - 1;
            assert((pos1 >= 0) && (pos1 < (m_pVtWord->m_iVtStateNum * m_pVtWord->m_iBlockMaxFrmLen)) );
            if (pLastCandLst[pos1].pre > -2) {
              preid = pos1 ;
            }
          }else{
            for (int m = 0 ; m < pVtState->iPreStateNum  ; m ++) {
              int pos1 = pVtState->pPreStateLst[m] * m_pVtWord->m_iBlockMaxFrmLen + j - 1;
              assert((pos1 >= 0) && (pos1 < (m_pVtWord->m_iVtStateNum * m_pVtWord->m_iBlockMaxFrmLen)) );
              if (pLastCandLst[pos1].pre == -2) {
                continue ;
              }
              float score = pLastCandLst[pos1].blockscore + pLastCandLst[pos1].totalblockscore_avg ;
              if (score > maxscore){
                maxscore = score ;
                preid = pos1 ;
              }
            }
          }
          
          if (preid == -1){
            SetInValidCand(pCurCandLst[iCandId]);
          }else{
            pCurCandLst[iCandId].pre = preid ;
            pCurCandLst[iCandId].score = pLastCandLst[preid].score + fStateScore ;
            //pCurCandLst[iCandId].framescore = fStateScore ;
            pCurCandLst[iCandId].blockscore = pCurCandLst[iCandId].score / (j+1) ;
            pCurCandLst[iCandId].totalblockscore_avg = pLastCandLst[preid].totalblockscore_avg ;
          }
          
        }
      }
    }
    
    //output
    m_pCandId_Best[iCandId_Cur] = -1 ;
    m_pCandScore_Best[iCandId_Cur] = -Z_FLT_MAX ;
    
    for (int i = 0 ; i < m_pVtWord->m_iVtStateNum ; i ++){
      ZVtState* pVtState = m_pVtWord->m_pVtStateLst[i];
      if (pVtState->iStateFlag & PHO_FLAG_WORD_END) {
        for (int j = m_pVtWord->m_iBlockMinFrmLen ; j < m_pVtWord->m_iBlockMaxFrmLen ; j ++){
          int iCandId =  i * m_pVtWord->m_iBlockMaxFrmLen + j ;
          if (pCurCandLst[iCandId].blockscore < m_pVtWord->m_fBlockMinScore ){
            continue;
          }
          float s1 = pCurCandLst[iCandId].blockscore + pCurCandLst[iCandId].totalblockscore_avg ;
          s1 = s1 / m_pVtWord->m_iBlockNum ;
          
          if (s1 > m_pCandScore_Best[iCandId_Cur]){
            m_pCandId_Best[iCandId_Cur] = iCandId ;
            m_pCandScore_Best[iCandId_Cur] = s1 ;
          }
        }
      }
    }
    
    return 0 ;
  }
  
  int ZVtDet::UpdateFrmScore(float* pFrmScore){
    
    int iCandId_Cur = (m_iFrmId_Cur) % m_iCandLen_Total ;
    int iCandId_Last = (m_iFrmId_Cur + m_iCandLen_Total - 1 ) % m_iCandLen_Total ;
    
    
    //DP
    DynamicProgramming(pFrmScore);
    
    //No AsrCheck
    if (m_iVtDetStatus == vtdet_ready
        && m_pCandScore_Best[iCandId_Cur] > m_pCandScore_Best[iCandId_Last]
        && m_pCandScore_Best[iCandId_Last] < m_pVtWord->m_fBlockAvgScore
        && m_pCandScore_Best[iCandId_Cur] > m_pVtWord->m_fBlockAvgScore) {
      
      ZLOG_INFO("%d to vtdet_process", m_pVtWord->m_iWordType);
      m_iVtDetStatus = vtdet_process ;
      
      m_iFrmId_Pre = m_iFrmId_Cur ;
      m_fBestScore_30 = -100.0f ;
      
      m_fBestScore_30 = m_pCandScore_Best[iCandId_Cur] ;
      m_iFrmId_Best_30 = m_iFrmId_Cur ;
      
      m_iFrmId_Best = m_iFrmId_Cur ;
      m_iFrmId_Start = m_iFrmId_Best_30 - GetBestLen(m_iFrmId_Best_30) ;
      
    }
    
//    if(m_iVtDetStatus == vtdet_process && m_iFrmId_Best == -1 ){
//      if (m_iFrmId_Cur < m_iFrmId_Pre + 10) {
//        if (m_pCandScore_Best[iCandId_Cur] > m_fBestScore_30) {
//          m_fBestScore_30 = m_pCandScore_Best[iCandId_Cur] ;
//          m_iFrmId_Best_30 = m_iFrmId_Cur ;
//        }
//      }else{
//        if (m_pCandScore_Best[iCandId_Cur] > m_fBestScore_30) {
//          m_fBestScore_30 = m_pCandScore_Best[iCandId_Cur] ;
//          m_iFrmId_Best_30 = m_iFrmId_Cur ;
//        }else{
//          m_iFrmId_Best = m_iFrmId_Cur ;
//          m_iFrmId_Start = m_iFrmId_Best_30 - GetBestLen(m_iFrmId_Best_30) ;
//          ZLOG_INFO("-------------%d BestScore: %f", m_pVtWord->m_iWordType, m_fBestScore_30);
//        }
//        
//      }
//      
//    }
    
    return 0 ;
  }
  
  int ZVtDet::GetBlockScore(float* pScore, int iMaxScoreLen){
    
    assert(0);
    
    
    return  0 ;
  }
  
  int ZVtDet::GetBestLen(int iFrmId){
    
    int iCandId = iFrmId % m_iCandLen_Total ;
    
    int rt = 0 ;
    int pp = m_pCandId_Best[iCandId] ;
    for (int k = iCandId ; pp != -1 && rt < m_iCandLen_Total - (m_iFrmId_Cur - iFrmId) ; k --) {
      if (k < 0) {
        k += m_iCandLen_Total ;
      }
      int iState = pp / m_pVtWord->m_iBlockMaxFrmLen ;
      int iStateCount = pp % m_pVtWord->m_iBlockMaxFrmLen ;
      //ZLOG_INFO("--------------FrmId: %d; FrmScore: %f; BlockScore: %f; AvgScore: %f; tp: %d; state: %d; count: %d;", iFrmId - rt, m_pCandLst[k][pp].framescore, m_pCandLst[k][pp].blockscore, m_pCandLst[k][pp].totalblockscore_avg, iState / 3 , iState % 3, iStateCount);
      
      pp = m_pCandLst[k][pp].pre ;
      assert(pp != -2) ;
      rt ++ ;
    }
    ZLOG_INFO("--------------StartFrmId: %d CurFrmId: %d BestFrmLen: %d  BestCandScore: %f",iFrmId - rt , iFrmId, rt, m_pCandScore_Best[iCandId]);
    return  rt ;
  }
  
  const WordDetInfo* ZVtDet::GetDetWordInfo(int iFrmOffset){
    
    m_WordDetInfo.iWordPos_Start = (m_iFrmId_Cur - m_iFrmId_Start + iFrmOffset) * m_iFrmSize ;
    m_WordDetInfo.iWordPos_End = (m_iFrmId_Cur - m_iFrmId_Best_30 + iFrmOffset) * m_iFrmSize ;
    
    return &m_WordDetInfo ;
  }
  
  int ZVtDet::Reset(){
    
    m_iVtDetStatus = vtdet_ready ;
    
    m_iFrmId_Cur = 0 ;
    m_iFrmId_Start = -1 ;
    m_iFrmId_Pre = -1 ;
    m_iFrmId_Best = -1 ;
    m_iFrmId_Energy = -1 ;
    m_iFrmId_LeftSil = -1 ;
    m_iFrmId_CfCheck = -1 ;
    m_iFrmId_RightSil = -1 ;
    
    int iCandId_Last = (m_iFrmId_Cur + m_iCandLen_Total - 1 ) % m_iCandLen_Total ;
    for (int j = 0 ; j < m_pVtWord->m_iVtStateNum * m_pVtWord->m_iBlockMaxFrmLen ; j ++){
      SetInValidCand(m_pCandLst[iCandId_Last][j]);
    }
    memset(m_pCandScore_Best, 0, m_iCandLen_Total * sizeof(float));
    memset(m_pCandId_Best, 0, m_iCandLen_Total * sizeof(int));
    
    m_iFlag_Sl = 1 ;
    
    m_fBestScore = -FLT_MAX ;
    
    memset(&m_WordDetInfo, 0, sizeof(WordDetInfo));
    
    return  0 ;
  }
  
  
  int ZVtDet::SetInValidCand(ZVtCand &pCand){
    
    memcpy(&pCand, &m_pDefaultCand, sizeof(ZVtCand));
    
    return  0 ;
  }
  
  bool ZVtDet::IsNeedReset(){
    
    if (m_iVtDetStatus == vtdet_finish) {
      return  true ;
    }else{
      return false ;
    }
  }
  
  bool ZVtDet::IsNeedDnn(){
    
    if (m_iFrmId_Best != -1) {
      return  false ;
    }else{
      return  true ;
    }
  }
  
  float ZVtDet::GetBestScore(int iFrmSize, ZMat* pFeaScore, ZAudBuff* m_pWavBuff){
    
    for (int i = 0 ; i < iFrmSize ; i ++) {
      
      int iCandId_Cur = (m_iFrmId_Cur) % m_iCandLen_Total ;

      float* pFrmScore = pFeaScore->data[i];
      DynamicProgramming(pFrmScore);
      
      if (m_fBestScore < m_pCandScore_Best[iCandId_Cur]) {
        m_fBestScore = m_pCandScore_Best[iCandId_Cur] ;
        m_iFrmId_Start = m_iFrmId_Cur - GetBestLen(m_iFrmId_Cur) ;
        m_iFrmId_Best = m_iFrmId_Cur ;
      }
      
      m_iFrmId_Cur ++ ;
      
    }
    return  m_fBestScore ;
  }
  
  int ZVtDet::SetDoSecSlCallback(dosecsl_callback fun_cb, void* param_cb){
    
    m_Fun_Ds_Cb = fun_cb ;
    m_Param_Ds_Cb = param_cb ;
    
    return  0 ;
  }
  
  int ZVtDet::SetCheckDirtyCallback(checkdirty_callback fun_cb, void* param_cb){
    
    m_Fun_Cd_Cb = fun_cb ;
    m_Param_Cd_Cb = param_cb ;
    
    return  0 ;
  }
  
  int ZVtDet::SetGetDataCallback(getdata_callback fun_cb, void* param_cb){
    
    m_Fun_Gd_Cb = fun_cb ;
    m_Param_Gd_Cb = param_cb ;
    
    return  0 ;
  }
  
};




