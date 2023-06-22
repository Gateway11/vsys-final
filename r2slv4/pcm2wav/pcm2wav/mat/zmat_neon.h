//
//  zmat_neon.h
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zmat_neon__
#define __r2vt4__zmat_neon__

#include "../zmath.h"
#include "zmat.h"
#include "zvec.h"
#include "../io/zinput.h"
#include "../io/zoutput.h"

namespace __r2vt4__ {

#ifdef __ARM_ARCH_ARM__
#define  __USE_MAT_NEON__
#else
//#define  __USE_MAT_NEON__
#endif
  
  class ZMat ;
  class ZVec ;
  class ZMat_Neon
  {
  public:
    ZMat_Neon(int row, int col, bool bTrans);
  public:
    ~ZMat_Neon(void);
    
  public:
    
    int Clean() ;
    int Print(const char* pLabel);
    int CopyFrom(const ZMat* pMatA);
    int CopyTo(ZMat* pMatA);
    int Copy(const ZMat_Neon* pMatA);
    
    int Read(ZInput* pInput);
    int Write(ZOutput* pOutput);
    
  public:
    int RowSigMoid();
    int RowTanh();
    int RowRelu();
    int RowCopy_4(const ZVec* pVec);
    int RowCopy_N(const ZMat_Neon* pVec_Noen);
    
    int ResetRow(int iRow);
    
    int Add_aAB_NEON(const ZMat_Neon* pMatA, const ZMat_Neon* pMatB);
    
  public:
    
    bool      m_bTrans ;
    int       m_iRow ;
    int       m_iCol ;
    
    int       m_iRow_Neon ;
    int       m_iCol_Neon ;
    
#ifdef __USE_MAT_NEON__
    
    float**   m_pData_Neon ;
    
    int       m_iZeroNum ;
    int*      m_pZeroPosLst ;
    
    inline void NEON_Matrix4Tran(float* m) ;
    
    //No New
    float* m_pData_NoNew ;
    
#else
    
    ZMat*   m_pMat ;
    
#endif
    
    
  };
  
};


#endif /* __r2vt4__zmat_neon__ */
