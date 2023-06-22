//
//  zcfmem.h
//  r2vt4
//
//  Created by hadoop on 6/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zcfmem__
#define __r2vt4__zcfmem__

#include "../dnn/znnet.h"
#include "../fea/zfea1.h"
#include "../mat/zvec.h"


namespace __r2vt4__ {
  
  class ZCfMem2
  {
  public:
    ZCfMem2(const char* pNnetPath, int iCn);
    
  public:
    ~ZCfMem2(void);
    
    float check_buff(const float** pWavData, int iWavLen, int iCn);
    float check_file(const char* pWavPath);
    int get_score(const float** pWavData, int iWavLen, int iCn, float* pScore, int iScoreLen);
    
  protected:
    
    int DoCmvn(ZMat* pFb);
    
    ZFea1* m_pFea1 ;
    ZFea1* m_pFea2 ;
    
    Nnet* m_pNnet ;
    Nnet_Memory_Total*  m_pNnetMem ;
    
    ZVec* m_pVec_Mean_g ;
    ZVec* m_pVec_Var_g ;
    ZVec* m_pVec_Mean_u ;
    ZVec* m_pVec_Var_u ;
    
  };
  
  
};


#endif /* __r2vt4__zcfmem__ */
