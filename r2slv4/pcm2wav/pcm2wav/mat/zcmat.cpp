//
//  zcmat.cpp
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zcmat.h"

namespace __r2vt4__ {

  ZCMat::ZCMat(int row, int col){
    
    this->col = col ;
    this->row = row ;
    this->col_inc = (col + 3) / 4 * 4 ;
    this->data = Z_SAFE_NEW_SSE_AR2(this->data, zcomplex, this->row, this->col_inc);
  }
  
  ZCMat::~ZCMat(void){
    
    Z_SAFE_DEL_SSE_AR2(data);
  }
  
  int ZCMat::Clean(){
    
    for (int i = 0 ; i < row ; i ++) {
      memset(data[i], 0, sizeof(zcomplex) * col_inc * 4);
    }
    return  0 ;
  }
  
  
  int ZCMat::Print(const char* pLabel){
    
    printf("--------------%s(%d,%d)----------------------------------------------\n",pLabel,row,col);
    for (int i = 0 ; i < zmin(row,5) ; i ++) {
      for (int j = 0 ; j < zmin(col,5) ; j ++) {
        printf("(%d:%f,%f) ",j,data[i][j][0],data[i][j][1]);
      }
      printf("\r\n");
    }
    printf("\r\n");
    
    return 0 ;
  }
  
  
  //pCMatA[pMatB->data_i[i][0]] .* pCMatA[pMatB->data_i[i][1]] -> pCMatC[i]
  int ZCMat::CalcGcc(ZCMat* pCMatA, ZMat* pMatB, ZCMat* pCMatC){
    
    assert(pMatB->row == pCMatC->row);
    assert(pMatB->col == 2) ;
    assert(pCMatA->col == pCMatC->col);
    assert(pMatB->row == pCMatA->row * (pCMatA->row - 1) / 2) ;
    
    for (int i = 0 ; i < pMatB->row ; i ++) {
      
#if defined(__arm__)
      zcomplex* & a = pCMatA->data[pMatB->data_i[i][0]] ;
      zcomplex* & b = pCMatA->data[pMatB->data_i[i][1]] ;
      zcomplex* & c = pCMatC->data[i] ;
      int N = (pCMatA->col / 2 + 1 + 3) / 4 ;
      
      __asm__ volatile
      (
       "mov r0, %0  \n\t"
       "mov r1, %1  \n\t"
       "mov r2, %2  \n\t"
       "mov r3, %3  \n\t"
       
       "loop:\n\t"
       
       //mul
       "vld2.32     {q0-q1}, [r1]!  \n\t"
       "vld2.32     {q2-q3}, [r2]!  \n\t"
       
       "vmul.f32    q8, q0, q2      \n\t"
       "vmul.f32    q9, q1, q3      \n\t"
       "vmul.f32    q10, q0, q3     \n\t"
       "vmul.f32    q11, q2, q1     \n\t"
       
       //real and img
       "vadd.f32    q12, q8, q9     \n\t"
       "vsub.f32    q13, q11, q10   \n\t"
       
       //module
       "vmul.f32    q8, q12, q12      \n\t"
       "vmla.f32    q8, q13, q13      \n\t"
       
       //sqrt+div
       //       //fast invsqrt approx
       //       "vmov.f32      d1, d0					\n\t"	//d1 = d0
       //       "vrsqrte.f32 	d0, d0					\n\t"	//d0 = ~ 1.0 / sqrt(d0)
       //       "vmul.f32      d2, d0, d1				\n\t"	//d3 = d0 * d2
       //       "vrsqrts.f32 	d3, d2, d0				\n\t"	//d4 = (3 - d0 * d3) / 2
       //       "vmul.f32      d0, d0, d3				\n\t"	//d0 = d0 * d4
       //       "vmul.f32      d2, d0, d1				\n\t"	//d3 = d0 * d2
       //       "vrsqrts.f32 	d3, d2, d0				\n\t"	//d4 = (3 - d0 * d3) / 2
       //       "vmul.f32      d0, d0, d3				\n\t"	//d0 = d0 * d4
       
       //fast invsqrt approx
       //d0=q8 d1=q9 d2=q10 d3=q11
       "vmov.f32      q9, q8            \n\t"	//d1 = d0
       "vrsqrte.f32 	q8, q8            \n\t"	//d0 = ~ 1.0 / sqrt(d0)
       
       "vmul.f32      q10, q8, q9				\n\t"	//d2 = d0 * d1
       "vrsqrts.f32 	q11, q10, q8			\n\t"	//d3 = (3 - d0 * d2) / 2
       "vmul.f32      q8, q8, q11				\n\t"	//d0 = d0 * d3
       
       "vmul.f32      q10, q8, q9				\n\t"	//d2 = d0 * d1
       "vrsqrts.f32 	q11, q10, q8			\n\t"	//d3 = (3 - d0 * d2) / 2
       "vmul.f32      q8, q8, q11				\n\t"	//d0 = d0 * d3
       
       //mul
       "vmul.f32      q12, q12, q8			\n\t"	//d0 = d0 * d4
       "vmul.f32      q13, q13, q8			\n\t"	//d0 = d0 * d4
       
       //store
       "vst2.32     {q12-q13}, [r0]!    \n\t"
       
       //loop
       "subs r3,#1   \n\t"
       "bne loop    "
       
       : // no output
       : "r" (c), "r" (a), "r" (b), "r" (N) 	// input - note *value* of pointer doesn't change
       : "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "q13", "r0", "r1", "r2", "r3"//clobber
       );
      
#elif defined(__aarch64__)
      zcomplex* & a = pCMatA->data[pMatB->data_i[i][0]] ;
      zcomplex* & b = pCMatA->data[pMatB->data_i[i][1]] ;
      zcomplex* & c = pCMatC->data[i] ;
      int N = (pCMatA->col / 2 + 1 + 3) / 4 ;
      
      __asm__ volatile
      (
       "mov x0, %0  \n\t"
       "mov x1, %1  \n\t"
       "mov x2, %2  \n\t"
       "mov x3, %3  \n\t"
       
       "loop:\n\t"
       
       //mul
       //"vld2.32     {q0-q1}, [r1]!  \n\t"
       "ld2     {v0.4s-v1.4s}, [x1], #32  \n\t"
       //"vld2.32     {q2-q3}, [r2]!  \n\t"
       "ld2     {v2.4s-v3.4s}, [x2], #32  \n\t"
       
       //"vmul.f32    q8, q0, q2      \n\t"
       //"vmul.f32    q9, q1, q3      \n\t"
       //"vmul.f32    q10, q0, q3     \n\t"
       //"vmul.f32    q11, q2, q1     \n\t"
       "fmul    v8.4s, v0.4s, v2.4s      \n\t"
       "fmul    v9.4s, v1.4s, v3.4s      \n\t"
       "fmul    v10.4s, v0.4s, v3.4s     \n\t"
       "fmul    v11.4s, v2.4s, v1.4s     \n\t"
       
       //real and img
       //"vadd.f32    q12, q8, q9     \n\t"
       "fadd    v12.4s, v8.4s, v9.4s     \n\t"
       //"vsub.f32    q13, q11, q10   \n\t"
       "fsub    v13.4s, v11.4s, v10.4s   \n\t"
       
       //module
       //"vmul.f32    q8, q12, q12      \n\t"
       //"vmla.f32    q8, q13, q13      \n\t"
       "fmul    v8.4s, v12.4s, v12.4s      \n\t"
       "fmla    v8.4s, v13.4s, v13.4s      \n\t"
       
       //sqrt+div
       //       //fast invsqrt approx
       //       "vmov.f32      d1, d0					\n\t"	//d1 = d0
       //       "vrsqrte.f32 	d0, d0					\n\t"	//d0 = ~ 1.0 / sqrt(d0)
       //       "vmul.f32      d2, d0, d1				\n\t"	//d3 = d0 * d2
       //       "vrsqrts.f32 	d3, d2, d0				\n\t"	//d4 = (3 - d0 * d3) / 2
       //       "vmul.f32      d0, d0, d3				\n\t"	//d0 = d0 * d4
       //       "vmul.f32      d2, d0, d1				\n\t"	//d3 = d0 * d2
       //       "vrsqrts.f32 	d3, d2, d0				\n\t"	//d4 = (3 - d0 * d3) / 2
       //       "vmul.f32      d0, d0, d3				\n\t"	//d0 = d0 * d4
       
       //fast invsqrt approx
       //d0=q8 d1=q9 d2=q10 d3=q11
       //"vmov.f32      q9, q8            \n\t"	//d1 = d0
       //"vrsqrte.f32 	q8, q8            \n\t"	//d0 = ~ 1.0 / sqrt(d0)
       "mov      v9.16b, v8.16b            \n\t"	//d1 = d0
       "frsqrte 	v8.4s, v8.4s            \n\t"	//d0 = ~ 1.0 / sqrt(d0)
       
       //"vmul.f32      q10, q8, q9				\n\t"	//d2 = d0 * d1
       //"vrsqrts.f32 	q11, q10, q8			\n\t"	//d3 = (3 - d0 * d2) / 2
       //"vmul.f32      q8, q8, q11				\n\t"	//d0 = d0 * d3
       "fmul      v10.4s, v8.4s, v9.4s				\n\t"	//d2 = d0 * d1
       "frsqrts 	v11.4s, v10.4s, v8.4s			\n\t"	//d3 = (3 - d0 * d2) / 2
       "fmul      v8.4s, v8.4s, v11.4s				\n\t"	//d0 = d0 * d3
       
       "fmul      v10.4s, v8.4s, v9.4s				\n\t"	//d2 = d0 * d1
       "frsqrts 	v11.4s, v10.4s, v8.4s			\n\t"	//d3 = (3 - d0 * d2) / 2
       "fmul      v8.4s, v8.4s, v11.4s				\n\t"	//d0 = d0 * d3
       
       //mul
       //"vmul.f32      q12, q12, q8			\n\t"	//d0 = d0 * d4
       //"vmul.f32      q13, q13, q8			\n\t"	//d0 = d0 * d4
       "fmul      v12.4s, v12.4s, v8.4s			\n\t"	//d0 = d0 * d4
       "fmul      v13.4s, v13.4s, v8.4s			\n\t"	//d0 = d0 * d4
       
       
       //store
       //"vst2.32     {q12-q13}, [r0]!    \n\t"
       "st2     {v12.4s-v13.4s}, [x0], #32    \n\t"
       
       //loop
       "subs x3,x3,#1   \n\t"
       "bne loop    "
       
       : // no output
       : "r" (c), "r" (a), "r" (b), "r" (N) 	// input - note *value* of pointer doesn't change
       : "memory", "v0", "v1", "v2", "v3", "v8", "v9", "v10", "v11", "v12", "v13", "x0", "x1", "x2", "x3"//clobber
       );
      
#else
      
      zcomplex* & a = pCMatA->data[pMatB->data_i[i][0]] ;
      zcomplex* & b = pCMatA->data[pMatB->data_i[i][1]] ;
      zcomplex* & c = pCMatC->data[i] ;
      int N = pCMatA->col / 2 + 1 ;
      
      for (int j = 0 ; j < N ; j ++) {
        
        float real = a[j][0] * b[j][0] + a[j][1] * b[j][1] ;
        float image = a[j][1] * b[j][0] - a[j][0] * b[j][1] ;
        float modulus = sqrtf(real*real + image*image + 1e-6);
        c[j][0] = real / modulus ;
        c[j][1] = image / modulus ;
        
      }
      
#endif
      
    }
    
    return 0 ;
    
  }
  
};




