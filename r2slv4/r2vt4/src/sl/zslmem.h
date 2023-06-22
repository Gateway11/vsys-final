//
//  zslmem.h
//  r2vt4
//
//  Created by hadoop on 3/24/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zslmem__
#define __r2vt4__zslmem__

#include "../mat/zmat.h"
#include "../mat/zcmat.h"

#include "../fft/zfftr2c.h"

#include "spline.h"

namespace __r2vt4__ {
  
  class ZSlMem
  {
  public:
    ZSlMem( int iMicNum, float* pMicPos, float* pMicI2sDelay);
    
  public:
    virtual ~ZSlMem(void);
    
    int init(int iMicNum, float* pMicPos, float* pMicI2sDelay, int iSectorNum_A, int iSectorNum_H);
    
    int PutData(float** pfDataBuff, int iDataLen) ;
    int GetCandidates(int iStartPos, int iEndPos, float *pfCandidates, int iCandiNum) ;
    int Reset();
    
  protected:
    float GetDis(float x , float y , float z, int iMicIdA, int iMicIdB);
    int ProcessSingleFrm();
    
  public:
    
    //Mic Info
    int   m_iMicNum ;
    ZMat* m_pMicPos ;
    
    int   m_iMicPairNum ;
    ZMat* m_pMicPair ;
    
    int   m_iSectorNum ;
    ZMat* m_pSectorInfo ;
    ZMat* m_pTimeDelay ;
    
    int   m_iMaxShiftPoint ;
    
    //Storage
    int		m_iFFT;          /* fft size */
    int   m_iFFT_r ;
    int		m_iWin;
    int		m_iSft;
    
    //Spline
    //ZSpline* m_pSpline ;
    spline* m_pSpline ;
    
    //Frm
    int     m_iPos;
    ZMat*   m_pMat_Win ;
    ZMat*   m_pMat_Ham ;
    ZVec*   m_pVec_Ham ;
    
    ZCMat*  m_pCMat_FFT ;
    ZCMat*  m_pCMat_GCC ;
    ZMat*   m_pMat_PHAT ;
    
    //For Spline
    ZMat*   m_pMat_SP ;
    
    //FFT
    ZFFT_R2C* m_pFFT_R2C ;
    ZFFT_C2R* m_pFFT_C2R ;
    
    ZVec*   m_pVec_Score ;
    
    //Score Loop
    int     m_iLoopPos ;
    int     m_iLoopSize ;
    ZMat*   m_pLoopMat ;
    
    ZVec* m_pScore ;
    
    //No New
    ZVec*   m_pData_NoNew ;
    
  };
  
};


#endif /* __r2vt4__zslmem__ */
