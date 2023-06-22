//
//  zmat.cpp
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zmat.h"
#include "../zmath.h"

namespace __r2vt4__ {

#ifdef QUICK_EXP
  float* ZMat::m_plogtab1 = NULL ;
  float* ZMat::m_plogtab2 = NULL ;
  float* ZMat::m_plogtab3 = NULL ;
  
  int ZMat::InitQuickExp(){
    
    if (m_plogtab1 == NULL){
      m_plogtab1 = Z_SAFE_NEW_AR1(m_plogtab1, float, QUICK_EXP_CONSTANT_2 + 1);
      m_plogtab2 = Z_SAFE_NEW_AR1(m_plogtab2, float, QUICK_EXP_CONSTANT_2 + 1);
      m_plogtab3 = Z_SAFE_NEW_AR1(m_plogtab3, float, QUICK_EXP_CONSTANT_2 + 1);
      
      for(int k = - QUICK_EXP_CONSTANT_1 ; k <= QUICK_EXP_CONSTANT_1; k ++) {
        float x = exp(-k * QUICK_EXP_CONSTANT_4) ;
        m_plogtab1[k + QUICK_EXP_CONSTANT_1] = log(1.0 + x);
        m_plogtab2[k + QUICK_EXP_CONSTANT_1] = 1.0f /(1.0 + x);
        m_plogtab3[k + QUICK_EXP_CONSTANT_1] = (1.0f - x*x) / (1.0f + x*x);
      }
    }
    return 0 ;
  }
  
  int ZMat::ExitQuickExp(){

    Z_SAFE_DEL_AR1(m_plogtab1);
    Z_SAFE_DEL_AR1(m_plogtab2);
    Z_SAFE_DEL_AR1(m_plogtab3);
    return  0 ;
  }
  
  float ZMat::QuickLogAdd(float x, float y){
    
    float b ;
    if (x < y)         /*** exchange if x<y ***/
      b=x, x=y, y=b;
    b = x-y;
    if (b > QUICK_EXP_SHIELD)     /** keep up to 16 decimal places of precision **/
      return(x);
    else
      return x + m_plogtab1[(int)(b*QUICK_EXP_CONSTANT_3 + QUICK_EXP_CONSTANT_1) ];
  }
#endif
  
  
  
  
  ZMat::ZMat(int row, int col){
    
    this->col = col ;
    this->row = row ;
    this->col_inc = (col + 3) / 4 * 4 ;
    this->data = Z_SAFE_NEW_SSE_AR2(this->data, float,this->row,this->col_inc);
    this->data_i = (int**)this->data ;
    
  }
  
  ZMat::~ZMat(void){
    
    Z_SAFE_DEL_SSE_AR2(data);
  }
  
  int ZMat::Clean(){
    
    for (int i = 0 ; i < row ; i ++) {
      memset(data[i],0,sizeof(float)* col_inc);
    }
    
    return 0 ;
  }
  
  int ZMat::Print(const char* pLabel){
    
    int ii = zmin(row, 5) ;
    int jj = zmin(col, 5) ;
    
    double** pScore = Z_SAFE_NEW_AR2(pScore, double, ii, jj) ;
    
    for (int i = 0 ; i < row;  i ++) {
      for (int j = 0 ; j < col ; j ++) {
        pScore[i % ii][j % jj] += fabs(data[i][j]) ;
      }
    }
    
    printf("--------------%s(%d,%d)----------------------------------------------\n",pLabel,row,col);
    for (int i = 0 ; i < ii ; i ++) {
      for (int j = 0 ; j < jj ; j ++) {
        printf("%f ",pScore[i][j] * ii * jj / row / col );
      }
      printf("\r\n");
    }
    printf("\r\n");
    
    Z_SAFE_DEL_AR2(pScore) ;
    
    return  0 ;
  }
  
  int ZMat::Print_I(const char* pLabel){
    
    
    for (int i = 0 ; i < row ; i ++) {
      for (int j = 0 ; j < col ; j ++) {
        printf("%04d ",data_i[i][j]);
      }
      printf("\r\n");
    }
    printf("\r\n");
    
    return 0 ;
    
  }
  
  int ZMat::Copy(ZVec** pVecLst, int num){
    
    assert(row == num);
    
    for (int i = 0 ; i < row ; i ++) {
      assert(pVecLst[i]->size == col);
      for (int j = 0 ; j < col ; j ++) {
        data[i][j] = pVecLst[i]->data[j];
      }
    }
    
    return  0 ;
  }
  
  int ZMat::Copy(const ZMat* pMatA){
    
    assert(row == pMatA->row);
    assert(col == pMatA->col);
    
    for (int i = 0 ; i < row ; i ++) {
      memcpy(data[i], pMatA->data[i], sizeof(float) * col);
    }
    
    return 0 ;
  }
  
  
  
  int ZMat::Read(ZInput* pInput){
    
    assert(pInput->m_iInputType == ZIN_KALDI_BINERY
           || pInput->m_iInputType == ZIN_RAW_FORMAT);
    
    
    if (pInput->m_iInputType == ZIN_KALDI_BINERY) {
      pInput->ExpectToken("FM");
      
      int row,col;
      pInput->ReadBasicType(&row);
      pInput->ReadBasicType(&col);
      assert(this->row == row);
      assert(this->col == col);
      
      for (int i = 0 ; i < row ; i ++) {
        pInput->ReadArray(data[i], col);
      }
    }else if(pInput->m_iInputType == ZIN_RAW_FORMAT){
      
      for (int i = 0 ; i < row ; i ++) {
        pInput->ReadArray(data[i], col);
      }
      
    }else {
      assert(0);
    }
    
    
    return 0 ;
  }
  
  int ZMat::Write(ZOutput* pOutput){
    
    assert(pOutput->m_iOutputType == ZOUT_KALDI_BINERY
           || pOutput->m_iOutputType == ZOUT_RAW_FORMAT);
    
    
    if (pOutput->m_iOutputType == ZOUT_KALDI_BINERY) {
      pOutput->WriteToken("FM");
      pOutput->WriteBasicType(&row);
      pOutput->WriteBasicType(&col);
      
      for (int i = 0 ; i < row ; i ++) {
        pOutput->WriteArray(data[i], col);
      }
    }else if(pOutput->m_iOutputType == ZOUT_RAW_FORMAT){
      for (int i = 0 ; i < row ; i ++) {
        pOutput->WriteArray(data[i], col);
      }
    }else{
      assert(0) ;
    }
    
    return 0 ;
  }
  
  int ZMat::Read_Sy(ZInput* pInput){
    
    assert(pInput->m_iInputType == ZIN_KALDI_BINERY);
    assert(row == col);
    
    int size = 0 ;
    pInput->ExpectToken("FP");
    pInput->ReadBasicType(&size);
    assert(row == size);
    for (int i = 0 ; i < row ; i ++) {
      pInput->ReadArray(data[i], i + 1);
    }
    
    for (int i = 0 ; i < row ; i ++) {
      for (int j = 0 ; j < i ; j ++) {
        data[j][i] = data[i][j];
      }
    }
    
    return 0 ;
  }
  
  int ZMat::Write_Sy(ZOutput* pOutput){
    
    assert(pOutput->m_iOutputType == ZOUT_KALDI_BINERY);
    
    assert(row == col);
    
    pOutput->WriteToken("FP");
    int size = row ;
    pOutput->WriteBasicType(&size);
    
    for (int i = 0 ; i < row ; i ++) {
      pOutput->WriteArray(data[i], i + 1);
    }
    return 0 ;
    
  }
  
  int ZMat::CheckMaxSize(int iMaxSize){
    
    if (this->col < iMaxSize) {
      
      Z_SAFE_DEL_SSE_AR2(data);
 
      this->col = iMaxSize ;
      this->col_inc = (col + 3) / 4 * 4 ;
      this->data = Z_SAFE_NEW_SSE_AR2(this->data, float,this->row,this->col_inc);
      this->data_i = (int**)this->data ;
 
    }
    
    Clean() ;
    
    return  0 ;
    
  }
  
  //add aI
  int ZMat::Add(float a){
    
    assert(row == col);
    
    for (int i = 0 ; i < row ; i ++) {
      data[i][i] += a ;
    }
    return 0 ;
  }
  
  //add a*A
  int ZMat::Add_aA(const ZMat* pMatA, float a){
    
    assert(pMatA->col == col);
    assert(pMatA->row == row);
    
    for (int i = 0 ; i < pMatA->row ; i ++) {
      cblas_saxpy(pMatA->col,a,pMatA->data[i],1,data[i],1);
    }
    
    return 0 ;
  }
  
  //add a*X*YT
  int ZMat::Add_aXYT(const ZVec* pVecX, const ZVec* pVecY, float a){
    
    assert(pVecX->size == row);
    assert(pVecY->size == col);
    
    cblas_sger(CblasRowMajor,pVecX->size,pVecY->size,a,
               pVecX->data,1,pVecY->data,1,data[0],col_inc);
    
    return 0 ;
  }
  
  //add a*A*B
  int ZMat::Add_aAB(const ZMat* pMatA, const ZMat* pMatB, float a){
    
    assert(pMatA->row == row);
    assert(pMatA->col == pMatB->row);
    assert(pMatB->col == col);
    
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,pMatA->row,pMatB->col,pMatA->col
                ,a,pMatA->data[0],pMatA->col_inc,pMatB->data[0],pMatB->col_inc,1.0f,
                data[0],col_inc);
    
    return 0 ;
  }
  
  //add a*AT*B
  int ZMat::Add_aATB(const ZMat* pMatA, const ZMat* pMatB, float a){
    
    assert(pMatA->col == row);
    assert(pMatA->row == pMatB->row);
    assert(pMatB->col == col);
    
    cblas_sgemm(CblasRowMajor,CblasTrans,CblasNoTrans,pMatA->col,pMatB->col,pMatA->row
                ,a,pMatA->data[0],pMatA->col_inc,pMatB->data[0],pMatB->col_inc,1.0f,
                data[0],col_inc);
    
    return 0 ;
  }
  
  //add a*A*BT
  int ZMat::Add_aABT(const ZMat* pMatA, const ZMat* pMatB, float a){
    
    assert(pMatA->row == row);
    assert(pMatA->col == pMatB->col);
    assert(pMatB->row == col);
    
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasTrans,pMatA->row,pMatB->row,pMatA->col
                ,a,pMatA->data[0],pMatA->col_inc,pMatB->data[0],pMatB->col_inc,1.0f,
                data[0],col_inc);
    
    return 0 ;
  }
  
  int ZMat::RowMul(float a){
    
    for (int i = 0 ; i < row ; i ++) {
      for (int j = 0 ; j < col ; j ++) {
        data[i][j] = data[i][j] * a ;
      }
    }
    
    return  0 ;
  }
  
  int ZMat::RowMul(ZVec* pVecX){
    
    assert(col == pVecX->size);
    
    for (int i = 0 ; i < row ; i ++) {
      for (int j = 0 ; j < col ; j ++) {
        data[i][j] = data[i][j] * pVecX->data[j] ;
      }
    }
    
    return  0 ;
    
  }
  
  int ZMat::RowCopy(const ZVec* pVec){
    
    assert(col == pVec->size);
    
    for (int i = 0 ; i < row ; i ++) {
      memcpy(data[i], pVec->data, sizeof(float) * col);
    }
    
    return  0 ;
  }
  
  int ZMat::RowSigMoid(){
    
#ifdef QUICK_EXP
    assert(m_plogtab1 != NULL);
    assert(m_plogtab2 != NULL);
    assert(m_plogtab3 != NULL);
#endif
    
    
    for(int i = 0; i< row ; i++){
      for (int j = 0 ; j < col ; j ++){
#ifdef QUICK_EXP
        if (data[i][j] < - QUICK_EXP_SHIELD){
          data[i][j] = m_plogtab2[0] ;
        }else if (data[i][j] > QUICK_EXP_SHIELD){
          data[i][j] = m_plogtab2[QUICK_EXP_CONSTANT_2] ;
        }else{
          data[i][j] = m_plogtab2[(int)(data[i][j] * QUICK_EXP_CONSTANT_3 + QUICK_EXP_CONSTANT_1)] ;
        }
#else
        data[i][j] = 1.0f /(1+exp(- data[i][j]));
#endif
        
      }
    }
    return  0 ;
  }
  
  int ZMat::RowSoftMax(){
    
#ifdef QUICK_EXP
    assert(m_plogtab1 != NULL);
    assert(m_plogtab2 != NULL);
    assert(m_plogtab3 != NULL);
#endif
    
    for (int i = 0 ; i < row ; i ++){
      float sum = data[i][0];
      for (int j = 1 ; j < col ; j ++){
#ifdef QUICK_EXP
        sum = QuickLogAdd(sum, data[i][j]);
#else
        sum = LogAdd1(sum, data[i][j]);
#endif
      }
      
      for (int j = 0 ; j < col ; j ++){
        data[i][j] = data[i][j] - sum ;
      }
    }
    return  0 ;
  }
  
  int ZMat::RowTanh(){
    
#ifdef QUICK_EXP
    assert(m_plogtab1 != NULL);
    assert(m_plogtab2 != NULL);
    assert(m_plogtab3 != NULL);
#endif
    
    
    for(int i = 0; i< row ; i++){
      for (int j = 0 ; j < col ; j ++){
#ifdef QUICK_EXP
        if (data[i][j] < - QUICK_EXP_SHIELD){
          data[i][j] = m_plogtab3[0] ;
        }else if (data[i][j] > QUICK_EXP_SHIELD){
          data[i][j] = m_plogtab3[QUICK_EXP_CONSTANT_2] ;
        }else{
          data[i][j] = m_plogtab3[(int)(data[i][j] * QUICK_EXP_CONSTANT_3 + QUICK_EXP_CONSTANT_1)] ;
        }
#else
        float x = exp(-data[i][j]) * exp(-data[i][j]) ;
        data[i][j] = (1.0f - x) /(1.0f + x);
#endif
      }
    }
    
    return 0 ;
  }
  
  int ZMat::RowRelu(){
    
    
    for(int i = 0; i< row ; i++){
      for (int j = 0 ; j < col ; j ++){
        if (data[i][j] < 0) {
          data[i][j] = 0 ;
        }
      }
    }
    return 0 ;
  }
  
  int ZMat::RowAdd(const ZVec* pVec){
    
    assert(col == pVec->size);
    
    for (int i = 0 ; i < row ; i ++) {
      for (int j = 0 ; j < col ; j ++) {
        data[i][j] += pVec->data[j];
      }
    }
    
    return  0 ;
  }
  
  
  int ZMat::RowCopy_4(const ZVec* pVec){
    
    assert(row == 4);
    assert(col == pVec->size) ;
    
    for (int i = 0 ; i < 4 ; i ++) {
      memcpy(data[i], pVec->data, sizeof(float) * col) ;
    }
    
    return  0 ;
  }
  
  int ZMat::RowCopy_N(const ZMat* pVec4){
    
    assert(pVec4->row == 4 );
    assert(col == pVec4->col) ;
    
    for (int i = 0 ; i < row ; i ++) {
      memcpy(data[i], pVec4->data[0], sizeof(float) * col) ;
    }
    
    return 0 ;
    
    
  }
  
  int ZMat::ResetRow(int iRow){
    
    row = iRow ;
    
    return  0 ;
  }
  
  int ZMat::RandInit(){
    
    for (int i = 0 ; i < row ; i ++) {
      for (int j = 0 ; j < col ; j ++) {
        data[i][j] = z_randuniform() ;
      }
    }
    return  0 ;
  }
  
  int ZMat::FixInit(){
    
    for (int i = 0 ; i < row ; i ++) {
      for (int j = 0 ; j < col ; j ++) {
        data[i][j] = i + j ;
      }
    }
    return  0 ;
  }
  
};




