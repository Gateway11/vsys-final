//
//  zmat_neon.cpp
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zmat_neon.h"
#include "../zmacro.h"

namespace __r2vt4__ {
  
  ZMat_Neon::ZMat_Neon(int row, int col, bool bTrans){
    
    m_bTrans = bTrans ;
    m_iRow = row ;
    m_iCol = col ;
    
#ifdef __USE_MAT_NEON__
    
    m_bF16 = false  ;
    m_pData_Neon_F16 = NULL ;
    
    m_pData_NoNew = Z_SAFE_NEW_SSE_AR1(m_pData_NoNew, float, 16) ;
    
    if (m_bTrans) {
      m_iRow_Neon = (col + 3) / 4 ;
      m_iCol_Neon = (row + 3) / 4 ;
    }else{
      m_iRow_Neon = (row + 3) / 4 ;
      m_iCol_Neon = (col + 3) / 4 ;
    }
    m_pData_Neon = Z_SAFE_NEW_SSE_AR2_A64(m_pData_Neon, float, m_iRow_Neon * m_iCol_Neon , 16);
    
    if ((row % 4 == 0) && (col % 4 == 0)) {
      m_iZeroNum = 0 ;
      m_pZeroPosLst = NULL ;
    }else{
      //Find ZERO POS
      ZMat* pMat_Tmp = Z_SAFE_NEW(pMat_Tmp, ZMat, row, col);
      for (int i = 0 ; i < row ; i ++) {
        for (int j = 0 ; j < col ; j ++) {
          pMat_Tmp->data[i][j] = 1.0f ;
        }
      }
      
      CopyFrom(pMat_Tmp);
      
      std::vector<int> ZeroLst ;
      for (int i = 0 ; i < m_iRow_Neon * m_iCol_Neon; i ++) {
        for (int j = 0 ; j < 16; j ++) {
          if (m_pData_Neon[i][j] < 0.5f) {
            ZeroLst.push_back(i * 16 + j);
          }
        }
      }
      Z_SAFE_DEL(pMat_Tmp);
      
      m_iZeroNum = ZeroLst.size() ;
      m_pZeroPosLst = Z_SAFE_NEW_AR1(m_pZeroPosLst, int, m_iZeroNum);
      for (int i = 0 ; i < m_iZeroNum ; i ++) {
        m_pZeroPosLst[i] = ZeroLst[i] ;
      }
    }
    
#else
    if (m_bTrans) {
      m_iRow_Neon = col ;
      m_iCol_Neon = row ;
    }else{
      m_iRow_Neon = row ;
      m_iCol_Neon = col ;
    }
    m_pMat = Z_SAFE_NEW(m_pMat, ZMat, m_iRow_Neon, m_iCol_Neon );
#endif
    
    
    
  }
  
  ZMat_Neon::~ZMat_Neon(void){
    
#ifdef __USE_MAT_NEON__
    Z_SAFE_DEL_SSE_AR2(m_pData_Neon);
    Z_SAFE_DEL_SSE_AR2(m_pData_Neon_F16);
    Z_SAFE_DEL_AR1(m_pZeroPosLst);
    
    Z_SAFE_DEL_SSE_AR1(m_pData_NoNew);
    
#else
    Z_SAFE_DEL(m_pMat);
#endif
    
    
  }
  
  
  int ZMat_Neon::Clean(){
    
#ifdef __USE_MAT_NEON__
    memset(m_pData_Neon[0], 0, sizeof(float) * 16 * m_iRow_Neon * m_iCol_Neon);
    return  0 ;
#else
    return m_pMat->Clean() ;
#endif
    
    
  }
  
  int ZMat_Neon::Print(const char* pLabel){
    
#ifdef __USE_MAT_NEON__
    ZMat* pMat = Z_SAFE_NEW(pMat, ZMat, m_iRow, m_iCol);
    CopyTo(pMat);
    pMat->Print(pLabel);
    Z_SAFE_DEL(pMat);
    return 0 ;
#else
    return  m_pMat->Print(pLabel) ;
#endif
    
    
  }
  
  int ZMat_Neon::CopyFrom(const ZMat* pMatA){
    
    assert(m_iRow == pMatA->row);
    assert(m_iCol == pMatA->col);
    
#ifdef __USE_MAT_NEON__
    
    int row_new = 0 , row_old = 0 , col_old = 0 ;
    float *pos1 = NULL, *pos2 = NULL ;
    if (m_bTrans) {
      for (int i = 0 ; i < m_iRow_Neon ; i ++, col_old += 4) {
        row_old = 0 ;
        for (int j = 0 ; j < m_iCol_Neon ; j ++, row_new ++, row_old += 4) {
          pos1 = m_pData_Neon[row_new] ;
          pos2 = pMatA->data[row_old] + col_old ;
          for (int k = 0 ; k < 4 ; k ++ , pos1 += 4, pos2 += pMatA->col_inc ) {
            if (row_old + k < m_iRow) {
              qmempy4(pos1 , pos2 );
            }else{
              memset(pos1 , 0, sizeof(float) * 4);
            }
          }
        }
      }
    }else{
      for (int i = 0 ; i < m_iRow_Neon ; i ++, row_old += 4) {
        col_old = 0 ;
        for (int j = 0 ; j < m_iCol_Neon ; j ++, row_new ++, col_old+= 4) {
          pos1 = m_pData_Neon[row_new] ;
          pos2 = pMatA->data[row_old] + col_old ;
          for (int k = 0 ; k < 4 ; k ++ , pos1 += 4, pos2 += pMatA->col_inc) {
            if (row_old + k < m_iRow) {
              qmempy4(pos1 , pos2 );
            }else{
              memset(pos1, 0, sizeof(float) * 4);
            }
          }
          NEON_Matrix4Tran2(m_pData_Neon[row_new],m_pData_Neon[row_new]);
        }
      }
    }
    return 0 ;
#else
    if (m_bTrans) {
      for (int i = 0 ; i < m_iRow_Neon ; i ++) {
        for (int j = 0 ; j < m_iCol_Neon ; j ++) {
          m_pMat->data[i][j] = pMatA->data[j][i] ;
        }
      }
    }else{
      m_pMat->Copy(pMatA);
    }
    return 0 ;
#endif
    
  }
  
  int ZMat_Neon::CopyTo(ZMat* pMatA){
    
#ifdef __USE_MAT_NEON__
    int row_new = 0 , row_old = 0 , col_old = 0 ;
    float *pos1 = NULL, *pos2 = NULL ;
    if (m_bTrans) {
      for (int i = 0 ; i < m_iRow_Neon ; i ++, col_old += 4) {
        row_old = 0 ;
        for (int j = 0 ; j < m_iCol_Neon ; j ++, row_new++, row_old += 4) {
          pos1 = m_pData_Neon[row_new] ;
          pos2 = pMatA->data[row_old] + col_old ;
          for (int k = 0 ; k < 4 ; k ++ ,pos1 += 4, pos2 += pMatA->col_inc ) {
            if (row_old + k < m_iRow) {
              qmempy4(pos2 , pos1);
            }
          }
        }
      }
    }else{
      for (int i = 0 ; i < m_iRow_Neon ; i ++, row_old += 4) {
        col_old = 0 ;
        for (int j = 0 ; j < m_iCol_Neon ; j ++, row_new++, col_old += 4) {
          NEON_Matrix4Tran2(m_pData_Neon[row_new], m_pData_NoNew);
          pos1 = m_pData_NoNew ;
          pos2 = pMatA->data[row_old] + col_old ;
          
          for (int k = 0 ; k < 4 ; k ++ ,pos1 += 4, pos2 += pMatA->col_inc ) {
            if (row_old + k < m_iRow) {
              qmempy4(pos2 , pos1 );
            }
          }
        }
      }
    }
    
    return 0 ;
#else
    if (m_bTrans) {
      for (int i = 0 ; i < m_iRow_Neon ; i ++) {
        for (int j = 0 ; j < m_iCol_Neon ; j ++) {
          pMatA->data[j][i] = m_pMat->data[i][j] ;
        }
      }
    }else{
      pMatA->Copy(m_pMat);
    }
    return 0 ;
#endif
    
  }
  
  int ZMat_Neon::Read(ZInput* pInput){
    
    ZMat* pMat = Z_SAFE_NEW(pMat, ZMat, m_iRow, m_iCol);
    pMat->Read(pInput);
    CopyFrom(pMat);
    Z_SAFE_DEL(pMat);
    return 0 ;
  }
  
  int ZMat_Neon::Write(ZOutput* pOutput){
    
    ZMat* pMat = Z_SAFE_NEW(pMat, ZMat, m_iRow, m_iCol);
    CopyTo(pMat);
    pMat->Write(pOutput);
    Z_SAFE_DEL(pMat);
    return 0 ;
  }
  
  int ZMat_Neon::Copy(const ZMat_Neon* pMatA){
    
    assert(pMatA->m_iRow == m_iRow);
    assert(pMatA->m_iCol == m_iCol);
    assert(pMatA->m_bTrans == m_bTrans);
    
#ifdef __USE_MAT_NEON__
    memcpy(m_pData_Neon[0], pMatA->m_pData_Neon[0], sizeof(float) * 16 * m_iRow_Neon * m_iCol_Neon);
#else
    m_pMat->Copy(pMatA->m_pMat);
#endif
    
    
    return 0 ;
  }
  
  int ZMat_Neon::RowSigMoid(){
    
#ifdef __USE_MAT_NEON__
    int totalsize = 16 * m_iRow_Neon * m_iCol_Neon ;
    for (int i = 0 ; i < totalsize; i ++) {
      float &data = m_pData_Neon[0][i] ;
#ifdef QUICK_EXP
      if (data < - QUICK_EXP_SHIELD){
        data = ZMat::m_plogtab2[0] ;
      }else if (data > QUICK_EXP_SHIELD){
        data = ZMat::m_plogtab2[QUICK_EXP_CONSTANT_2] ;
      }else{
        data = ZMat::m_plogtab2[(int)(data * QUICK_EXP_CONSTANT_3 + QUICK_EXP_CONSTANT_1)] ;
      }
#else
      data = 1.0f /(1+exp(- data));
#endif
    }
    
    for (int i = 0 ; i < m_iZeroNum ; i ++) {
      m_pData_Neon[0][m_pZeroPosLst[i]] = 0 ;
    }
    
    return 0 ;
#else
    return  m_pMat->RowSigMoid() ;
#endif
    
    
  }
  
  int ZMat_Neon::RowTanh(){
    
#ifdef __USE_MAT_NEON__
    int totalsize = 16 * m_iRow_Neon * m_iCol_Neon ;
    for (int i = 0 ; i < totalsize; i ++) {
      float &data = m_pData_Neon[0][i] ;
#ifdef QUICK_EXP
      if (data < - QUICK_EXP_SHIELD){
        data = ZMat::m_plogtab3[0] ;
      }else if (data > QUICK_EXP_SHIELD){
        data = ZMat::m_plogtab3[QUICK_EXP_CONSTANT_2] ;
      }else{
        data = ZMat::m_plogtab3[(int)(data * QUICK_EXP_CONSTANT_3 + QUICK_EXP_CONSTANT_1)] ;
      }
#else
      float x = exp(-data) ;
      x = x * x ;
      data = (1.0f - x) /(1.0f + x);
#endif
    }
    
    for (int i = 0 ; i < m_iZeroNum ; i ++) {
      m_pData_Neon[0][m_pZeroPosLst[i]] = 0 ;
    }
    
    return 0 ;
#else
    return m_pMat->RowTanh() ;
#endif
    
  }
  
  int ZMat_Neon::RowRelu(){
    
#ifdef __USE_MAT_NEON__
    int totalsize = 16 * m_iRow_Neon * m_iCol_Neon ;
    
    for (int i = 0 ; i < totalsize ; i ++) {
      float &data = m_pData_Neon[0][i] ;
      if (data < 0) {
        data = 0 ;
      }
    }
    return 0 ;
#else
    return m_pMat->RowRelu() ;
#endif
  }
  
  int ZMat_Neon::RowCopy_4(const ZVec* pVec){
    
    assert(m_iRow == 4) ;
    assert(pVec->size == m_iCol) ;
    assert(!m_bTrans) ;
    
    ZMat* pMat = Z_SAFE_NEW(pMat, ZMat, m_iRow, m_iCol);
    pMat->RowCopy(pVec);
    this->CopyFrom(pMat);
    Z_SAFE_DEL(pMat) ;
    
    return  0 ;
  }
  
  int ZMat_Neon::RowCopy_N(const ZMat_Neon* pVec_Noen){
    
#ifdef __USE_MAT_NEON__
    assert(!m_bTrans) ;
    assert(m_bTrans == pVec_Noen->m_bTrans) ;
    assert(pVec_Noen->m_iRow_Neon == 1) ;
    assert(m_iCol_Neon == pVec_Noen->m_iCol_Neon) ;
    
    for (int i = 0 ; i < m_iRow_Neon ; i ++) {
      memcpy(m_pData_Neon[i * m_iCol_Neon], pVec_Noen->m_pData_Neon[0], sizeof(float) * m_iCol_Neon * 16 ) ;
    }
    return 0 ;
    
#else
    return m_pMat->RowCopy_N(pVec_Noen->m_pMat) ;
#endif
    
  }
  
  
  int ZMat_Neon::ResetRow(int iRow){
    
#ifdef __USE_MAT_NEON__
    
    assert(!m_bTrans);
    
    m_iRow = iRow ;
    m_iRow_Neon = (iRow + 3) / 4 ;
    
    if (m_iRow != m_iRow_Neon * 4) {
      memset(m_pData_Neon[(m_iRow_Neon - 1) * m_iCol_Neon], 0, sizeof(float) * 16 * m_iCol_Neon) ;
    }
    
    return  0 ;
    
#else
    m_iRow = iRow ;
    m_iRow_Neon = iRow ;
    
    return m_pMat->ResetRow(iRow) ;
#endif
    
    
  }
  
  
  int ZMat_Neon::Add_aAB_NEON(const ZMat_Neon* pMatA, const ZMat_Neon* pMatB){
    
    assert(pMatA->m_iRow_Neon == m_iRow_Neon);
    assert(pMatA->m_iCol_Neon == pMatB->m_iRow_Neon);
    assert(pMatB->m_iCol_Neon == m_iCol_Neon);
    
#ifdef __USE_MAT_NEON__
    
    int M = pMatA->m_iRow_Neon;
    int N = pMatB->m_iCol_Neon;
    int K = pMatA->m_iCol_Neon;
    
#if defined(__arm__) //&& __ANDROID_API__ <= 26
    
    if (pMatB->m_bF16) {
      
      int offset_a = 16 ;
      int offset_b = pMatB->m_iCol_Neon * 16 ;
      
      for (int m = 0 ; m < M ; m ++) {
        float * a = pMatA->m_pData_Neon[m * pMatA->m_iCol_Neon] ;
        unsigned short * b = pMatB->m_pData_Neon_F16[0] ;
        float * c = m_pData_Neon[m * m_iCol_Neon] ;
        
        for (int k = 0 ; k < K ; k ++) {
          
          __asm__ volatile
          (
           "mov r0, %0  \n\t"
           "mov r1, %1  \n\t"
           "mov r2, %2  \n\t"
           "mov r3, %3  \n\t"
           "vldmia r1, { q8-q11 }  \n\t"
           
           "LOOPF16:\n\t"
           
           // Store A & B leaving room for q4-q7, which should be preserved
           "vldmia r2!, { q4-q5 }   \n\t"
           "vldmia r0, { q12-q15 } \n\t"
           
           "vcvt.f32.f16  q0, d8	\n\t"
           "vcvt.f32.f16  q1, d9	\n\t"
           "vcvt.f32.f16  q2, d10	\n\t"
           "vcvt.f32.f16  q3, d11	\n\t"
           
           // result = first column of B x first row of A
           "vmla.f32 q12, q8, d0[0]\n\t"
           "vmla.f32 q13, q8, d2[0]\n\t"
           "vmla.f32 q14, q8, d4[0]\n\t"
           "vmla.f32 q15, q8, d6[0]\n\t"
           
           // result += second column of B x second row of A
           "vmla.f32 q12, q9, d0[1]\n\t"
           "vmla.f32 q13, q9, d2[1]\n\t"
           "vmla.f32 q14, q9, d4[1]\n\t"
           "vmla.f32 q15, q9, d6[1]\n\t"
           
           // result += third column of B x third row of A
           "vmla.f32 q12, q10, d1[0]\n\t"
           "vmla.f32 q13, q10, d3[0]\n\t"
           "vmla.f32 q14, q10, d5[0]\n\t"
           "vmla.f32 q15, q10, d7[0]\n\t"
           
           // result += last column of B x last row of A
           "vmla.f32 q12, q11, d1[1]\n\t"
           "vmla.f32 q13, q11, d3[1]\n\t"
           "vmla.f32 q14, q11, d5[1]\n\t"
           "vmla.f32 q15, q11, d7[1]\n\t"
           
           // output = result registers
           "vstmia r0!, { q12-q15 }\n\t"
           "pld [r2, #0]    \n\t"
           "pld [r0, #0]    \n\t"
           
           
           //loop
           "SUBS r3,#1   \n\t"
           "BNE LOOPF16    "
           
           : // no output
           : "r" (c), "r" (a), "r" (b), "r" (N) 	// input - note *value* of pointer doesn't change
           : "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15", "r0", "r1", "r2", "r3"//clobber
           );
          
          
          a += offset_a ;
          b += offset_b ;
          
        }
      }
    }else{
      int offset_a = 16 ;
      int offset_b = pMatB->m_iCol_Neon * 16 ;
      
      for (int m = 0 ; m < M ; m ++) {
        float * a = pMatA->m_pData_Neon[m * pMatA->m_iCol_Neon] ;
        float * b = pMatB->m_pData_Neon[0] ;
        float * c = m_pData_Neon[m * m_iCol_Neon] ;
        
        for (int k = 0 ; k < K ; k ++) {
          
          __asm__ volatile
          (
           "mov r0, %0  \n\t"
           "mov r1, %1  \n\t"
           "mov r2, %2  \n\t"
           "mov r3, %3  \n\t"
           "vldmia r1, { q8-q11 }  \n\t"
           
           "LOOPF32:\n\t"
           
           // Store A & B leaving room for q4-q7, which should be preserved
           "vldmia r2!, { q0-q3 }   \n\t"
           "vldmia r0, { q12-q15 } \n\t"
           
           // result = first column of B x first row of A
           "vmla.f32 q12, q8, d0[0]\n\t"
           "vmla.f32 q13, q8, d2[0]\n\t"
           "vmla.f32 q14, q8, d4[0]\n\t"
           "vmla.f32 q15, q8, d6[0]\n\t"
           
           // result += second column of B x second row of A
           "vmla.f32 q12, q9, d0[1]\n\t"
           "vmla.f32 q13, q9, d2[1]\n\t"
           "vmla.f32 q14, q9, d4[1]\n\t"
           "vmla.f32 q15, q9, d6[1]\n\t"
           
           // result += third column of B x third row of A
           "vmla.f32 q12, q10, d1[0]\n\t"
           "vmla.f32 q13, q10, d3[0]\n\t"
           "vmla.f32 q14, q10, d5[0]\n\t"
           "vmla.f32 q15, q10, d7[0]\n\t"
           
           // result += last column of B x last row of A
           "vmla.f32 q12, q11, d1[1]\n\t"
           "vmla.f32 q13, q11, d3[1]\n\t"
           "vmla.f32 q14, q11, d5[1]\n\t"
           "vmla.f32 q15, q11, d7[1]\n\t"
           
           // output = result registers
           "vstmia r0!, { q12-q15 }\n\t"
           "pld [r2, #0]    \n\t"
           "pld [r0, #0]    \n\t"
           
           //loop
           "SUBS r3,#1   \n\t"
           "BNE LOOPF32    "
           
           : // no output
           : "r" (c), "r" (a), "r" (b), "r" (N) 	// input - note *value* of pointer doesn't change
           : "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15", "r0", "r1", "r2", "r3"//clobber
           );
          
          
          a += offset_a ;
          b += offset_b ;
          
        }
      }
    }
    
#elif defined(__aarch64__)
    
    
    if (pMatB->m_bF16) {
      for (int m = 0 ; m < M ; m ++) {
        for (int k = 0 ; k < K ; k ++) {
          
          float * a = pMatA->m_pData_Neon[m * pMatA->m_iCol_Neon + k] ;
          unsigned short * b = pMatB->m_pData_Neon_F16[k * pMatB->m_iCol_Neon] ;
          float * c = m_pData_Neon[m * m_iCol_Neon] ;
          
          __asm__ volatile
          (
           "mov x0, %0  \n\t"
           "mov x1, %1  \n\t"
           "mov x2, %2  \n\t"
           "mov x3, %3  \n\t"
           //"vldmia r1, { q8-q11 }  \n\t"
           "ld1 { v8.4s-v11.4s }, [x1]  \n\t"
           
           "LOOP16:\n\t"
           
           // Store A & B leaving room for q4-q7, which should be preserved
           //"vldmia r0, { q12-q15 } \n\t"
           //"vldmia r2!, { q0-q3 }   \n\t"
           "ld1 { v4.4s-v5.4s }, [x2], #32   \n\t"
           "ld1 { v12.4s-v15.4s }, [x0] \n\t"
           
           "fcvtl  v0.4s,  v4.4h \n\t"
           "fcvtl2 v1.4s,  v4.8h \n\t"
           "fcvtl  v2.4s,  v5.4h \n\t"
           "fcvtl2 v3.4s,  v5.8h \n\t"
           
           // result = first column of B x first row of A
           //"vmla.f32 q12, q8, d0[0]\n\t"
           //"vmla.f32 q13, q8, d2[0]\n\t"
           //"vmla.f32 q14, q8, d4[0]\n\t"
           //"vmla.f32 q15, q8, d6[0]\n\t"
           "fmla v12.4s, v8.4s, v0.s[0]\n\t"
           "fmla v13.4s, v8.4s, v1.s[0]\n\t"
           "fmla v14.4s, v8.4s, v2.s[0]\n\t"
           "fmla v15.4s, v8.4s, v3.s[0]\n\t"
           
           // result += second column of B x second row of A
           //"vmla.f32 q12, q9, d0[1]\n\t"
           //"vmla.f32 q13, q9, d2[1]\n\t"
           //"vmla.f32 q14, q9, d4[1]\n\t"
           //"vmla.f32 q15, q9, d6[1]\n\t"
           "fmla v12.4s, v9.4s, v0.s[1]\n\t"
           "fmla v13.4s, v9.4s, v1.s[1]\n\t"
           "fmla v14.4s, v9.4s, v2.s[1]\n\t"
           "fmla v15.4s, v9.4s, v3.s[1]\n\t"
           
           // result += third column of B x third row of A
           //"vmla.f32 q12, q10, d1[0]\n\t"
           //"vmla.f32 q13, q10, d3[0]\n\t"
           //"vmla.f32 q14, q10, d5[0]\n\t"
           //"vmla.f32 q15, q10, d7[0]\n\t"
           "fmla v12.4s, v10.4s, v0.s[2]\n\t"
           "fmla v13.4s, v10.4s, v1.s[2]\n\t"
           "fmla v14.4s, v10.4s, v2.s[2]\n\t"
           "fmla v15.4s, v10.4s, v3.s[2]\n\t"
           
           // result += last column of B x last row of A
           //"vmla.f32 q12, q11, d1[1]\n\t"
           //"vmla.f32 q13, q11, d3[1]\n\t"
           //"vmla.f32 q14, q11, d5[1]\n\t"
           //"vmla.f32 q15, q11, d7[1]\n\t"
           "fmla v12.4s, v11.4s, v0.s[3]\n\t"
           "fmla v13.4s, v11.4s, v1.s[3]\n\t"
           "fmla v14.4s, v11.4s, v2.s[3]\n\t"
           "fmla v15.4s, v11.4s, v3.s[3]\n\t"
           
           // output = result registers
           //"vstmia r0!, { q12-q15 }\n\t"
           "st1 { v12.4s-v15.4s }, [x0], #64\n\t"
           "prfm  pldl1keep, [x2, #0]\n\t"
           "prfm  pldl1keep, [x0, #0]\n\t"
           
           //loop
           "SUBS x3,x3,#1   \n\t"
           "BNE LOOP16    "
           
           : // no output
           : "r" (c), "r" (a), "r" (b), "r" (N) 	// input - note *value* of pointer doesn't change
           : "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "x0", "x1", "x2", "x3"//clobber
           );
          
        }
      }
      
    }else{
      
      for (int m = 0 ; m < M ; m ++) {
        for (int k = 0 ; k < K ; k ++) {
          
          float * a = pMatA->m_pData_Neon[m * pMatA->m_iCol_Neon + k] ;
          float * b = pMatB->m_pData_Neon[k * pMatB->m_iCol_Neon] ;
          float * c = m_pData_Neon[m * m_iCol_Neon] ;
          
          __asm__ volatile
          (
           "mov x0, %0  \n\t"
           "mov x1, %1  \n\t"
           "mov x2, %2  \n\t"
           "mov x3, %3  \n\t"
           //"vldmia r1, { q8-q11 }  \n\t"
           "ld1 { v8.4s-v11.4s }, [x1]  \n\t"
           
           "LOOP32:\n\t"
           
           // Store A & B leaving room for q4-q7, which should be preserved
           //"vldmia r0, { q12-q15 } \n\t"
           //"vldmia r2!, { q0-q3 }   \n\t"
           "ld1 { v0.4s-v3.4s }, [x2], #64   \n\t"
           "ld1 { v12.4s-v15.4s }, [x0] \n\t"
           
           // result = first column of B x first row of A
           //"vmla.f32 q12, q8, d0[0]\n\t"
           //"vmla.f32 q13, q8, d2[0]\n\t"
           //"vmla.f32 q14, q8, d4[0]\n\t"
           //"vmla.f32 q15, q8, d6[0]\n\t"
           "fmla v12.4s, v8.4s, v0.s[0]\n\t"
           "fmla v13.4s, v8.4s, v1.s[0]\n\t"
           "fmla v14.4s, v8.4s, v2.s[0]\n\t"
           "fmla v15.4s, v8.4s, v3.s[0]\n\t"
           
           // result += second column of B x second row of A
           //"vmla.f32 q12, q9, d0[1]\n\t"
           //"vmla.f32 q13, q9, d2[1]\n\t"
           //"vmla.f32 q14, q9, d4[1]\n\t"
           //"vmla.f32 q15, q9, d6[1]\n\t"
           "fmla v12.4s, v9.4s, v0.s[1]\n\t"
           "fmla v13.4s, v9.4s, v1.s[1]\n\t"
           "fmla v14.4s, v9.4s, v2.s[1]\n\t"
           "fmla v15.4s, v9.4s, v3.s[1]\n\t"
           
           // result += third column of B x third row of A
           //"vmla.f32 q12, q10, d1[0]\n\t"
           //"vmla.f32 q13, q10, d3[0]\n\t"
           //"vmla.f32 q14, q10, d5[0]\n\t"
           //"vmla.f32 q15, q10, d7[0]\n\t"
           "fmla v12.4s, v10.4s, v0.s[2]\n\t"
           "fmla v13.4s, v10.4s, v1.s[2]\n\t"
           "fmla v14.4s, v10.4s, v2.s[2]\n\t"
           "fmla v15.4s, v10.4s, v3.s[2]\n\t"
           
           // result += last column of B x last row of A
           //"vmla.f32 q12, q11, d1[1]\n\t"
           //"vmla.f32 q13, q11, d3[1]\n\t"
           //"vmla.f32 q14, q11, d5[1]\n\t"
           //"vmla.f32 q15, q11, d7[1]\n\t"
           "fmla v12.4s, v11.4s, v0.s[3]\n\t"
           "fmla v13.4s, v11.4s, v1.s[3]\n\t"
           "fmla v14.4s, v11.4s, v2.s[3]\n\t"
           "fmla v15.4s, v11.4s, v3.s[3]\n\t"
           
           // output = result registers
           //"vstmia r0!, { q12-q15 }\n\t"
           "st1 { v12.4s-v15.4s }, [x0], #64\n\t"
           "prfm  pldl1keep, [x2, #0]\n\t"
           "prfm  pldl1keep, [x0, #0]\n\t"
           
           
           //loop
           "SUBS x3,x3,#1   \n\t"
           "BNE LOOP32    "
           
           : // no output
           : "r" (c), "r" (a), "r" (b), "r" (N) 	// input - note *value* of pointer doesn't change
           : "memory", "v0", "v1", "v2", "v3", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "x0", "x1", "x2", "x3"//clobber
           );
          
        }
      }
    }
    
#else
    for (int m = 0 ; m < M ; m ++) {
      for (int n = 0 ; n < N ; n ++) {
        for (int k = 0 ; k < K ; k ++) {
          
          cblas_sgemm(CblasColMajor,CblasNoTrans,CblasNoTrans,4,4,4,
                      1.0f, pMatA->m_pData_Neon[m * pMatA->m_iCol_Neon + k],4, pMatB->m_pData_Neon[k * pMatB->m_iCol_Neon + n],4,
                      1.0f, m_pData_Neon[m * m_iCol_Neon + n],4);
          
        }
      }
    }
#endif
    
    return  0 ;
#else
    return  m_pMat->Add_aAB(pMatA->m_pMat, pMatB->m_pMat);
#endif
    
    
    
  }
  
  
  
  
#ifdef __USE_MAT_NEON__
  int ZMat_Neon::Change2F16(){
    
#if defined(__arm__) //&& __ANDROID_API__ <= 26
    
    m_bF16 = true ;
    m_pData_Neon_F16 = Z_SAFE_NEW_SSE_AR2(m_pData_Neon_F16, unsigned short, m_iRow_Neon * m_iCol_Neon , 16);
    for (int i = 0 ; i < m_iRow_Neon * m_iCol_Neon ; i ++) {
      float* a = m_pData_Neon[i] ;
      unsigned short * b = m_pData_Neon_F16[i] ;
      __asm__ volatile
      (
       
       "vldmia %0, { q0-q3 } \n\t"
       
       "vcvt.f16.f32 d8,  q0 \n\t"
       "vcvt.f16.f32 d9,  q1 \n\t"
       "vcvt.f16.f32 d10, q2 \n\t"
       "vcvt.f16.f32 d11, q3 \n\t"
       
       // output = result registers
       "vstmia %1, { q4-q5 }"
       
       : // no output
       : "r" (a), "r" (b) 	// input - note *value* of pointer doesn't change
       : "memory", "q0", "q1", "q2", "q3", "q4", "q5"//clobber
       );
    }
    return  0 ;
    
#elif defined(__aarch64__)
    
    m_bF16 = true ;
    m_pData_Neon_F16 = Z_SAFE_NEW_SSE_AR2(m_pData_Neon_F16, unsigned short, m_iRow_Neon * m_iCol_Neon , 16);
    for (int i = 0 ; i < m_iRow_Neon * m_iCol_Neon ; i ++) {
      float* a = m_pData_Neon[i] ;
      unsigned short * b = m_pData_Neon_F16[i] ;
      __asm__ volatile
      (
       //"vldmia r0, { q0-q3 } \n\t"
       "ld1 { v0.4s-v3.4s }, [%0]  \n\t"
       
       //"vcvt.f16.f32 d8,  q0 \n\t"
       //"vcvt.f16.f32 d9,  q1 \n\t"
       //"vcvt.f16.f32 d10, q2 \n\t"
       //"vcvt.f16.f32 d11, q3 \n\t"
       "fcvtn  v4.4h,  v0.4s \n\t"
       "fcvtn2 v4.8h,  v1.4s \n\t"
       "fcvtn  v5.4h,  v2.4s \n\t"
       "fcvtn2 v5.8h,  v3.4s \n\t"
       
       // output = result registers
       //"vstmia r1, { q4-q5 }"
       "st1 { v4.4s-v5.4s }, [%1] \n\t"
       
       : // no output
       : "r" (a), "r" (b) 	// input - note *value* of pointer doesn't change
       : "memory", "v0", "v1", "v2", "v3", "v4", "v5"//clobber
       );
    }
    
    return 0 ;
    
#else
    
    return 0 ;
    
#endif
    
  }
  
#endif
  
};




