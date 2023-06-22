//
//  zfilter.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zfilter__
#define __r2vt4__zfilter__

#include "../zmath.h"
#include "../mat/zvec.h"
#include "../mat/zmat.h"

namespace  __r2vt4__ {
  
  //--------------------------------------------------------------
  class ZFilter
  {
  public:
    ZFilter(int iLeftPos, int iRightPos, int iInDim, int iOutDim);
  public:
    virtual ~ZFilter(void);
    
    int InitFilter(int iLeftPos, int iRightPos, int iInDim, int iOutDim);
    int FilterFea(ZMat* pInFea, int iInFeaNum, ZMat* &pOutFea, int &iOutFeaNum);
    int ResetFilter();
    
    virtual int OnResetFilter(){ return 0; };
    
    virtual int DoSingleFilter(float** pFeaIn, bool bFirstOutFrm) = 0 ;
    virtual int OnFirstInFrame(float* pFeaIn) = 0;
    
  protected:
    
    int m_bFirstInFrame ;
    int m_bFirstOutFrame ;
    
    int m_iLeftPos ;
    int m_iRightPos ;
    int m_iCurPos ;
    
    int m_iInDim ;
    int m_iInBuffLen ;
    int m_iInFeaPos ;
    ZMat* m_pInFea ;
    
    int m_iOutDim ;
    int m_iOutBuffLen ;
    int m_iOutFeaPos ;
    ZMat* m_pOutFea ;
    
    float** m_pSingleData ;
    
  };
  
  //--------------------------------------------------------------
  class ZFilter_Delt :public ZFilter
  {
  public:
    ZFilter_Delt(int iLeftPos, int iRightPos, int iInDim, int iOutDim);
  public:
    virtual ~ZFilter_Delt(void);
    
  public:
    virtual int DoSingleFilter(float** pFeaIn, bool bFirstOutFrm);
    virtual int OnFirstInFrame(float* pFeaIn);
  };
  
  //--------------------------------------------------------------
  class ZFilter_Norm :public ZFilter
  {
  public:
    ZFilter_Norm(int iLeftPos, int iRightPos, int iInDim, int iOutDim);
  public:
    virtual ~ZFilter_Norm(void);
    
  public:
    virtual int DoSingleFilter(float** pFeaIn, bool bFirstOutFrm);
    virtual int OnFirstInFrame(float* pFeaIn);
    
    ZMat* m_pNormMat ;
    int m_iNormLen ;
    
    static float avg_data[174];
    float* m_pfMean ;
    float* m_pfVar ;
  };
  
  //--------------------------------------------------------------
  class ZFilter_Cmb :public ZFilter
  {
  public:
    ZFilter_Cmb(int iLeftPos, int iRightPos, int iInDim, int iOutDim);
  public:
    virtual ~ZFilter_Cmb(void);
    
  public:
    virtual int DoSingleFilter(float** pFeaIn, bool bFirstOutFrm);
    virtual int OnFirstInFrame(float* pFeaIn);
    
#ifdef NORM_GLOBAL
    ZMat* m_pNormMat ;
#endif
  };

};


#endif /* __r2vt4__zfilter__ */
