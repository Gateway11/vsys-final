//
//  r2mem1.h
//  r2ad
//
//  Created by hadoop on 10/22/15.
//  Copyright (c) 2015 hadoop. All rights reserved.
//

#ifndef __r2ad__r2mem1__
#define __r2ad__r2mem1__

#include "r2ad1.h"
#include "r2sto1.h"

#include "../r2math.h"
#include "../r2mem_i.h"
#include "../r2mem_rdc.h"
#include "../r2mem_aec.h"
#include "../r2mem_o.h"
#include "../r2mem_buff.h"
#include "../r2mem_rs2.h"

#ifndef __ARM_ARCH_ARM__
#define _RAW_AUDIO
#define _RS_AUDIO
#define _AEC_AUDIO
#else
//#define _RAW_AUDIO
//#define _RS_AUDIO
//#define _AEC_AUDIO
#endif

class r2mem1
{
public:
  r2mem1(r2sto1* pSto);
public:
  ~r2mem1(void);
  
  int ProcessData(char* pData_In, int iLen_In, char*& pData_Out, int &iLen_Out);
  int ProcessData(char* pData_In, int iLen_In);
  int reset();
  
public:
  
  r2sto1* m_pSto ;
  
  //in
  r2mem_i* m_pMem_In ;
  
  //rs
  //r2mem_rs* m_pMem_Rs ;
  r2mem_rs2* m_pMem_Rs ;
  
  //rdc
  r2mem_rdc* m_pMem_RDc ;
  
  //aec
  r2mem_aec* m_pMem_Aec ;
  
  //out
  r2mem_o* m_pMem_Out ;
  
  //buff
  r2mem_buff* m_pMem_Buff ;
  
  //debug
  std::string m_strDebugFolder ;
  
  //fix mic
  bool m_bFirstFrm ;
  
#ifdef __ARM_ARCH_ARM__
  bool m_bFirstData ;
#endif
  
#ifdef _RAW_AUDIO
  FILE* m_pRawFile ;
  int m_iRecordLen_Max ;
  int m_iRecordLen_Cur ;
#endif
  
#ifdef _RS_AUDIO
  FILE** m_pRsFile ;
#endif
  
#ifdef _AEC_AUDIO
  FILE** m_pAecFile ;
#endif
  
  
};

#endif /* defined(__r2ad__r2mem1__) */
