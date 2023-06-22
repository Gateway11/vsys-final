//
//  r2ad1api.cpp
//  r2ad
//
//  Created by hadoop on 10/22/15.
//  Copyright (c) 2015 hadoop. All rights reserved.
//

#include "r2ad1.h"
#include "r2sto1.h"
#include "r2mem1.h"

static r2sto1* g_pSto1 = NULL ;

/************************************************************************/
/** System Init	, Exit
 */
int r2ad1_sysinit(const char* pWorkDir){
  
  if (g_pSto1 == NULL) {
    g_pSto1 = R2_SAFE_NEW(g_pSto1, r2sto1,pWorkDir);
  }
  return  0 ;
}

int r2ad1_sysexit(){
  
  R2_SAFE_DEL(g_pSto1);
  
  return  0 ;
}

/************************************************************************/
/** Task Alloc , Free
 */
r2ad1_htask r2ad1_create(){
  
  r2mem1* pTask = R2_SAFE_NEW(pTask, r2mem1, g_pSto1);
  return (r2ad1_htask)pTask ;
}

int r2ad1_free(r2ad1_htask htask){
  
  r2mem1* pTask = (r2mem1*) htask ;
  R2_SAFE_DEL(pTask);
  return  0 ;
}

int r2ad1_reset(r2ad1_htask htask){
  
  r2mem1* pTask = (r2mem1*) htask ;
  return  pTask->reset();
}

/************************************************************************/
/** Main Procedure
 */
int r2ad1_putdata(r2ad1_htask htask, char* pData_In, int iLen_In){
  
  r2mem1* pTask = (r2mem1*) htask ;
  return  pTask->ProcessData(pData_In, iLen_In);
}

int r2ad1_putdata2(r2ad1_htask htask, char* pData_In, int iLen_In, char* &pData_Out, int &iLen_Out){
  
  
  r2mem1* pTask = (r2mem1*) htask ;
  return  pTask->ProcessData(pData_In, iLen_In, pData_Out, iLen_Out);
}

int r2ad1_getdatalen(r2ad1_htask htask){
  
  r2mem1* pTask = (r2mem1*) htask ;
  return  pTask->m_pMem_Buff->getdatalen();
}

int r2ad1_getdata(r2ad1_htask htask, char* pData_Out, int iLen_Out){
  
  r2mem1* pTask = (r2mem1*) htask ;
  return  pTask->m_pMem_Buff->getdata(pData_Out, iLen_Out);
}

int r2ad_mem_print(){
  
  R2_PRINT_MEM_INFO() ;
  return  0 ;
}
