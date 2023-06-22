//
//  r2mem3.h
//  r2ad2
//
//  Created by hadoop on 8/4/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

#ifndef __r2ad2__r2mem3__
#define __r2ad2__r2mem3__


#include "r2sto3.h"
#include "../r2mem_bf.h"
#include "../r2mem_agc.h"
#include "../r2mem_i.h"
#include "../r2mem_o.h"
#include "../r2mem_buff.h"

class r2mem3
{
public:
  r2mem3(r2sto3* pSto);
public:
  ~r2mem3(void);
  
  int ProcessData(char* pData_In, int iLen_In, char*& pData_Out, int &iLen_Out);
  int ProcessData(char* pData_In, int iLen_In);
  int reset();
  r2_mic_info* geterrorinfo(float** pData_Mul, int iLen_Mul);
public:
  
  r2sto3* m_pSto ;
  
  //i
  r2mem_i* m_pMem_In ;
  
  //bf
  r2mem_bf* m_pMem_Bf ;
  
  //agc
  r2mem_agc* m_pMem_Agc ;
  
  //out
  r2mem_o* m_pMem_Out ;
  
  //buff
  r2mem_buff* m_pMem_Buff ;
  
  bool m_bFirstFrm ;

};


#endif /* __r2ad2__r2mem3__ */
