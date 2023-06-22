//
//  zmat.h
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zmat__
#define __r2vt4__zmat__

#include "../zmath.h"
#include "../io/zinput.h"
#include "../io/zoutput.h"

#include "zvec.h"

namespace __r2vt4__ {
  
#define QUICK_EXP
#define QUICK_EXP_CONTEXT 16
#define QUICK_EXP_SHIELD  15.9f
  
#define QUICK_EXP_CONSTANT_1 16000
#define QUICK_EXP_CONSTANT_2 32000
  
#define QUICK_EXP_CONSTANT_3 1000.0f
#define QUICK_EXP_CONSTANT_4 0.001f
  
  
  class ZVec ;
  class ZMat
  {
  public:
    ZMat(int row, int col);
  public:
    ~ZMat(void);
    
  public:
    int Clean();
    int Print(const char* pLabel);
    int Print_I(const char* pLabel);
    int Copy(ZVec** pVecLst, int num);
    int Copy(const ZMat* pMatA);
    
    
  public:
    int Read(ZInput* pInput);
    int Write(ZOutput* pOutput);
    
    int Read_Sy(ZInput* pInput);
    int Write_Sy(ZOutput* pOutput);
    
    int CheckMaxSize(int iMaxSize);
    
  public:
    //add aI
    int Add(float a);
    //add a*A
    int Add_aA(const ZMat* pMatA, float a=1.0f);
    //add a*X*YT
    int Add_aXYT(const ZVec* pVecX, const ZVec* pVecY, float a=1.0f);
    //add a*A*B
    int Add_aAB(const ZMat* pMatA, const ZMat* pMatB, float a=1.0f);
    //add a*AT*B
    int Add_aATB(const ZMat* pMatA, const ZMat* pMatB, float a=1.0f);
    //add a*A*BT
    int Add_aABT(const ZMat* pMatA, const ZMat* pMatB, float a=1.0f);
    
  public:
    int RowMul(float a);
    int RowMul(ZVec* pVecX);
    
  public:
    int RowCopy(const ZVec* pVec);
    int RowSigMoid();
    int RowSoftMax();
    int RowTanh();
    int RowRelu();
    int RowAdd(const ZVec* pVec);
    
  public:
    int RowCopy_4(const ZVec* pVec);
    int RowCopy_N(const ZMat* pVec4);
    
  public:
    int ResetRow(int iRow);
    
  public:
    int RandInit();
    int FixInit() ;
    
  public:
    int row ;
    int col ;
    int col_inc ;
    float** data ;
    int** data_i ;
    
  public:
#ifdef QUICK_EXP
    static float* m_plogtab1;
    static float* m_plogtab2;
    static float* m_plogtab3;
    static int InitQuickExp();
    static int ExitQuickExp();
    //static float QuickLogAdd(float x, float y);
#endif
    
    
  };

};


#endif /* __r2vt4__zmat__ */
