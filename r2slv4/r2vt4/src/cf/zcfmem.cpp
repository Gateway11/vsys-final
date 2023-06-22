//
//  ZCfMem2.cpp
//  r2vt4
//
//  Created by hadoop on 6/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zcfmem.h"

#include "../aud/zaudapi.h"

namespace __r2vt4__ {
  
  static float g_mean[40] = {13.7421436f,15.0453911f,15.9204512f,16.7998886f,17.4285927f,17.6744175f,17.6159649f,17.5254097f,17.1446419f,16.7877274f,16.2332592f,15.9262819f,15.7469435f,15.6904697f,15.7652569f,16.2019920f,16.6780529f,17.0173016f,17.2286701f,17.3532562f,17.5416565f,18.0207539f,18.6952934f,19.0303555f,18.9684696f,18.5608673f,18.2106113f,18.5905991f,19.0756550f,18.8323231f,18.0359821f,17.6346264f,17.6067657f,17.5186272f,17.6005516f,17.6876831f,17.5054874f,17.0688610f,16.3985405f,15.8875160f} ;
  //static float g_var[40] = {2.6323245f,3.1204875f,3.3416245f,3.3639035f,3.4560015f,3.5302765f,3.6267998f,3.6921041f,3.6706676f,3.5767031f,3.4342997f,3.2967703f,3.2243714f,3.2207637f,3.2937849f,3.3763046f,3.3767383f,3.3506882f,3.3206606f,3.2790186f,3.2508709f,3.3622520f,3.4468288f,3.5271540f,3.6789906f,3.7489891f,3.6960216f,3.7239645f,3.7879384f,3.8190377f,3.7546463f,3.5983365f,3.5174086f,3.4575558f,3.5327787f,3.5994041f,3.5876117f,3.5171609f,3.4288664f,3.4826026f};
  static float g_var[40] = {2.6567121,3.1415098,3.3688388,3.3936794,3.4752316,3.5418837,3.6329956,3.6949136,3.6745887,3.5870757,3.4560022,3.3280771,3.2609520,3.2561924,3.3213246,3.3991964,3.4024787,3.3808091,3.3519733,3.3074808,3.2727475,3.3743880,3.4550984,3.5287075,3.6704750,3.7351453,3.6819551,3.7099051,3.7730913,3.7999306,3.7271991,3.5673130,3.4886131,3.4285052,3.4996629,3.5641139,3.5538430,3.4915180,3.4044476,3.4543362};
  
  ZCfMem2::ZCfMem2(const char* pNnetPath, int iCn){
    
    m_pFea1 = Z_SAFE_NEW(m_pFea1, ZFea1, 16000, iCn, 40);
    m_pFea2 = Z_SAFE_NEW(m_pFea2, ZFea1, 16000, 1, 40);
    
    m_pNnet = Z_SAFE_NEW(m_pNnet, Nnet, pNnetPath, false);
    
    m_pNnetMem = Z_SAFE_NEW(m_pNnetMem, Nnet_Memory_Total, m_pNnet) ;
    
    m_pVec_Mean_g = Z_SAFE_NEW(m_pVec_Mean_g, ZVec, 40) ;
    m_pVec_Var_g = Z_SAFE_NEW(m_pVec_Var_g, ZVec, 40) ;
    
    for (int i = 0 ; i < 40 ; i ++) {
      m_pVec_Mean_g->data[i] = g_mean[i] ;
      m_pVec_Var_g->data[i] = g_var[i] ;
    }
    
    m_pVec_Mean_u = Z_SAFE_NEW(m_pVec_Mean_u, ZVec, 40) ;
    m_pVec_Var_u = Z_SAFE_NEW(m_pVec_Var_u, ZVec, 40) ;
    
  }
  
  ZCfMem2::~ZCfMem2(void){
    
    Z_SAFE_DEL(m_pNnetMem);
    Z_SAFE_DEL(m_pNnet) ;
    
    Z_SAFE_DEL(m_pFea1) ;
    Z_SAFE_DEL(m_pFea2) ;
    
    Z_SAFE_DEL(m_pVec_Mean_g) ;
    Z_SAFE_DEL(m_pVec_Var_g) ;
    
    Z_SAFE_DEL(m_pVec_Mean_u) ;
    Z_SAFE_DEL(m_pVec_Var_u) ;
    
  }
  
  
  float ZCfMem2::check_file(const char* pWavPath){
    
    r2_aud* pAud = r2_aud_in(pWavPath, 16000);
    
    assert(pAud->cn == m_pFea1->m_iCn) ;
    
    float score = check_buff((const float**)pAud->data, pAud->len, pAud->cn);
    
    r2_aud_free(pAud);
    
    return  score ;
    
    
  }
  
  int ZCfMem2::get_score(const float** pWavData, int iWavLen, int iCn, float* pScore, int iScoreLen){
    
    assert(iScoreLen >= m_pNnet->m_iOutputDim) ;
    
    ZMat* pFb = NULL ;
    if (iCn == 1) {
      m_pFea2->ExtractFBank(pWavData, iWavLen, pFb) ;
    }else{
      m_pFea1->ExtractFBank(pWavData, iWavLen, pFb) ;
    }
    
    //pFb->Print("pFb");
    if (pFb->row < 20) {
      return  -100.0f ;
    }
    DoCmvn(pFb);
    
    
    ZMat* pFbScore = NULL ;
    int iFbSize = 0  ;
    
    //pFb->Print("pFb_cmvn");
    m_pNnetMem->GetOutPut(pFb, pFb->row, pFbScore, iFbSize) ;
    
    //pFbScore->Print("pFbScore");
    
    memset(pScore, 0, sizeof(float) * iScoreLen) ;
    
    for (int i = 0 ; i < iFbSize ; i ++) {
      for (int j = 0 ; j < pFbScore->col ; j ++) {
        pScore[j] += pFbScore->data[i][j] ;
      }
    }
    
    return  0 ;
    
  }
  
  float ZCfMem2::check_buff(const float** pWavData, int iWavLen, int iCn){
    
    ZMat* pFb = NULL ;
    if (iCn == 1) {
      m_pFea2->ExtractFBank(pWavData, iWavLen, pFb) ;
    }else{
      m_pFea1->ExtractFBank(pWavData, iWavLen, pFb) ;
    }
    
    
    //pFb->Print("pFb");
    if (pFb->row < 20) {
      return  -100.0f ;
    }
    DoCmvn(pFb);
    
    ZMat* pFbScore = NULL ;
    int iFbSize = 0  ;
    
    //pFb->Print("pFb_cmvn");
    m_pNnetMem->GetOutPut(pFb, pFb->row, pFbScore, iFbSize) ;
    
    //pFbScore->Print("pFbScore");
    float score0 = 0.0f , score1 = 0.0f , sum = 0.0f ;
    for (int i = 0 ; i < pFbScore->row ; i ++) {
      score0 += pFbScore->data[i][0] ;
      score1 += pFbScore->data[i][1] ;
    }
    score0 = score0 / pFbScore->row ;
    score1 = score1 / pFbScore->row ;
    
    sum = z_log_add(score0, score1);
    score0 -= sum ;
    score1 -= sum ;
    ZLOG_INFO("--------------cf score: %f %f", score0, score1);
    
    return  score1 ;
    
  }
  
  int ZCfMem2::DoCmvn(ZMat* pFb){
    
    if (m_pNnet->components_[0]->_component_name == LayerName_BLSTM_KALDI) {
      
      m_pVec_Mean_u->Clean() ;
      m_pVec_Var_u->Clean() ;
      
      for (int i = 0 ; i < pFb->row; i ++) {
        for (int j = 0 ; j < pFb->col ; j ++) {
          m_pVec_Mean_u->data[j] += pFb->data[i][j];
          m_pVec_Var_u->data[j] += pFb->data[i][j] * pFb->data[i][j] ;
        }
      }
      
      for (int j = 0 ; j < pFb->col ; j ++) {
        m_pVec_Mean_u->data[j] = - m_pVec_Mean_u->data[j]/ pFb->row ;
        m_pVec_Var_u->data[j] = m_pVec_Var_u->data[j] / pFb->row ;
        m_pVec_Var_u->data[j] = (m_pVec_Var_u->data[j] - m_pVec_Mean_u->data[j] * m_pVec_Mean_u->data[j])  ;
        m_pVec_Var_u->data[j] = 1.0f / sqrtf(m_pVec_Var_u->data[j] + 1.0e-5) ;
      }
    }else{
      m_pVec_Mean_u->Clean() ;
      m_pVec_Var_u->Clean() ;
      
      for (int i = 0 ; i < pFb->row; i ++) {
        for (int j = 0 ; j < pFb->col ; j ++) {
          m_pVec_Mean_u->data[j] += pFb->data[i][j];
          m_pVec_Var_u->data[j] += pFb->data[i][j] * pFb->data[i][j] ;
        }
      }
      
      for (int j = 0 ; j < pFb->col ; j ++) {
        m_pVec_Mean_u->data[j] = - m_pVec_Mean_u->data[j]/ pFb->row ;
        m_pVec_Var_u->data[j] = m_pVec_Var_u->data[j] / pFb->row ;
        m_pVec_Var_u->data[j] = (m_pVec_Var_u->data[j] - m_pVec_Mean_u->data[j] * m_pVec_Mean_u->data[j])  ;
        m_pVec_Var_u->data[j] = (m_pVec_Var_u->data[j] + m_pVec_Var_g->data[j]) / 2 ;
        //m_pVec_Var_u->data[j] = m_pVec_Var_g->data[j] ;
        m_pVec_Var_u->data[j] = 1.0f / sqrtf(m_pVec_Var_u->data[j]+ 1.0e-5) ;
      }
    }
    
    pFb->RowAdd(m_pVec_Mean_u) ;
    pFb->RowMul(m_pVec_Var_u) ;
    
    return  0 ;
  }
  
};




