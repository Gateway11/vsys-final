//
//  zvec.cpp
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zvec.h"
#include "../zmacro.h"

namespace __r2vt4__ {

  ZVec::ZVec(int size){
    
    this->size = size ;
    this->data = Z_SAFE_NEW_SSE_AR1(this->data,float,(size + 15) / 16 * 16);
    this->data_i = (int*)this->data ;
  }
  
  ZVec::~ZVec(void){
    
    Z_SAFE_DEL_SSE_AR1(data);
  }
  
  int ZVec::Clean(){
    
    memset(data,0, (size + 15) / 16 * 16 * sizeof(float) );
    return 0 ;
  }
  
  int ZVec::Print(const char* pLabel){
    
    printf("%s(%d)----------------------------------------------\n",pLabel,size);
    for (int i = 0 ; i < zmin(size,10) ; i ++) {
      printf("%0.10f ",data[i]);
    }
    printf("\r\n\r\n");
    return 0 ;
  }
  
  int ZVec::Copy(const ZVec* pVecX){
    
    assert(size == pVecX->size);
    
    int total = (size + 3) / 4 * 4;
    memcpy(data, pVecX->data, sizeof(float) * total);
    return 0 ;
  }
  
  
  int ZVec::Read(ZInput* pInput){
    
    assert(pInput->m_iInputType == ZIN_KALDI_BINERY
           || pInput->m_iInputType == ZIN_RAW_FORMAT);
    
    if (pInput->m_iInputType == ZIN_KALDI_BINERY) {
      
      pInput->ExpectToken("FV");
      int ss = 0;
      pInput->ReadBasicType(&ss);
      assert(ss == size);
      pInput->ReadArray(data, size);
      
    }else if(pInput->m_iInputType == ZIN_RAW_FORMAT){
      
      pInput->ReadArray(data, size);
      
    }else {
      assert(0);
    }
    
    return 0 ;
  }
  
  int ZVec::Write(ZOutput* pOutput){
    
    //assert(pOutput->m_iOutputType == ZOUT_KALDI_BINERY);
    
    pOutput->WriteToken("FV");
    pOutput->WriteBasicType(&size);
    pOutput->WriteArray(data, size);
    
    return 0 ;
  }
  
  int ZVec::CheckMaxSize(int iMaxSize){
    
    if (this->size < iMaxSize) {
      Z_SAFE_DEL_SSE_AR1(data);
      
      this->size = iMaxSize ;
      this->data = Z_SAFE_NEW_SSE_AR1(this->data,float,(size + 15) / 16 * 16);
      this->data_i = (int*)this->data ;
    }
    
    Clean() ;
    
    return  0 ;
  }
  
  int ZVec::Add(float a){
    
    for (int i = 0 ; i < size ; i ++) {
      data[i] += a ;
    }
    return  0 ;
    
  }
  
  int ZVec::Add_aX(ZVec* pVecX, float a){
    
    assert(pVecX->size == size);
    cblas_saxpy(size,a,pVecX->data,1,data,1);
    
    return 0 ;
  }
  
  int ZVec::Add_aAX(const ZMat* pMatA, const ZVec* pVecX,float a){
    
    assert(pMatA->row == size);
    assert(pMatA->col == pVecX->size);
    
    cblas_sgemv(CblasRowMajor,CblasNoTrans,pMatA->row,pMatA->col,a,pMatA->data[0],
                pMatA->col_inc,pVecX->data,1,1.0f,data,1);
    
    return 0 ;
  }
  
  int ZVec::Add_aATX(ZMat* pMatA, ZVec* pVecX,float a){
    
    assert(pMatA->col == size);
    assert(pMatA->row == pVecX->size);
    
    cblas_sgemv(CblasRowMajor,CblasTrans,pMatA->row,pMatA->col,a,pMatA->data[0],
                pMatA->col_inc,pVecX->data,1,1.0f,data,1);
    
    return 0 ;
  }
  
  int ZVec::Add_aAX_Sy(ZMat* pMatA, ZVec* pVecX,float a){
    
    assert(pMatA->row == size);
    assert(pMatA->col == pVecX->size);
    
    cblas_ssymv(CblasRowMajor,CblasUpper,pMatA->row,a,pMatA->data[0],
                pMatA->col_inc,pVecX->data,1,1.0f,data,1);
    
    return 0 ;
  }
  
  int ZVec::Add_aXY(ZVec* pVecX, ZVec* pVecY){
    
#if defined(__arm__)
    int N = (size + 3)/4 ;
    
    float* a = data ;
    float* b = pVecX->data ;
    float* c = pVecY->data ;
    
    __asm__ volatile
    (
     "mov r0, %0  \n\t"
     "mov r1, %1  \n\t"
     "mov r2, %2  \n\t"
     "mov r3, %3  \n\t"
     
     // Store m & v - avoiding q4-q7 which need to be preserved - q0 = result
     
     "LOOPXYLEFT:\n\t"
     
     "vldmia r0 , {q0}	\n\t"
     "vldmia r1!, {q1}  \n\t"	// q1 = v
     "vldmia r2!, {q2}	\n\t"	// q8-q11 = m
     
     "vmla.f32 q0, q1, q2 \n\t"
     
     "vstmia r0!, {q0}  \n\t"
     
     "SUBS r3,#1      \n\t"
     "BNE LOOPXYLEFT "
     
     // output = result registers
     
     
     : // no output
     : "r" (a), "r" (b), "r" (c), "r" (N) 	// input - note *value* of pointer doesn't change
     : "memory", "q0", "q1", "q2", "r0", "r1", "r2", "r3" //clobber
     );
    
#elif defined(__aarch64__)
    
    int N = (size + 3)/4 ;
    
    float* a = data ;
    float* b = pVecX->data ;
    float* c = pVecY->data ;
    
    __asm__ volatile
    (
     "mov x0, %0  \n\t"
     "mov x1, %1  \n\t"
     "mov x2, %2  \n\t"
     "mov x3, %3  \n\t"
     
     // Store m & v - avoiding q4-q7 which need to be preserved - q0 = result
     
     "LOOPXY:\n\t"
     
     //"vldmia r0 , {q0}	\n\t"
     "ld1 {v0.4s}, [x0] 	\n\t"
     //"vldmia r1!, {q1}  \n\t"	// q1 = v
     "ld1 {v1.4s}, [x1], #16 	\n\t"
     //"vldmia r2!, {q2}	\n\t"	// q8-q11 = m
     "ld1 {v2.4s}, [x2], #16 	\n\t"
     
     //"vmla.f32 q0, q1, q2 \n\t"
     "fmla v0.4s, v1.4s, v2.4s \n\t"
     
     //"vstmia r0!, {q0}  \n\t"
     "st1 {v0.4s}, [x0], #16 \n\t"
     
     "SUBS x3,x3,#1      \n\t"
     "BNE LOOPXY "
     
     // output = result registers
     
     
     : // no output
     : "r" (a), "r" (b), "r" (c), "r" (N) 	// input - note *value* of pointer doesn't change
     : "memory", "v0", "v1", "v2", "x0", "x1", "x2", "x3" //clobber
     );
#else
    
    for (int i = 0 ; i < size ; i ++) {
      data[i] += pVecX->data[i] * pVecY->data[i];
    }
    
#endif
    
    
    return  0 ;
  }
  
  int ZVec::RowActive(std::string acn){
    
    if(acn == "Relu"){
      return RowRelu() ;
    }else if(acn == "Tanh"){
      return RowTanh() ;
    }else if(acn == "Sigmoid"){
      return RowSigMoid() ;
    }else{
      assert(0) ;
      return 1 ;
    }
  }
  
  
  
  int ZVec::RowSigMoid(){
    
    for (int i = 0 ; i < size; i ++) {
      float &da = data[i] ;
#ifdef QUICK_EXP
      if (da < - QUICK_EXP_SHIELD){
        da = ZMat::m_plogtab2[0] ;
      }else if (da > QUICK_EXP_SHIELD){
        da = ZMat::m_plogtab2[QUICK_EXP_CONSTANT_2] ;
      }else{
        da = ZMat::m_plogtab2[(int)(da * QUICK_EXP_CONSTANT_3 + QUICK_EXP_CONSTANT_1)] ;
      }
#else
      dd = 1.0f /(1+exp(-dd));
#endif
    }
    return  0 ;
  }
  
  int ZVec::RowSoftMax(){
    
    float sum = data[0];
    for (int i = 1 ; i < size ; i ++){
#ifdef QUICK_EXP
      sum = QuickLogAdd(sum, data[i]);
#else
      sum = LogAdd1(sum, data[i]);
#endif
    }
    
    for (int i = 0 ; i < size ; i ++){
      data[i] = data[i] - sum ;
    }
    
    return 0 ;
  }
  
  int ZVec::RowTanh(){
    
    for (int i = 0 ; i < size ; i ++){
      float &da = data[i] ;
#ifdef QUICK_EXP
      if (da < - QUICK_EXP_SHIELD){
        da = ZMat::m_plogtab3[0] ;
      }else if (da > QUICK_EXP_SHIELD){
        da = ZMat::m_plogtab3[QUICK_EXP_CONSTANT_2] ;
      }else{
        da = ZMat::m_plogtab3[(int)(da * QUICK_EXP_CONSTANT_3 + QUICK_EXP_CONSTANT_1)] ;
      }
#else
      float x = exp(-da) * exp(-da) ;
      da = (1.0f - x) /(1.0f + x);
#endif
    }
    
    return 0 ;
  }
  
  int ZVec::RowRelu(){
    
    for (int i = 0 ; i < size ; i ++) {
      if (data[i] < 0) {
        data[i] = 0.0f ;
      }
    }
    return 0 ;
  }
  
  int ZVec::RowAdd(ZVec* pVecX){
    
    assert(pVecX->size == size) ;
    
    for (int i = 0 ; i < pVecX->size ; i ++) {
      data[i] += pVecX->data[i] ;
    }
    
    return  0 ;
    
  }
  
  int ZVec::RowMul(ZVec* pVecX){
    
    assert(pVecX->size == size);
    
    for (int i = 0 ; i < pVecX->size ; i ++) {
      data[i] *= pVecX->data[i] ;
    }
    
    return 0 ;
  }
  
  int ZVec::RowMul(float a){
    
    for (int i = 0 ; i < size ; i ++) {
      data[i] = data[i] * a ;
    }
    return 0 ;
  }
  
  
  int ZVec::RowSum2(ZMat* pMatA){
    
    
    
    for (int i = 0 ; i < size ; i ++) {
      for (int j = 0 ; j < pMatA->col ; j ++) {
        data[i] += pMatA->data[i][j] * pMatA->data[i][j] ;
      }
    }
    
    return 0 ;
  }
  
  float ZVec::Sum(){
    
    double sum = 0.0f ;
    for (int i = 0 ; i < size ; i ++) {
      sum += data[i] ;
    }
    return  sum ;
  }
  
  float ZVec::Dot(ZVec* pVecX) const{
    
    assert(pVecX->size == size);
    
    return cblas_sdot(pVecX->size,pVecX->data,1,data,1);
  }
  
  float ZVec::Dot(float* pDataX) const{
    
    float res = 0.0f ;
    for (int i = 0 ; i < size ; i ++) {
      res += data[i] * pDataX[i] ;
    }
    return res ;
  }
  
  
  int ZVec::MaxId(){
    
    int iMaxId = 0 ;
    for (int i = 1 ; i < size ; i ++) {
      if (data[i] > data[iMaxId]) {
        iMaxId = i ;
      }
    }
    return iMaxId ;
  }
  
  int ZVec::Add_aAX_NENO(const ZMat_Neon* pMatA, const ZVec* pVecX){
    
    if (pMatA->m_bTrans) {
      assert(pMatA->m_iRow == pVecX->size);
      assert(pMatA->m_iCol == this->size);
    }else{
      assert(pMatA->m_iCol == pVecX->size);
      assert(pMatA->m_iRow == this->size);
    }
    
#ifdef __USE_MAT_NEON__
    int M = pMatA->m_iRow_Neon;
    int K = pMatA->m_iCol_Neon;
    
#if defined(__arm__)
    
    ///////////////////////////////////////////////////////////////////
    
    for (int m = 0 ; m < M ; m ++) {
      
      float * a = pMatA->m_pData_Neon[m * pMatA->m_iCol_Neon] ;
      float * b = pVecX->data;
      float * c = data + m * 4 ;
      
      __asm__ volatile
      (
       "mov r0, %0  \n\t"
       "mov r1, %1  \n\t"
       "mov r2, %2  \n\t"
       "mov r3, %3  \n\t"
       
       // Store m & v - avoiding q4-q7 which need to be preserved - q0 = result
       "vldmia r0, { q12 }\n\t"
       
       "LOOPAX1:\n\t"
       
       "vldmia r2!, { q0 }		\n\t"	// q1 = v
       "vldmia r1!, { q8-q11 }	\n\t"	// q8-q11 = m
       "pld [r1, #0]    \n\t"
       "vmla.f32 q12, q8, d0[0]\n\t"
       "vmla.f32 q12, q9, d0[1]\n\t"
       "vmla.f32 q12, q10, d1[0]\n\t"
       "vmla.f32 q12, q11, d1[1]\n\t"
       
       "SUBS r3,#1   \n\t"
       "BNE LOOPAX1 \n\t"
       
       // output = result registers
       "vstmia r0, { q12 }"
       
       : // no output
       : "r" (c), "r" (a), "r" (b), "r" (K) 	// input - note *value* of pointer doesn't change
       : "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "r0", "r1", "r2", "r3" //clobber
       );
      
      
    }
    
#elif defined( __aarch64__) && __ANDROID_API__ <= 26
    
    for (int m = 0 ; m < M ; m ++) {
      
      float * a = pMatA->m_pData_Neon[m * pMatA->m_iCol_Neon] ;
      float * b = pVecX->data;
      float * c = data + m * 4 ;
      
      
      __asm__ volatile
      (
       "mov x0, %0  \n\t"
       "mov x1, %1  \n\t"
       "mov x2, %2  \n\t"
       "mov x3, %3  \n\t"
       
       //"vldmia r0, { q12 }\n\t"
       "ld1 { v12.4s }, [x0]\n\t"
       
       "LOOPAX1:\n\t"
       
       //"vldmia r2!, { q0 }		\n\t"	// q1 = v
       "ld1 { v0.4s }, [x2], #16	\n\t"	// q1 = v
       //"vldmia r1!, { q8-q11 }	\n\t"	// q8-q11 = m
       "ld1 { v8.4s-v11.4s }, [x1], #64		\n\t"	// q1 = v
       
       //TODO: need check review
       //"pld [x1, #0]    \n\t"
       "prfm  pldl1keep, [x1, #0]\n\t"
       
       //"vmla.f32 q12, q8, d0[0]\n\t"
       "fmla v12.4s, v8.4s,  v0.s[0]\n\t"
       //"vmla.f32 q12, q9, d0[1]\n\t"
       "fmla v12.4s, v9.4s,  v0.s[1]\n\t"
       //"vmla.f32 q12, q10, d1[0]\n\t"
       "fmla v12.4s, v10.4s, v0.s[2]\n\t"
       //"vmla.f32 q12, q11, d1[1]\n\t"
       "fmla v12.4s, v11.4s, v0.s[3]\n\t"
       
       "SUBS x3,x3,#1        \n\t"
       "BNE LOOPAX1       \n\t"
       
       // output = result registers
       //"vstmia r0, { q12 }"
       "st1 { v12.4s }, [x0]"
       
       : // no output
       : "r" (c), "r" (a), "r" (b), "r" (K) 	// input - note *value* of pointer doesn't change
       : "memory", "v0", "v1", "v2", "v3", "v8", "v9", "v10", "v11", "v12", "x0", "x1", "x2", "x3" //clobber
       );
      
    }
    
#else
    for (int m = 0 ; m < M ; m ++) {
      for (int k = 0 ; k < K ; k ++) {
        cblas_sgemv(CblasColMajor,CblasNoTrans,4,4,1.0,pMatA->m_pData_Neon[m * pMatA->m_iCol_Neon + k],
                    4,pVecX->data +  k * 4,1,1.0f,data + m * 4,1);
      }
    }
    
#endif
    
    return  0 ;
    
#else
    
    return Add_aAX(pMatA->m_pMat, pVecX);
#endif
    
  }
  
};




