//
//  r2ad2api.cpp
//  r2ad
//
//  Created by hadoop on 10/22/15.
//  Copyright (c) 2015 hadoop. All rights reserved.
//

#include "r2ad2.h"

#include "r2sto2.h"
#include "r2mem2.h"
#include "../r2math.h"
#include <assert.h>

static r2sto2* g_pSto2 = NULL ;

int r2ad2_sysinit(const char* pWorkDir){
  
  if (g_pSto2 == NULL) {
    g_pSto2 = R2_SAFE_NEW(g_pSto2, r2sto2, pWorkDir);
  }
  return 0 ;
}

int r2ad2_sysexit(){
  
  R2_SAFE_DEL(g_pSto2) ;
  
  return 0 ;
}

r2ad2_htask r2ad2_create(){
  
  r2mem2* pTask = R2_SAFE_NEW(pTask, r2mem2, g_pSto2) ;
  ZLOG_INFO("---------------------------------------------initok");
  
  return (r2ad2_htask)pTask ;
}

int r2ad2_free(r2ad2_htask htask){
  
  r2mem2* pTask = (r2mem2*) htask ;
  R2_SAFE_DEL(pTask) ;
  
  return 0 ;
}

int r2ad2_reset(r2ad2_htask htask){
  
  r2mem2* pTask = (r2mem2*) htask ;
  return pTask->reset();
}

int r2ad2_putaudiodata2(r2ad2_htask htask, char* pData_In, int iLen_In, int iAecFlag, int iAwakeFlag, int iSleepFlag, int iAsrFlag){
  
  
  r2mem2* pTask = (r2mem2*) htask ;
  return pTask->ProcessData(pData_In, iLen_In, iAecFlag, iAwakeFlag, iSleepFlag, iAsrFlag, 0);
}

int r2ad2_getmsg2(r2ad2_htask htask, r2ad_msg_block*** pMsgLst, int *iMsgNum){
  
  r2mem2* pTask = (r2mem2*) htask ;
  return pTask->GetMsgLst(*pMsgLst, *iMsgNum);
}

float  r2ad2_getenergy_Lastframe(r2ad2_htask htask){
  
  r2mem2* pTask = (r2mem2*) htask ;
  return pTask->m_pMem_Vbv3->GetEn_LastFrm() ;
}

float  r2ad2_getenergy_Threshold(r2ad2_htask htask){
  
  r2mem2* pTask = (r2mem2*) htask ;
  return pTask->m_pMem_Vbv3->GetEn_Shield() ;
}

int r2ad2_steer(r2ad2_htask htask, float fAzimuth, float fElevation){
  
  r2mem2* pTask = (r2mem2*) htask ;
  return pTask->m_pMem_Bf->steer(fAzimuth, fElevation) ;
}
