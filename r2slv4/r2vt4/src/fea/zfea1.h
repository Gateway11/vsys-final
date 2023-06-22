//
//  zfea1.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zfea1__
#define __r2vt4__zfea1__

#include "../zmath.h"
#include "../mat/zvec.h"
#include "../mat/zmat.h"
#include "../fft/zfftr2r.h"

namespace __r2vt4__ {

#define mel(x) (1127.0f*logf(1.0f + (x)/700.0f))
#define mel_inv(x) ((expf((x)/1127.0f)-1.0f)*700.0f)
  
  class ZFea1
  {
  public:
    ZFea1(int iSr, int iCn, int iFbDim);
  public:
    ~ZFea1(void);
    
  public:
    int ExtractFBank(const float** pAud, int len, ZMat* &pFb);
    
  protected:
    int	InitMelPos(float fLoHz,float fHiHz, ZVec* pMelPos, ZVec* pMelWt);
    
    int GenFbData(float* pPsd, ZVec* pMelPos, ZVec* pMelWt, float* pFb);
    
  public:
    //	FB Info
    int		m_iSr;			/* sample Frequency in KHz */
    int   m_iCn;
    
    int		m_iFFT;          /* fft size */
    int		m_iWin;
    int		m_iSft;
    
    int		m_iFbDim;
    ZVec*	m_MelPos ;
    ZVec*	m_MelWt ;
    
    float m_fPreEm ;
    
    ZVec* m_pHam ;
    ZMat* m_pHamBuff ;
    ZVec* m_pPsd ;
    
    //fftw
    ZFFT* m_ffts ;
    
    //out
    int m_iFbTotal ;
    ZMat* m_pFb ;
  };
  

};


#endif /* __r2vt4__zfea1__ */
