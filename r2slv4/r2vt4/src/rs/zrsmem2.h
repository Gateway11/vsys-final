//
//  zrsmem2.h
//  r2vt4
//
//  Created by hadoop on 6/1/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zrsmem2__
#define __r2vt4__zrsmem2__

#include "../zmath.h"
#include "../mat/zmat.h"
#include "r2ssp.h"

namespace __r2vt4__ {

  class ZRsMem2
  {
  public:
    ZRsMem2(int iCn, int iSrIn, int iSrOut);
    
  public:
    ~ZRsMem2(void);
    
  public:
    int Reset();
    int Process(const float** pWavIn, const int iLenIn, float** &pWavOut, int &iLenOut);
 
  public:
    
    int m_iCn ;
    int m_iSr_In ;
    int m_iSr_Out ;
    
    r2ssp_handle* m_hEngine_Rs ;
    
    int m_iFrmSize_In ;
    int m_iLen_In;
    ZMat* m_pMat_In ;
    
    int m_iFrmSize_Out ;
    int m_iLen_Out ;
    ZMat* m_pMat_Out ;
    
  };

  

};


#endif /* __r2vt4__zrsmem2__ */
