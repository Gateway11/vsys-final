//
//  zcfapi.cpp
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zcfapi.h"
//#include "zcfsto.h"
#include "zcfmem.h"

using namespace __r2vt4__ ;

/************************************************************************/
/** System Init	, Exit
 */
int r2_cf_sysinit(){
  
#ifdef QUICK_EXP
  ZMat::InitQuickExp();
#endif
  
  return  0 ;
  
}

int r2_cf_sysexit(){
  
#ifdef QUICK_EXP
  ZMat::ExitQuickExp();
#endif
  
  return  0 ;
  
}


/************************************************************************/
/** Task Alloc , Free
 */
r2_cf_htask r2_cf_create(const char* pNnetPath, int iCn){
  
  ZCfMem2* pCfMem = Z_SAFE_NEW(pCfMem, ZCfMem2, pNnetPath, iCn);
  
  return (r2_cf_htask)pCfMem ;
  
}

int r2_cf_free(r2_cf_htask hTask){
  
  ZCfMem2* pCfMem = (ZCfMem2*) hTask ;
  
  Z_SAFE_DEL(pCfMem);
  
  return 0 ;
  
}

/************************************************************************/
/** classify
 *
 * pWaveBuff: iWavLen * iCn
 * iFlag: 0 End
 */
float r2_cf_check_buff(r2_cf_htask hTask, const float** pWavBuff, int iWavLen, int iCn, int iFlag){
  
  ZCfMem2* pCfMem = (ZCfMem2*) hTask ;
  return pCfMem->check_buff(pWavBuff, iWavLen, iCn);
  
}

float r2_cf_check_file(r2_cf_htask hTask, const char* pFilePath, int iFlag){
  
  ZCfMem2* pCfMem = (ZCfMem2*) hTask ;
  return pCfMem->check_file(pFilePath);
  
}

int r2_cf_get_score(r2_cf_htask hTask, const float** pWavData, int iWavLen, int iCn, float* pScore, int iScoreLen){
  
  ZCfMem2* pCfMem = (ZCfMem2*) hTask ;
  return pCfMem->get_score(pWavData, iWavLen, iCn, pScore, iScoreLen);
  
}




