//
//  zfea1.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zfea1.h"

namespace __r2vt4__ {
  
  ZFea1::ZFea1(int iSr, int iCn, int iFbDim){
    
    assert(iSr == 16000) ;
    
    m_iSr = iSr ;
    m_iCn = iCn ;
    
    m_iWin = 400 ;
    m_iSft = 160 ;
    m_iFbDim = iFbDim ;
    m_iFFT = 512 ;
    
    //Low
    m_MelPos = Z_SAFE_NEW(m_MelPos, ZVec,iFbDim + 2);
    m_MelWt = Z_SAFE_NEW(m_MelWt, ZVec,m_iFFT/2 + 1);
    
    InitMelPos(20,8000,m_MelPos,m_MelWt) ;
    
    //Pre Em
    m_fPreEm = 0.97f ;
    
    //Hamming Windows
    float fFrmWin = 0.54f ;
    m_pHam = Z_SAFE_NEW(m_pHam,ZVec,m_iWin) ;
    for (int i = 0 ; i < m_iWin ; i ++){
      m_pHam->data[i] = (float)(fFrmWin - (1-fFrmWin)*cos(2 * i * Z_PI / (m_iWin-1)));
    }
    m_pHamBuff =  Z_SAFE_NEW(m_pHamBuff,ZMat,m_iCn,m_iWin) ;
    
    //FFT PSD
    m_pPsd = Z_SAFE_NEW(m_pPsd,ZVec,m_iFFT / 2 + 1);
    m_ffts = Z_SAFE_NEW(m_ffts,ZFFT,m_iFFT, m_iCn);
    
    //Fb Buff
    m_iFbTotal = 100 ; //init 100 frame
    m_pFb = Z_SAFE_NEW(m_pFb,ZMat,m_iFbTotal,m_iFbDim);
  }
  
  ZFea1::~ZFea1(void){
    
    Z_SAFE_DEL(m_MelPos);
    Z_SAFE_DEL(m_MelWt);
    
    Z_SAFE_DEL(m_pHam);
    Z_SAFE_DEL(m_pHamBuff);
    Z_SAFE_DEL(m_pPsd);
    
    Z_SAFE_DEL(m_ffts);
    Z_SAFE_DEL(m_pFb);
    
    Z_SAFE_DEL(m_pFb);
  }
  
  int ZFea1::ExtractFBank(const float** pAud, int len, ZMat* &pFb){
    
    //Alloc Output Memory
    int iFbNum = (len - m_iWin) / m_iSft + 1 ;
    if (m_iFbTotal < iFbNum) {
      Z_SAFE_DEL(m_pFb);
      m_iFbTotal = iFbNum * 2 ;
      m_pFb = Z_SAFE_NEW(m_pFb,ZMat,m_iFbTotal,m_iFbDim);
    }
    
    m_pFb->row = iFbNum ;
    pFb = m_pFb ;
    
    //ExtractFBank
    for (int i = 0 ; i < iFbNum ; i ++) {
      for (int j = 0 ; j < m_iCn ; j ++) {
        memcpy(m_pHamBuff->data[j], pAud[j] + m_iSft * i , sizeof(float) * m_iWin) ;
      }
      
      
      //dither
      //      for (int j = 0 ; j < m_iWin ; j ++) {
      //        m_pHamBuff->data[j] +=  z_randgauss();
      //      }
      
      for (int k = 0 ; k < m_iCn ; k ++) {
        float sum = 0.0f ;
        for (int j = 0 ; j < m_iWin ; j ++){
          sum += m_pHamBuff->data[k][j];
        }
        sum = sum / m_iWin ;
        for (int j = 0 ; j < m_iWin ; j ++){
          m_pHamBuff->data[k][j] -= sum;
        }
        
        for (int j = m_iWin - 1 ; j > 0 ; j --){
          m_pHamBuff->data[k][j] = m_pHamBuff->data[k][j] - m_fPreEm * m_pHamBuff->data[k][j-1] ;
        }
        m_pHamBuff->data[k][0] = m_pHamBuff->data[k][0] - m_fPreEm *  m_pHamBuff->data[k][0] ;
      
      }
      
      //Hamming
      m_pHamBuff->RowMul(m_pHam);
      
      //m_pHamBuff    ===>    m_pPsd
      m_ffts->Execute(m_pHamBuff,m_pPsd);
      
      //m_pPsd->Print("1");
      
      //Gen Fb
      GenFbData(m_pPsd->data, m_MelPos, m_MelWt, m_pFb->data[i]);
      
      //m_pFb->Print("1");
      
      //to log
      for (int m = 0 ; m < m_iFbDim ; m ++){
        if(m_pFb->data[i][m] < 0.0000001f){
          m_pFb->data[i][m] = 0.0000001f ;
        }
        m_pFb->data[i][m] = log(m_pFb->data[i][m]);
      }
    }
    return  0 ;
    
  }
  
  int	ZFea1::InitMelPos(float fLoHz,float fHiHz, ZVec* pMelPos, ZVec* pMelWt){
    
    float fLoHz_M = (float)mel(fLoHz) ;
    float fHiHz_M = (float)mel(fHiHz) ;
    
    float * FiltFreq = Z_SAFE_NEW_AR1(FiltFreq,float, pMelPos->size) ;
    float fSpan_M = (fHiHz_M - fLoHz_M)/(pMelPos->size - 1);
    for (int i = 0 ; i < pMelPos->size ; i ++){
      FiltFreq[i] = fLoHz_M + i * fSpan_M;
    }
    
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
  
  int ZFea1::GenFbData(float* pPsd, ZVec* pMelPos, ZVec* pMelWt, float* pFb){
    
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




