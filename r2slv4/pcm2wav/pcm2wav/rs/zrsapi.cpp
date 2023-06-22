//
//  zrsapi.cpp
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zrsapi.h"
#include "../zmath.h"
#include "zrssto.h"

using namespace __r2vt4__ ;



// aud opr
r2_rs_htask r2_rs_create(int iCn, int iSrIn, int iSrOut, int iFrmOut){
  
  ZRsMem* pRsMem = Z_SAFE_NEW(pRsMem, ZRsMem, iCn, iSrIn, iSrOut, iFrmOut);
  return (r2_rs_htask)pRsMem ;
  
}

int r2_rs_free(r2_rs_htask hTask){
  
  ZRsMem* pRsMem = (ZRsMem*) hTask ;
  Z_SAFE_DEL(pRsMem);
  
  return 0 ;
  
}

int r2_rs_process_float(r2_rs_htask hTask,const float** pWavIn, const int iLenIn, float** &pWavOut, int &iLenOut){
  
  ZRsMem* pRsMem = (ZRsMem*) hTask ;
  return  pRsMem->Process(pWavIn, iLenIn, pWavOut, iLenOut) ;
  
}


int r2_rs_reset(r2_rs_htask hTask){
  
  ZRsMem* pRsMem = (ZRsMem*) hTask ;
  
  return pRsMem->Reset() ;
}


