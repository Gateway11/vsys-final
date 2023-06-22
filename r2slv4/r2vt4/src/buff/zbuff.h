//
//  zbuff.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zbuff__
#define __r2vt4__zbuff__

#include "../zmath.h"
#include "../mat/zvec.h"
#include "../mat/zmat.h"

namespace __r2vt4__ {

  //ZVecBuff--------------------------------------------------------------
  class ZVecBuff
  {
  public:
    ZVecBuff(int size);
  public:
    ~ZVecBuff(void);
    
  public:
    int PutBuff(const float* pData , int iDataNum);
    int GetBuff(float* pData, int start, int end);
    int Reset();
    
    int StoreFile(const char* pFilePath, int start, int end);
    
  public:
    
    int     m_iCurPos ;
    ZVec*   m_pVec ;
  };
  
  class ZAudBuff_S
  {
  public:
    ZAudBuff_S(int size);
  public:
    ~ZAudBuff_S(void);
    
  public:
    int PutBuff(const float* pData , int iDataNum);
    int GetBuff(float* pData, int start, int end);
    int Reset();
    
  public:
    bool    m_bWorking ;
    int     m_iCurSize ;
    float*  m_pData ;
    int     m_iTotalSize ;
    
  };
  
  
  //ZMatBuff--------------------------------------------------------------
  class ZMatBuff
  {
  public:
    ZMatBuff(int row , int col);
  public:
    ~ZMatBuff(void);
    
  public:
    int PutBuff(const float** pDataBuff, int iDataNum);
    float*  GetBuff(int offset);
    int Reset();
    
    int StoreFile(const char* pFilePath, int start, int end);
    
  public:
    
    int     m_iCurPos ;
    ZMat*   m_pMat ;
  };
  
  //ZAudBuff--------------------------------------------------------------
  class ZAudBuff
  {
  public:
    ZAudBuff(int iMicNum , int iMaxLen);
  public:
    ~ZAudBuff(void);
    
  public:
    int PutAudio(const float** pAudBuff, int iLen);
    int GetLastAudio(float** pAudBuff, int iStart, int iEnd);
    int GetLastAudio(float* pAudBuff, int iStart, int iEnd, int iCn);
    int Reset();

  public:
    
    int     m_iMicNum ;
    int     m_iMaxLen ;
    
    int     m_iCurPos ;
    ZMat*   m_pAudio ;
    
  };
  
};


#endif /* __r2vt4__zbuff__ */
