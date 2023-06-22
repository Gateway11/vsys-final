//
//  zrssto.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zrssto__
#define __r2vt4__zrssto__

#include "fa_resample.h"
#include "../mat/zmat.h"
#include "../zmath.h"
#include "../mat/zmat_neon.h"

namespace __r2vt4__ {
  
  class ZRsMem
  {
  public:
    ZRsMem(int iCn, int iSrIn, int iSrOut, int iFrmOut);
    
  public:
    ~ZRsMem(void);
    
  public:
    int Reset();
    int Process(const float** pWavIn, const int iLenIn, float** &pWavOut, int &iLenOut);
    int ProcessBatch();
    
  public:
    
    int m_iCn ;
    int m_iSrIn ;
    int m_iSrOut ;
    
  protected:
    
    int m_iM ;
    int m_iL ;
    int m_iQ ;
    int m_iFrmOut ;
    int m_iBatchSize ;
    
    
    ZNS_LIBRESAMPLE::uintptr_t m_pResample ;
    ZMat* m_pFileter ;
    ZMat_Neon* m_pFileter_Neon ;
    
    ZVec* m_pBatchAud_In ;
    ZVec* m_pBatchAud_Out ;
  
  protected:
    
    ZMat* m_pAud_In ;
    int m_iLen_In ;
    int m_iLen_In_Total ;
    
    ZMat* m_pAud_Out ;
    int m_iLen_Out ;
    int m_iLen_Out_Total ;
    
  };
  
};


#endif /* __r2vt4__zrssto__ */
