//
//  zfea2.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zfea2__
#define __r2vt4__zfea2__

#include "../mat/zvec.h"
#include "../mat/zmat.h"
#include "../fft/zfftr2r.h"

namespace  __r2vt4__ {
  
#define CHS_NUM			22
#define CHS_NUM_W		7
  
#define mel(x) (1127.0f*logf(1.0f + (x)/700.0f))
#define mel_inv(x) ((expf((x)/1127.0f)-1.0f)*700.0f)
  
  class ZFea2
  {
  public:
    ZFea2(int iSr, int iBat, int iCn);
  public:
    ~ZFea2(void);
    
  public:
    int ResetFb();
    int ContinueFb(const float** pAud, int len,ZMat* &pFb, int &iFbNum);
    
  protected:
    int InitFb(int iSr, int iBat, int iCn);
    int	InitMelPos(float fLoHz,float fHiHz, ZVec* pMelPos, ZVec* pMelWt);
    int	InitMelPos2(float fLoHz,float fHiHz, ZVec* pMelPos, ZVec* pMelWt);
    
    int ProBlockBuff();
    int GenFbData(float* pPsd, ZVec* pMelPos, ZVec* pMelWt, float* pFb);
    
  public:
    //	FB Info
    int		m_iBat;			/* number of batch data*/
    int		m_iSr;			/* sample Frequency in KHz */
    int   m_iCn ;
    
    int		m_iFFT;          /* fft size */
    int		m_iWin;
    int		m_iSft;
    
    int		m_iFbDim;
    ZVec*	m_MelPos_L ;
    ZVec*	m_MelWt_L ;
    ZVec*	m_MelPos_H ;
    ZVec*	m_MelWt_H ;
    
    int m_iBatPos ;
    int m_iBatWin ;
    float m_fPreEm ;
    ZMat* m_pPreEmBuff ;
    
    ZVec* m_pHam ;
    ZMat* m_pHamBuff ;
    ZMat* m_pPsd ;
    
    //fftw
    ZFFT* m_fft ;
    int	m_iFbNum ;
    int m_iFbTotal ;
    ZMat* m_pFb ;
  };
  
}

#endif /* __r2vt4__zfea2__ */
