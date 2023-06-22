//
//  zphosto.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zphosto__
#define __r2vt4__zphosto__

#include "../../zmath.h"
#include "../zvtapi.h"

namespace __r2vt4__ {

#define  PHO_FLAG_WORD_BEGIN	0x0001
#define  PHO_FLAG_WORD_END		0x0002
#define  PHO_FLAG_BLOCK_BEGIN	0x0004
#define  PHO_FLAG_BLOCK_END     0x0008
#define  PHO_FLAG_ENABLE_SIL    0x0010
  
  struct ZPhoNode{
    int iPhoStateId[3] ;
  };
  
  struct ZPhoBatch{
    int iPhoFlag ;
    int iPhoNum ;
    ZPhoNode* pPhoList ;
  };
  
  
  //ZPhoTable--------------------------------------------------------------
  class ZPhoTable
  {
  public:
    ZPhoTable(void);
  public:
    ~ZPhoTable(void);
    
  public:
    std::vector<ZPhoBatch*> ParseWordTriPhoLst(const char* pWordTriPhoLst);
    std::vector<ZPhoBatch*> ParseWordMonoPhoLst(const char* pWordMonoPhoLst, bool bRightSil);
    
  public:
    //io
    int LoadOldTable(const char* pMonPhoPath, const char* pFixTriPhoPath, const char* pTriPhoMapPath) ;
    int LoadOldTable2(const char* pMonPhoPath, const char* pFixTriPhoIdPath, const char* pFixTriPhoPath, const char* pTriPhoMapPath) ;
    int LoadNewTable(const char* pNewTablePath);
    
    int SaveNewTable(const char* pNewTablePath);
    
  protected:
    int GetTriPhoId(const char* pTriPhoneName) ;
    int GetTriPhoId(const char* pPre, const char* pCur, const char* Next) ;
    
  protected:
    
    //MonPho
    int m_iMonPhoNum ;
    std::vector<std::string> m_MonPhoLst ;
    std::map<std::string, int> m_MonPho2Id ;
    
    //TriPho Fix
    int m_iTriPhoNum_Fix ;
    ZPhoNode *m_pFixPhoState ;
    
    //TriPho Map
    int m_iTriPhoNum_Total ;
    int *m_pTriPho2FixPho ;
    
  };
  
  //ZVtState--------------------------------------------------------------
  struct ZVtState{
    
    int iStateFlag ;
    
    int iCurStateNum ;
    int* pCurStateLst ;
    
    int iPreStateNum ;
    int* pPreStateLst ;
    
    int iPhoId ;
    int iPhoStateId ;
    
  };
  
  //ZVtWord--------------------------------------------------------------
  class ZVtWord{
    
  public:
    ZVtWord(void);
    ~ZVtWord(void);
    
  public:
    int SetPhoBatchLst(std::vector<ZPhoBatch*> PhoBatchLst);
    int PrintWordInfo();
    bool CheckNoCmdChange(bool bAec);
    
  public:
    
    WordType   m_iWordType ;
    std::string   m_strWordContent ;
    
    int     m_iBlockMinFrmLen ;
    int     m_iBlockMaxFrmLen ;
    
    float   m_fBlockAvgScore ;
    float   m_fBlockMinScore ;
    
    bool    m_bLeftSilCheck ;
    int     m_iLeftSilOffset ;
    int     m_iLeftSilSkip ;
    float   m_fLeftSilShield ;
    
    bool    m_bRightSilCheck ;
    int     m_iRightSilOffset ;
    int     m_iRightSilSkip ;
    float   m_fRightSilShield ;
    
    bool    m_bClassifyCheck ;
    float   m_fClassifyNnetShield ;
    std::string m_strClassifyNnetPath ;
    
    bool    m_bRemoteAsrCheckWithAec ;
    bool    m_bRemoteAsrCheckWithNoAec ;
    
    int     m_iBlockNum ;
    int     m_iVtStateNum ;
    ZVtState** m_pVtStateLst ;
    
    std::vector<ZPhoBatch*> m_PhoBatchLst ;
    
  };

};


#endif /* __r2vt4__zphosto__ */
