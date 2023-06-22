//
//  zfea2.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zfea2.h"

namespace __r2vt4__ {
  
  ZFea2::ZFea2(int iSr, int iBat, int iCn)
  {
    //	FB Info
    m_iBat = 0 ;
    m_iSr = 0 ;
    m_iCn = 0 ;
    
    m_iFFT = 0 ;
    m_iWin = 0 ;
    m_iSft = 0 ;
    
    m_iFbDim = 0 ;
    m_MelPos_L = NULL ;
    m_MelPos_H = NULL ;
    m_MelWt_L = NULL ;
    m_MelWt_H = NULL ;
    
    //	FB Ext
    m_iBatPos = 0 ;
    m_iBatWin = 0 ;
    m_fPreEm = 0.0f ;
    m_pPreEmBuff = 0 ;
    
    m_pHam = 0 ;
    m_pHamBuff = 0 ;
    m_pPsd = 0 ;
    
    m_fft = NULL ;
    m_iFbNum = 0 ;
    m_iFbTotal = 0 ;
    m_pFb = 0 ;
    
    InitFb(iSr,iBat,iCn);
  }
  
  ZFea2::~ZFea2(void)
  {
    Z_SAFE_DEL(m_MelWt_L);
    Z_SAFE_DEL(m_MelWt_H);
    Z_SAFE_DEL(m_MelPos_L);
    Z_SAFE_DEL(m_MelPos_H);
    
    
    Z_SAFE_DEL(m_pPreEmBuff);
    Z_SAFE_DEL(m_pHam);
    Z_SAFE_DEL(m_pHamBuff);
    Z_SAFE_DEL(m_pPsd);
    
    Z_SAFE_DEL(m_fft);
    Z_SAFE_DEL(m_pFb);
    
  }
  
  
  int ZFea2::InitFb(int iSr, int iBat, int iCn){
    
    m_iSr = iSr ;
    m_iBat = iBat ;
    m_iCn = iCn ;
    
    if (m_iSr == 8000){
      m_iWin = 200 ;
      m_iSft = 80 ;
      m_iFbDim = CHS_NUM + CHS_NUM_W ;
      m_iFFT = 256 ;
      
      //Low
      m_MelPos_L = Z_SAFE_NEW(m_MelPos_L, ZVec,CHS_NUM + 2);
      m_MelWt_L = Z_SAFE_NEW(m_MelWt_L, ZVec,m_iFFT/2 + 1);
      InitMelPos(125,3800,m_MelPos_L,m_MelWt_L);
      
    }else if (m_iSr == 16000){
      
      m_iWin = 400 ;
      m_iSft = 160 ;
      m_iFbDim = CHS_NUM + CHS_NUM_W ;
      m_iFFT = 512 ;
      
      //Low
      m_MelPos_L = Z_SAFE_NEW(m_MelPos_L, ZVec,CHS_NUM + 2);
      m_MelWt_L = Z_SAFE_NEW(m_MelWt_L, ZVec,m_iFFT/2 + 1);
      InitMelPos(125,3800,m_MelPos_L,m_MelWt_L) ;
      
      //High
      m_MelPos_H = Z_SAFE_NEW(m_MelPos_H, ZVec,CHS_NUM_W + 2);
      m_MelWt_H = Z_SAFE_NEW(m_MelWt_H, ZVec,m_iFFT/2 + 1);
      InitMelPos2(4000,7800,m_MelPos_H,m_MelWt_H);
      
    }else{
      return 1 ;
    }
    
    //Pre Em
    m_iBatWin = m_iWin + (m_iBat - 1) * m_iSft ;
    m_iBatPos = 0 ;
    
    m_fPreEm = 0.97f ;
    m_pPreEmBuff = Z_SAFE_NEW(m_pPreEmBuff,ZMat, m_iCn, m_iBatWin) ;
    
    //Hamming Windows
    float fFrmWin = 0.54f ;
    m_pHam = Z_SAFE_NEW(m_pHam,ZVec,m_iFFT) ;
    for (int i = 0 ; i < m_iWin ; i ++){
      m_pHam->data[i] = (float)(fFrmWin - (1-fFrmWin)*cos(2 * i * Z_PI / (m_iWin-1)));
    }
    m_pHamBuff = Z_SAFE_NEW(m_pHamBuff,ZMat,m_iBat*m_iCn,m_iFFT) ;
    
    //FFT PSD
    m_pPsd = Z_SAFE_NEW(m_pPsd,ZMat,m_iBat * m_iCn ,m_iFFT / 2 + 1);
    m_fft = Z_SAFE_NEW(m_fft,ZFFT,m_iFFT,m_iBat * m_iCn );
    
    //Fb Buff
    m_iFbTotal = 100 ; //init 1000 frame
    m_iFbNum = 0 ;
    m_pFb = Z_SAFE_NEW(m_pFb,ZMat,m_iFbTotal,m_iFbDim);
    
    return 0 ;
  }
  
  int	ZFea2::InitMelPos(float fLoHz,float fHiHz, ZVec* pMelPos, ZVec* pMelWt){
    
    float fLoHz_M = (float)mel(fLoHz) ;
    float fHiHz_M = (float)mel(fHiHz) ;
    
    float * FiltFreq = Z_SAFE_NEW_AR1(FiltFreq,float, pMelPos->size) ;
    float fSpan_M = (fHiHz_M - fLoHz_M)/(pMelPos->size - 1);
    for (int i = 0 ; i < pMelPos->size ; i ++){
      FiltFreq[i] = fLoHz_M + i * fSpan_M;
    }
    //for (int i = 0 ; i < pMelPos->size; i ++) {
    //    printf("%f ",mel_inv(FiltFreq[i]));
    //}
    //printf("\n");
    fLoHz = FiltFreq[0];
    fHiHz = FiltFreq[pMelPos->size - 1];
    for(int i = 0; i <= m_iFFT/2; i++){
      float Freq = mel((m_iSr)/(float)(m_iFFT)*i);
      if (Freq <= fLoHz || Freq >= fHiHz ) {
        continue;
      } else {
        int j = 0 ;
        for(j = 0; j < pMelPos->size - 1 ; j++) {
          if(Freq >= FiltFreq[j] && Freq <= FiltFreq[j+1])
            break;
        }
        if (pMelPos->data_i[j] == 0){
          pMelPos->data_i[j] = i ;
        }
        pMelPos->data_i[j+1] = i + 1 ;
        pMelWt->data[i] = (FiltFreq[j+1] - Freq)/(FiltFreq[j+1]-FiltFreq[j]);
      }
    }
    Z_SAFE_DEL_AR1(FiltFreq);
    return 0 ;
  }
  
  
  int	ZFea2::InitMelPos2(float fLoHz,float fHiHz, ZVec* pMelPos, ZVec* pMelWt){
    
    float fLoHz_M = (float)mel(fLoHz) ;
    float fHiHz_M = (float)mel(fHiHz) ;
    
    float * FiltFreq = Z_SAFE_NEW_AR1(FiltFreq,float, pMelPos->size) ;
    float fSpan_M = (fHiHz_M - fLoHz_M)/(pMelPos->size - 2);
    for (int i = 0 ; i < pMelPos->size ; i ++){
      FiltFreq[i] = fLoHz_M + (i-1) * fSpan_M;
    }
    fLoHz = FiltFreq[0];
    fHiHz = FiltFreq[pMelPos->size - 1];
    for(int i = 0; i <= m_iFFT/2; i++){
      float Freq =(float)mel((m_iSr)/(float)(m_iFFT)*i);
      if (Freq <= fLoHz || Freq >= fHiHz ) {
        continue;
      } else {
        int j = 0 ;
        for(j = 0; j < pMelPos->size - 1 ; j++) {
          if(Freq >= FiltFreq[j] && Freq <= FiltFreq[j+1])
            break;
        }
        if (pMelPos->data_i[j] == 0){
          pMelPos->data_i[j] = i ;
        }
        pMelPos->data_i[j+1] = i + 1 ;
        pMelWt->data[i] = (FiltFreq[j+1] - Freq)/(FiltFreq[j+1]-FiltFreq[j]);
      }
    }
    Z_SAFE_DEL_AR1(FiltFreq);
    return 0 ;
  }
  
  int ZFea2::ResetFb(){
    
    m_iBatPos = 0 ;
    
    return 0 ;
  }
  
  int ZFea2::ContinueFb(const float** pAud, int len,ZMat* &pFb, int &iFbNum){
    
    m_iFbNum = (len + m_iBatPos) / m_iSft ;
    if (m_iFbNum > m_iFbTotal){
      m_iFbTotal = m_iFbNum * 2 ;
      Z_SAFE_DEL(m_pFb);
      m_pFb = Z_SAFE_NEW(m_pFb,ZMat,m_iFbTotal,m_iFbDim);
    }
    
    m_iFbNum = 0 ;
    int cur = 0 ;
    while(cur < len){
      int ll = zmin(len - cur, m_iBatWin-m_iBatPos);
      for (int i = 0 ; i < ll ; i ++){
        for (int j = 0 ; j < m_iCn ; j ++) {
          m_pPreEmBuff->data[j][m_iBatPos+i] = pAud[j][cur + i] ;
        }
      }
      m_iBatPos += ll ;
      cur += ll ;
      
      //Fe
      if (m_iBatPos == m_iBatWin){
        ProBlockBuff();
        //Shift
        for (int j = 0 ; j < m_iCn ; j ++) {
          memcpy(m_pPreEmBuff->data[j], m_pPreEmBuff->data[j] + m_iSft*m_iBat, sizeof(float)*(m_iWin - m_iSft));
        }
        
        m_iBatPos = m_iWin - m_iSft;
        m_iFbNum += m_iBat ;
      }
    }
    
    iFbNum = m_iFbNum ;
    pFb = m_pFb ;
    
    return 0 ;
  }
  
  int ZFea2::ProBlockBuff(){
    
    //Process
    m_pHamBuff->Clean();
    
    //Cut
    for (int i = 0 ; i < m_iCn ; i ++) {
      for (int j = 0 ; j < m_iBat ; j ++){
        memcpy(m_pHamBuff->data[i * m_iBat + j],m_pPreEmBuff->data[i] + m_iSft * j , sizeof(float)*m_iWin);
      }
    }

    
    //PreEm
    for (int i = 0 ; i < m_pHamBuff->row ; i ++){
      
//      for (int j = 0 ; j < m_iWin ; j ++) {
//        m_pHamBuff->data[i][j] +=  z_randgauss();
//      }
      
      float sum = 0.0f ;
      for (int j = 0 ; j < m_iWin ; j ++){
        sum += m_pHamBuff->data[i][j];
      }
      sum = sum / m_iWin ;
      for (int j = 0 ; j < m_iWin ; j ++){
        m_pHamBuff->data[i][j] -= sum;
      }
      for (int j = m_iWin - 1 ; j > 0 ; j --){
        m_pHamBuff->data[i][j] = m_pHamBuff->data[i][j] - m_fPreEm * m_pHamBuff->data[i][j-1] ;
      }
      m_pHamBuff->data[i][0] = m_pHamBuff->data[i][0] - m_fPreEm *  m_pHamBuff->data[i][0] ;
    }
    
    //Hamming
    m_pHamBuff->RowMul(m_pHam);
    
    //FFT
    //m_pHamBuff    ===>    m_pPsd
    m_fft->Execute(m_pHamBuff,m_pPsd);
    
    //Avg Psd
    if (m_iCn > 1) {
    for (int i = 0 ; i < m_iBat ; i ++) {
      for (int j = 0 ; j < m_iFFT / 2 + 1 ; j ++) {
        for (int k = 1 ; k < m_iCn ; k ++) {
          m_pPsd->data[i][j] += m_pPsd->data[i + m_iBat * k][j] ;
        }
        m_pPsd->data[i][j] = m_pPsd->data[i][j] / m_iCn ;
      }
    }
    }
        
    //Fb
    for (int j = 0 ; j < m_iBat ; j ++){
      float* pPsd = m_pPsd->data[j];
      float* pFb = m_pFb->data[m_iFbNum + j];
      memset(pFb,0,sizeof(float)*m_iFbDim);
      
      //fb
      GenFbData(pPsd,m_MelPos_L,m_MelWt_L,pFb);
      if (m_MelPos_H != NULL){
        GenFbData(pPsd,m_MelPos_H,m_MelWt_H,pFb+CHS_NUM);
      }
      for (int m = 0 ; m < m_iFbDim ; m ++){
        if(pFb[m] < 0.0000001f){
          pFb[m] = 0.0000001f ;
        }
        pFb[m] = log(pFb[m]);
      }
    }
    
    return 0 ;
  }
  
  int ZFea2::GenFbData(float* pPsd, ZVec* pMelPos, ZVec* pMelWt, float* pFb){
    
    for (int m = 0 ; m < pMelPos->size - 1 ; m ++){
      float sc1 = 0.0f , sc2 = 0.0f ;
      for (int n = pMelPos->data_i[m] ; n < pMelPos->data_i[m + 1] ; n ++){
        sc1 += pPsd[n] * pMelWt->data[n] ;
        sc2 += pPsd[n] ;
      }
      sc2 = sc2 - sc1 ;
      if (m < pMelPos->size - 2){
        pFb[m] = sc2 ;
      }
      if (m > 0){
        pFb[m-1] += sc1 ;
      }
    }
    return 0 ;
  }
  
}




