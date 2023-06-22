//
//  zslmem.cpp
//  r2vt4
//
//  Created by hadoop on 3/24/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zslmem.h"

namespace __r2vt4__ {
  
  ZSlMem::ZSlMem(  int iMicNum, float* pMicPos, float* pMicI2sDelay){
    
    init(iMicNum, pMicPos, pMicI2sDelay, 72, 1);
    
    m_pData_NoNew = Z_SAFE_NEW(m_pData_NoNew, ZVec, 1000) ;

  }
  
  ZSlMem::~ZSlMem(void){
    
    Z_SAFE_DEL(m_pMicPos);
    Z_SAFE_DEL(m_pMicPair);
    Z_SAFE_DEL(m_pSectorInfo);
    Z_SAFE_DEL(m_pTimeDelay);
    
    Z_SAFE_DEL(m_pMat_Win);
    Z_SAFE_DEL(m_pMat_Ham);
    Z_SAFE_DEL(m_pVec_Ham);
    
    Z_SAFE_DEL(m_pCMat_FFT);
    Z_SAFE_DEL(m_pCMat_GCC);
    Z_SAFE_DEL(m_pMat_PHAT);
    
    Z_SAFE_DEL(m_pMat_SP);
    
    Z_SAFE_DEL(m_pFFT_R2C);
    Z_SAFE_DEL(m_pFFT_C2R);
    
    Z_SAFE_DEL(m_pSpline);
    Z_SAFE_DEL(m_pVec_Score);
    Z_SAFE_DEL(m_pLoopMat);
    
    Z_SAFE_DEL(m_pScore) ;
    
    Z_SAFE_DEL(m_pData_NoNew) ;
  }
  
  int ZSlMem::init(int iMicNum, float* pMicPos, float* pMicI2sDelay, int iSectorNum_A, int iSectorNum_H){
    
    
    //Init Frm Info
    m_iFFT = 1024 ;
    m_iFFT_r = m_iFFT;
    m_iWin = 1024 ;
    m_iSft = 256 ;
    
    
    //Init Mic Info
    m_iMicNum = iMicNum ;
    m_pMicPos = Z_SAFE_NEW(m_pMicPos, ZMat, m_iMicNum, 3);
    for (int i = 0 ; i < m_iMicNum ; i ++) {
      m_pMicPos->data[i][0] = pMicPos[i*3] ;
      m_pMicPos->data[i][1] = pMicPos[i*3+1] ;
      m_pMicPos->data[i][2] = pMicPos[i*3+2] ;
    }
    
    //Init Mic Pair
    m_iMicPairNum = (m_iMicNum - 1) * m_iMicNum / 2 ;
    m_pMicPair = Z_SAFE_NEW(m_pMicPair, ZMat, m_iMicPairNum, 2);
    m_iMicPairNum = 0 ;
    for (int i = 0 ; i < m_iMicNum ; i ++) {
      for (int j = i + 1 ; j < m_iMicNum ; j ++) {
        m_pMicPair->data_i[m_iMicPairNum][0] = i ;
        m_pMicPair->data_i[m_iMicPairNum][1] = j ;
        m_iMicPairNum ++ ;
      }
    }
    //assert(m_iMicPairNum == (m_iMicNum - 1) * m_iMicNum / 2) ;
    
    //Init Mic Delay and Max Shift Point
    int iTotalNum = iSectorNum_A * iSectorNum_H + 1 ;
    m_pSectorInfo = Z_SAFE_NEW(m_pSectorInfo, ZMat, iTotalNum, 2);
    m_iSectorNum = 0 ;
    for (int i = 0 ; i < iSectorNum_H ; i ++) {
      int iSN = iSectorNum_A * cos(i * Z_PI / iSectorNum_H  / 2) ;
      //printf("%d:%d\n",i,iSN) ;
      for (int j = 0 ; j < iSN ; j ++) {
        m_pSectorInfo->data[m_iSectorNum][0] = Z_2PI * j / iSN ;
        m_pSectorInfo->data[m_iSectorNum][1] = i * Z_PI / iSectorNum_H / 2 ;
        m_iSectorNum ++ ;
      }
    }
    
    if (iSectorNum_H > 1) {
      m_pSectorInfo->data[m_iSectorNum][0] = 0  ;
      m_pSectorInfo->data[m_iSectorNum][1] = Z_PI / 2 ;
      m_iSectorNum ++ ;
    }
    
    m_pTimeDelay = Z_SAFE_NEW(m_pTimeDelay, ZMat, m_iMicPairNum, m_iSectorNum);
    m_iMaxShiftPoint = 0 ;
    
    float x=0.0f, y=0.0f, z=0.0f, r=3.0, d=0.0f ;
    for (int i = 0 ; i < m_iSectorNum ; i ++) {
      x = r * cos(m_pSectorInfo->data[i][0]) * cos(m_pSectorInfo->data[i][1]) ;
      y = r * sin(m_pSectorInfo->data[i][0]) * cos(m_pSectorInfo->data[i][1]) ;
      z = r * sin(m_pSectorInfo->data[i][1]) ;
      for (int j = 0 ; j < m_iMicPairNum ; j ++) {
        if (pMicI2sDelay != NULL) {
          d = GetDis(x, y, z, m_pMicPair->data_i[j][0], m_pMicPair->data_i[j][1]) - 16000 * ( pMicI2sDelay[m_pMicPair->data_i[j][0]] - pMicI2sDelay[m_pMicPair->data_i[j][1]] ) ;
        }else{
          d = GetDis(x, y, z, m_pMicPair->data_i[j][0], m_pMicPair->data_i[j][1]) ;
        }
        if (m_iMaxShiftPoint < fabs(d)) {
          m_iMaxShiftPoint = fabs(d) ;
        }
        m_pTimeDelay->data[j][i] = d ;
      }
    }
    
    m_iMaxShiftPoint ++ ;
    for (int i = 0 ; i < m_iSectorNum ; i ++) {
      for (int j = 0 ; j < m_iMicPairNum ; j ++) {
        m_pTimeDelay->data[j][i] += m_iMaxShiftPoint ;
      }
    }

    
    m_iPos = 0 ;
    m_pMat_Win = Z_SAFE_NEW(m_pMat_Win, ZMat, m_iMicNum, m_iFFT);
    m_pVec_Ham = Z_SAFE_NEW(m_pVec_Ham, ZVec, m_iFFT);
    m_pMat_Ham = Z_SAFE_NEW(m_pMat_Ham, ZMat, m_iMicNum, m_iFFT);
    float fFrmWin = 0.54f ;
    for (int i = 0 ; i < m_iWin ; i ++){
      m_pVec_Ham->data[i] = (float)(fFrmWin - (1-fFrmWin)*cos(2 * i * Z_PI / (m_iWin-1)));
    }
    
    m_pCMat_FFT = Z_SAFE_NEW(m_pCMat_FFT, ZCMat, m_iMicNum , m_iFFT);
    m_pCMat_GCC = Z_SAFE_NEW(m_pCMat_GCC, ZCMat, m_iMicPairNum , m_iFFT);
    m_pMat_PHAT = Z_SAFE_NEW(m_pMat_PHAT, ZMat, m_iMicPairNum , m_iFFT_r);
    
    m_pMat_SP = Z_SAFE_NEW(m_pMat_SP, ZMat, m_iMicPairNum, m_iMaxShiftPoint * 2 + 1) ;
    
    //FFT
    m_pFFT_R2C = Z_SAFE_NEW(m_pFFT_R2C, ZFFT_R2C, m_iFFT, m_iMicNum) ;
    m_pFFT_C2R = Z_SAFE_NEW(m_pFFT_C2R, ZFFT_C2R, m_iFFT_r, m_iMicPairNum) ;
    
    //
    //m_pSpline = Z_SAFE_NEW(m_pSpline, ZSpline, m_iMaxShiftPoint * 2 + 1);
    m_pSpline = Z_SAFE_NEW(m_pSpline, spline);
    m_pVec_Score = Z_SAFE_NEW(m_pVec_Score, ZVec, m_iSectorNum) ;
    
    //Score Loop
    m_iLoopPos = 0;
    m_iLoopSize = 20 * 16000 / m_iSft ;
    m_pLoopMat = Z_SAFE_NEW(m_pLoopMat, ZMat, m_iLoopSize, m_iSectorNum);
    
    
    m_pScore = Z_SAFE_NEW(m_pScore, ZVec, m_iSectorNum) ;
    return 0 ;
  }
  
  int ZSlMem::PutData(float** pfDataBuff, int iDataLen){
    
    int iLeft = m_iWin - m_iSft ;
    m_pData_NoNew->CheckMaxSize(iLeft);
    
    int iCur = 0 ;
    while (iCur < iDataLen) {
      int iLen = zmin(iDataLen - iCur, m_iWin - m_iPos) ;
      for (int i = 0 ; i < m_iMicNum ; i ++) {
        memcpy(m_pMat_Win->data[i] + m_iPos, pfDataBuff[i] + iCur , sizeof(float) * iLen) ;
      }
      iCur += iLen ;
      m_iPos += iLen ;
      
      if (m_iPos == m_iWin) {
        ProcessSingleFrm() ;
        m_iPos = iLeft ;
        
        if (iLeft > 0) {
          for (int i = 0 ; i < m_iMicNum ; i ++) {
            memcpy(m_pData_NoNew->data,  m_pMat_Win->data[i] + m_iSft, sizeof(float) * iLeft) ;
            memcpy(m_pMat_Win->data[i], m_pData_NoNew->data, sizeof(float) * iLeft);
        }
      }
    }
    }
    
    return 0 ;
  }
  
  int ZSlMem::GetCandidates(int iStartPos, int iEndPos, float *pfCandidates, int iCandiNum){
    
    assert(iStartPos > 0);
    assert(iEndPos >= 0);
    assert(iStartPos > iEndPos) ;
    //assert(iCandiNum ==1);
    
    int iStartFrm = (iStartPos - m_iWin) / m_iSft + 1 ;
    int iEndFrm = iEndPos / m_iSft ;
    
    if (iStartFrm < iEndFrm + 4) {
      iStartFrm = iEndFrm + 4 ;
    }
    
    assert(iStartFrm > iEndFrm) ;
    
    m_pScore->CheckMaxSize(m_iSectorNum) ;
    
    for (int i = iStartFrm ; i > iEndFrm ; i--) {
      int pos = m_iLoopPos - i  ;
      while (pos < 0) {
        pos += m_iLoopSize ;
      }
      for (int j = 0 ; j < m_iSectorNum ; j ++) {
        m_pScore->data[j] += m_pLoopMat->data[pos][j] ;
      }
    }
    
    int iMaxId = 0 ;
    for (int j = 0 ; j < m_iSectorNum ; j ++) {
      if (m_pScore->data[j] > m_pScore->data[iMaxId]) {
        iMaxId = j ;
      }
    }
    memcpy(pfCandidates, m_pSectorInfo->data[iMaxId], sizeof(float)*2);
    
    //printf("MaxId:%d\n",iMaxId);
    //pScore->Print("Final Score");
    
    
    //    FILE* pScoreFile = fopen("/Users/hadoop/score.txt", "wb");
    //    for (int i = 0 ; i < m_iLoopPos ; i ++) {
    //      for (int j = 0 ; j < m_iSectorNum ; j ++) {
    //        fprintf(pScoreFile, "%f\t",m_pLoopMat->data[i][j]);
    //      }
    //      fprintf(pScoreFile, "\n");
    //    }
    //    fclose(pScoreFile) ;
    
    return 0 ;
  }
  
  int ZSlMem::Reset(){
    
    m_iPos = 0 ;
    m_iLoopPos = 0 ;
    
    return 0 ;
  }
  
  
  float ZSlMem::GetDis(float x , float y , float z, int iMicIdA, int iMicIdB){
    
    float disA = (x - m_pMicPos->data[iMicIdA][0]) * (x - m_pMicPos->data[iMicIdA][0]) ;
    disA += (y - m_pMicPos->data[iMicIdA][1]) * (y - m_pMicPos->data[iMicIdA][1]) ;
    disA += (z - m_pMicPos->data[iMicIdA][2]) * (z - m_pMicPos->data[iMicIdA][2]) ;
    disA = sqrtf(disA);
    
    float disB = (x - m_pMicPos->data[iMicIdB][0]) * (x - m_pMicPos->data[iMicIdB][0]) ;
    disB += (y - m_pMicPos->data[iMicIdB][1]) * (y - m_pMicPos->data[iMicIdB][1]) ;
    disB += (z - m_pMicPos->data[iMicIdB][2]) * (z - m_pMicPos->data[iMicIdB][2]) ;
    disB = sqrtf(disB);
    
    return  (disB - disA) * 16000.0f / 343.3f * m_iFFT_r / m_iFFT ;
    
  }
  
  int ZSlMem::ProcessSingleFrm(){
    
    m_pMat_Ham->Copy(m_pMat_Win);
    m_pMat_Ham->RowMul(m_pVec_Ham);
    
    //FFT
    m_pFFT_R2C->ExecuteR2C(m_pMat_Ham, m_pCMat_FFT) ;
    //m_pCMat_FFT->Print("m_pCMat_FFT");
    
    //GCC
    ZCMat::CalcGcc(m_pCMat_FFT, m_pMicPair, m_pCMat_GCC);
    //m_pCMat_GCC->Print("m_pCMat_GCC");
    
    //PHAT
    m_pFFT_C2R->ExecuteC2R(m_pCMat_GCC, m_pMat_PHAT);
    //m_pMat_PHAT->Print("m_pMat_PHAT");
    
    
    //Pre Spline
    for (int i = 0 ; i < m_iMicPairNum ; i ++) {
      memcpy(m_pMat_SP->data[i], m_pMat_PHAT->data[i] + m_iFFT - m_iMaxShiftPoint,
             sizeof(float) * m_iMaxShiftPoint) ;
      memcpy(m_pMat_SP->data[i] + m_iMaxShiftPoint , m_pMat_PHAT->data[i] ,
             sizeof(float) * (m_iMaxShiftPoint + 1) ) ;
    }
    m_pMat_SP->RowMul(1.0f/m_iFFT);
    //m_pMat_SP->Print("m_pMat_SP");
    
    //Get Score
    m_pVec_Score->Clean() ;
    
    //New Spline-----------------------------------------------------------
    //    for (int i = 0 ; i < m_iMicPairNum ; i ++) {
    //      m_pSpline->set_points(m_pMat_SP->data[i]);
    //      for (int j = 0 ; j < m_iSectorNum ; j ++) {
    //        m_pVec_Score->data[j] += m_pSpline->get_point(m_pTimeDelay->data[i][j]);
    //      }
    //    }
    
    //Old Spline-----------------------------------------------------------
    int iSplineSize = m_iMaxShiftPoint*2 + 1 ;
    std::vector<double> X ;
    std::vector<double> Y ;
    for (int i = 0 ; i < iSplineSize; i ++) {
      X.push_back(i);
      Y.push_back(i);
    }
    for (int i = 0 ; i < m_iMicPairNum ; i ++) {
      for (int j = 0; j < iSplineSize ; j ++) {
        Y[j] = m_pMat_SP->data[i][j] ;
      }
      m_pSpline->set_points(X, Y);
      for (int j = 0 ; j < m_iSectorNum ; j ++) {
        m_pVec_Score->data[j] += m_pSpline->get_point(m_pTimeDelay->data[i][j]);
      }
    }
    //m_pVec_Score->Print("11");
    //-----------------------------------------------------------
//    int iMaxId = 0 ;
//    for (int j = 0 ; j < m_iSectorNum ; j ++) {
//      if (m_pVec_Score->data[j] > m_pVec_Score->data[iMaxId]) {
//        iMaxId = j ;
//      }
//    }
//    //printf("%f\n", m_pSectorInfo->data[iMaxId][0] / Z_PI * 180 - 180 );
//    printf("%f\n", m_pVec_Score->data[iMaxId] / (m_iMicNum * (m_iMicNum - 1 )  / 2 ));
    //add loop
    memcpy(m_pLoopMat->data[m_iLoopPos], m_pVec_Score->data, sizeof(float) * m_iSectorNum);
    m_iLoopPos ++ ;
    if (m_iLoopPos == m_iLoopSize) {
      m_iLoopPos = 0 ;
    }
    
    return 0 ;
  }
  
  
};




