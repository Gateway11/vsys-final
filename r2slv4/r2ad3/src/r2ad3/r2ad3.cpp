//
//  r2ad3.cpp
//  r2ad2
//
//  Created by hadoop on 8/4/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

#include "r2ad3.h"

#include "r2sto3.h"
#include "r2mem3.h"

static r2sto3* g_pSto3 = NULL ;

/************************************************************************/
/** System Init	, Exit
 */
int r2ad3_sysinit(const char* pWorkDir){
  
  if (g_pSto3 == NULL) {
    g_pSto3 = R2_SAFE_NEW(g_pSto3, r2sto3,pWorkDir);
  }
  return 0 ;
}

int r2ad3_sysexit(){
  
  R2_SAFE_DEL(g_pSto3);
  
  
  return 0 ;
}

/************************************************************************/
/** Task Alloc , Free
 */
r2ad3_htask r2ad3_create(){
  
  r2mem3* pTask = R2_SAFE_NEW(pTask, r2mem3, g_pSto3);
  return (r2ad3_htask)pTask ;
  
}

int r2ad3_free(r2ad3_htask htask){
  
  r2mem3* pTask = (r2mem3*) htask ;
  R2_SAFE_DEL(pTask);
  return  0 ;
  
}

// reset
int r2ad3_reset(r2ad3_htask htask){
  
  r2mem3* pTask = (r2mem3*) htask ;
  return  pTask->reset();
  
}

/************************************************************************/
/** Main Procedure
 */
int r2ad3_putdata(r2ad3_htask htask, char* pData_In, int iLen_In){
  
  r2mem3* pTask = (r2mem3*) htask ;
  return  pTask->ProcessData(pData_In, iLen_In);
  
}

int r2ad3_putdata2(r2ad3_htask htask, char* pData_In, int iLen_In, char* &pData_Out, int &iLen_Out){
  
  r2mem3* pTask = (r2mem3*) htask ;
  return  pTask->ProcessData(pData_In, iLen_In, pData_Out, iLen_Out);
  
}

int r2ad3_getdatalen(r2ad3_htask htask){
  
  r2mem3* pTask = (r2mem3*) htask ;
  return  pTask->m_pMem_Buff->getdatalen();
  
}

int r2ad3_getdata(r2ad3_htask htask, char* pData_Out, int iLen_Out){
  
  r2mem3* pTask = (r2mem3*) htask ;
  return  pTask->m_pMem_Buff->getdata(pData_Out, iLen_Out);
  
}

