//
//  zslapi.cpp
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zslapi.h"
#include "zslmem.h"

#include "../mat/zmat.h"

using namespace __r2vt4__ ;

/************************************************************************/
/** System Init	, Exit
 */
int r2_sl_sysinit(){
  
#ifdef QUICK_EXP
  ZMat::InitQuickExp();
#endif
  
  return  0 ;
  
}

int r2_sl_sysexit(){
  
#ifdef QUICK_EXP
  ZMat::ExitQuickExp();
#endif
  
  return  0 ;
  
}


/************************************************************************/
/** Task Alloc , Free
 */
r2_sl_htask r2_sl_create(int iMicNum, float* pMicPos, float* pMicI2sDelay){
  
  ZSlMem* pSlMem = Z_SAFE_NEW(pSlMem, ZSlMem, iMicNum, pMicPos, pMicI2sDelay);
  
  return (r2_sl_htask)pSlMem ;
}

int r2_sl_free(r2_sl_htask hTask){
  
  ZSlMem* pSlMem = (ZSlMem*) hTask ;
  
  Z_SAFE_DEL(pSlMem);
  
  return 0 ;
}

/************************************************************************/
/** R2 SL
 *
 * pWaveBuff: iWavLen * iCn
 * iFlag: 0 End
 */
float r2_sl_put_data(r2_sl_htask hTask, float** pfDataBuff, int iDataLen){
  
  ZSlMem* pSlMem = (ZSlMem*) hTask ;
  return  pSlMem->PutData(pfDataBuff, iDataLen) ;
}

float r2_sl_get_candidate(r2_sl_htask hTask, int iStartPos, int iEndPos, float *pfCandidates, int iCandiNum){
  
  ZSlMem* pSlMem = (ZSlMem*) hTask ;
  return  pSlMem->GetCandidates(iStartPos, iEndPos, pfCandidates, iCandiNum) ;
}

int r2_sl_reset(r2_sl_htask hTask){
  
  ZSlMem* pSlMem = (ZSlMem*) hTask ;
  return pSlMem->Reset();
}




