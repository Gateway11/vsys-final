//
//  zvbvapi.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zvbvapi.h"
#include "../zmath.h"
#include "zvbvmem.h"

using namespace __r2vt4__ ;


ZVbvSto* pVbvSto = NULL ;

r2_vbv_htask r2_vbv_create(int iMicNum, float* pMicPos, float* pMicDelay, const char* pVtNnetPath, const char* pVtPhoneTablePath){
  
  if (pVbvSto == NULL) {
    pVbvSto = Z_SAFE_NEW(pVbvSto, ZVbvSto, iMicNum, pMicPos, pMicDelay,  pVtNnetPath, pVtPhoneTablePath) ;
  }
  
  ZVbvMem* pVbvMem = Z_SAFE_NEW(pVbvMem, ZVbvMem, pVbvSto) ;
  
  
  return  (r2_vbv_htask) pVbvMem ;
}



int r2_vbv_set_param(r2_vbv_htask hTask, const int iParam, const void* pValue){
  
  ZVbvMem* pVbvMem = (ZVbvMem*)hTask ;
  return  pVbvMem->SetParam(iParam, pValue);
}



int r2_vbv_free(r2_vbv_htask hTask){
  
  ZVbvMem* pVbvMem = (ZVbvMem*)hTask ;
  Z_SAFE_DEL(pVbvMem);
  
  Z_SAFE_DEL(pVbvSto);
  
  return  0 ;
}


int r2_vbv_setwords(r2_vbv_htask hTask, const WordInfo* pWordLst, int iWordNum){
  
  ZVbvMem* pVbvMem = (ZVbvMem*)hTask ;
  return  pVbvMem->m_pVt->SetWords(pWordLst, iWordNum) ;
}


int r2_vbv_getwords(r2_vbv_htask hTask, const WordInfo** pWordLst, int* iWordNum){
  
  ZVbvMem* pVbvMem = (ZVbvMem*)hTask ;
  return  pVbvMem->m_pVt->GetWords(pWordLst, iWordNum) ;
}

int r2_vbv_process(r2_vbv_htask hTask, const float** pWavBuff, int iWavLen, int iVtFlag, bool bDirtyReset){
  
  ZVbvMem* pVbvMem = (ZVbvMem*)hTask ;
  return  pVbvMem->Process(pWavBuff, iWavLen, iVtFlag, bDirtyReset) ;
}


int r2_vbv_getdetwordinfo(r2_vbv_htask hTask, const WordInfo** pWordInfo, const WordDetInfo** pWordDetInfo){
  
  ZVbvMem* pVbvMem = (ZVbvMem*)hTask ;
  return  pVbvMem->m_pVt->GetDetWordInfo(pWordInfo, pWordDetInfo) ;
}


int r2_vbv_reset(r2_vbv_htask hTask){
  
  ZVbvMem* pVbvMem = (ZVbvMem*)hTask ;
  return  pVbvMem->Reset() ;
}


int r2_vbv_getlastaudio(r2_vbv_htask hTask, int iStart, int iEnd, float** pWavBuff){
  
  ZVbvMem* pVbvMem = (ZVbvMem*)hTask ;
  return  pVbvMem->m_pAudBuff->GetLastAudio(pWavBuff, iStart, iEnd) ;
}


int r2_vbv_getsl(r2_vbv_htask hTask, int iStart, int iEnd, float pSlInfo[3]){
  
  ZVbvMem* pVbvMem = (ZVbvMem*)hTask ;
  return  pVbvMem->m_pSl->GetSl(iStart, iEnd, pSlInfo) ;
}

float  r2_vbv_geten_lastfrm(r2_vbv_htask hTask){
  
  ZVbvMem* pVbvMem = (ZVbvMem*)hTask ;
  return pVbvMem->m_pVad->getenergy_Lastframe() ;
}

float  r2_vbv_geten_shield(r2_vbv_htask hTask){
  
  ZVbvMem* pVbvMem = (ZVbvMem*)hTask ;
  return pVbvMem->m_pVad->getenergy_Threshold() ;
}
