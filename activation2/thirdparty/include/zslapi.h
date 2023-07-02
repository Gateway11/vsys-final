//
//  zslapi.h
//  r2vt5
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt5__zslapi__
#define __r2vt5__zslapi__

#ifdef __cplusplus
extern "C" {
#endif
  
  
  /**
   *  寻向算法类型
   *
   *  @param SL_GCC      GCC PHAT
   *  @param SL_SRP      SRP PHAT
   *  @param SL_PSRP     PSRP
   *  @param SL_MCCC     MCCC
   */
  enum r2_sl_algo{
    R2_SL_ALGO_GCC = 1 ,
    R2_SL_ALGO_SRP ,
    R2_SL_ALGO_PSRP ,
    R2_SL_ALGO_MCCC ,
    R2_SL_ALGO_SICHONG
  };
  
  enum r2_sl_micarray_shape{
    R2_SL_MICARRAY_LINEAR = 1,    //线阵
    R2_SL_MICARRAY_PLANE,         //面阵
    R2_SL_MICARRAY_CUBIC          //空阵
  };
  
  /** task handle */
  typedef long long r2_sl_htask ;
  
  /************************************************************************/
  /** System Init	, Exit
   */
  int r2_sl_sysinit(float fRadius, r2_sl_micarray_shape shape,  r2_sl_algo algo);
  int r2_sl_sysexit();
  
  /************************************************************************/
  /** Task Alloc , Free
   */
  r2_sl_htask r2_sl_create(int iMicNum, float* pMicPos, float* pMicI2sDelay);
  int r2_sl_free(r2_sl_htask hTask);
  
  /************************************************************************/
  /** R2 SL
   *
   * pWaveBuff: iWavLen * iCn
   * iFlag: 0 End
   */
  float r2_sl_put_data(r2_sl_htask hTask, float** pfDataBuff, int iDataLen);
  float r2_sl_get_candidate(r2_sl_htask hTask, int iStartPos, int iEndPos, float *pfCandidates, int iCandiNum);
  
  float r2_sl_get_candidata2(r2_sl_htask hTask, int iStartPos, int iEndPos, float *pfCandidates, int& iCandiNum);
  
  int r2_sl_reset(r2_sl_htask hTask);
  
#ifdef __cplusplus
};
#endif



#endif /* __r2vt5__zslapi__ */
