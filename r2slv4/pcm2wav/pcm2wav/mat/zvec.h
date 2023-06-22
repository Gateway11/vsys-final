//
//  zvec.h
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zvec__
#define __r2vt4__zvec__

#include "../zmath.h"
#include "../io/zinput.h"
#include "../io/zoutput.h"

#include "zmat.h"
#include "zmat_neon.h"

namespace __r2vt4__ {

  class ZMat ;
  class ZMat_Neon ;
  
  class ZVec
  {
  public:
    ZVec(int size);
  public:
    ~ZVec(void);
    
  public:
    int Clean();
    int Print(const char* pLabel);
    int Copy(const ZVec* pVecX);
    
  public:
    int Read(ZInput* pInput);
    int Write(ZOutput* pOutput);
    
    int CheckMaxSize(int iMaxSize);
    
  public:
    int Add(float a);
    int Add_aX(ZVec* pVecX, float a = 1.0f);
    int Add_aAX(const ZMat* pMatA,const ZVec* pVecX,float a = 1.0f);
    int Add_aATX(ZMat* pMatA, ZVec* pVecX,float a = 1.0f);
    int Add_aAX_Sy(ZMat* pMatA, ZVec* pVecX,float a = 1.0f);
    
    int Add_aXY(ZVec* pVecX, ZVec* pVecY);
    
  public:
    
    int RowActive(std::string acn);
    int RowSigMoid();
    int RowSoftMax();
    int RowTanh();
    int RowRelu();
    int RowAdd(ZVec* pVecX);
    
    
    
  public:
    int RowMul(ZVec* pVecX);
    int RowMul(float a);
    int RowSum2(ZMat* pMatA) ;
    
  public:
    float Sum();
    float Dot(ZVec* pVecX) const;
    float Dot(float* pDataX) const ;
    int MaxId();
    
  public:
    int Add_aAX_NENO(const ZMat_Neon* pMatA, const ZVec* pVecX) ;
    
  public:
    int size ;
    float* data ;
    int * data_i ;
    
    
  };
};


#endif /* __r2vt4__zvec__ */
