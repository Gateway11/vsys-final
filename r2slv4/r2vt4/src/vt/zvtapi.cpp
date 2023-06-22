//
//  zvtapi.cpp
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zvtapi.h"

#include "../zmath.h"
#include "../mat/zmat.h"
#include "../mat/zmat_neon.h"

using namespace __r2vt4__ ;

//#define USE_GASR

#ifdef USE_GASR
#include "../gvt/gvtmem.h"
#include "../gvt/gvtsto.h"
GVtSto* g_VtSto = NULL ;
#else
#include "zvtsto.h"
#include "zvtmem.h"
ZVtSto* g_VtSto = NULL ;
#endif

int r2_vt_sysinit(const char* pNnetPath, const char* pPhoneTablePath){
  
#ifdef QUICK_EXP
  ZMat::InitQuickExp();
#endif
  
  if (g_VtSto == NULL) {
#ifdef USE_GASR
    g_VtSto = Z_SAFE_NEW(g_VtSto,GVtSto, pNnetPath);
#else
    g_VtSto = Z_SAFE_NEW(g_VtSto,ZVtSto, pNnetPath, pPhoneTablePath);
#endif
  }
  
  return  0 ;
}

int r2_vt_sysexit(){
  
  Z_SAFE_DEL(g_VtSto);
  
#ifdef QUICK_EXP
  ZMat::ExitQuickExp();
#endif
  
  return  0 ;
}


r2_vt_htask r2_vt_create(int iCn){
  
#ifdef USE_GASR
  GVtMem* pVtMem = Z_SAFE_NEW(pVtMem,GVtMem, iCn);
#else
  ZVtMem* pVtMem = Z_SAFE_NEW(pVtMem,ZVtMem,g_VtSto, iCn);
#endif
  
  
  return (r2_vt_htask) pVtMem ;
}


int r2_vt_free(r2_vt_htask hTask){
  
#ifdef USE_GASR
  GVtMem* pVtMem = (GVtMem*) hTask ;
#else
  ZVtMem* pVtMem = (ZVtMem*) hTask ;
#endif
  
  Z_SAFE_DEL(pVtMem);
  
  return  0 ;
}

int r2_vt_setwords(r2_vt_htask hTask, const WordInfo* pWordLst, int iWordNum){
  
#ifdef USE_GASR
  GVtMem* pVtMem = (GVtMem*) hTask ;
#else
  ZVtMem* pVtMem = (ZVtMem*) hTask ;
#endif
  
  return  pVtMem->SetWords(pWordLst, iWordNum) ;
  
}

int r2_vt_getwords(r2_vt_htask hTask, const WordInfo** pWordLst, int* iWordNum){
  
#ifdef USE_GASR
  GVtMem* pVtMem = (GVtMem*) hTask ;
#else
  ZVtMem* pVtMem = (ZVtMem*) hTask ;
#endif
  
  return  pVtMem->GetWords(pWordLst, iWordNum) ;
}

int r2_vt_process(r2_vt_htask hTask, const float** pWavBuff, int iWavLen, int iVtFlag){
  
#ifdef USE_GASR
  GVtMem* pVtMem = (GVtMem*) hTask ;
#else
  ZVtMem* pVtMem = (ZVtMem*) hTask ;
#endif
  
  return  pVtMem->Detect(pWavBuff, iWavLen, iVtFlag) ;
}

int r2_vt_getdetwordinfo(r2_vt_htask hTask, const WordInfo** pWordInfo, const WordDetInfo** pWordDetInfo, int iOffsetFrm){
  
#ifdef USE_GASR
  GVtMem* pVtMem = (GVtMem*) hTask ;
  return  pVtMem->GetDetWordInfo(pWordInfo, pWordDetInfo, 0 ) ;
#else
  ZVtMem* pVtMem = (ZVtMem*) hTask ;
  return  pVtMem->GetDetWordInfo(pWordInfo, pWordDetInfo, iOffsetFrm + g_VtSto->m_iFrmOffset) ;
#endif
  
  
}

int r2_vt_reset(r2_vt_htask hTask){
  
#ifdef USE_GASR
  GVtMem* pVtMem = (GVtMem*) hTask ;
#else
  ZVtMem* pVtMem = (ZVtMem*) hTask ;
#endif
  
  return  pVtMem->Reset() ;
}

float r2_vt_getbestscore_eer(r2_vt_htask hTask, const float** pWavBuff, int iWavLen){
  
#ifdef USE_GASR
  return 0.0f ;
#else
  ZVtMem* pVtMem = (ZVtMem*) hTask ;
  return  pVtMem->GetBestScore(pWavBuff, iWavLen);
#endif
  
  
  
}

int r2_vt_set_dosecsl_cb(r2_vt_htask hTask, dosecsl_callback fun_cb, void* param_cb){
  
#ifdef USE_GASR
  GVtMem* pVtMem = (GVtMem*) hTask ;
#else
  ZVtMem* pVtMem = (ZVtMem*) hTask ;
#endif
  return pVtMem->SetDoSecSlCallback(fun_cb, param_cb);
}

int r2_vt_set_checkdirty_cb(r2_vt_htask hTask, checkdirty_callback fun_cb, void* param_cb){
  
#ifdef USE_GASR
  GVtMem* pVtMem = (GVtMem*) hTask ;
#else
  ZVtMem* pVtMem = (ZVtMem*) hTask ;
#endif
  return pVtMem->SetCheckDirtyCallback(fun_cb, param_cb);
  
}

int r2_vt_set_getdata_cb(r2_vt_htask hTask, getdata_callback fun_cb, void* param_cb){
  
#ifdef USE_GASR
  GVtMem* pVtMem = (GVtMem*) hTask ;
#else
  ZVtMem* pVtMem = (ZVtMem*) hTask ;
#endif
  return pVtMem->SetGetDataCallback(fun_cb, param_cb);
}


int r2vt_mem_print(){
  
  Z_PRINT_MEM_INFO();
  return 0 ;
}



