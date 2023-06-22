//
//  zphosto.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zphosto.h"
#include "../../io/zinput.h"
#include "../../io/zoutput.h"

namespace __r2vt4__ {
  
  //ZPhoTable--------------------------------------------------------------
  ZPhoTable::ZPhoTable(void){
    
    //MonPho
    m_iMonPhoNum = 0 ;
    
    //TriPho Fix
    m_iTriPhoNum_Fix = 0 ;
    m_pFixPhoState = NULL ;
    
    //TriPho Map
    m_iTriPhoNum_Total = 0  ;
    m_pTriPho2FixPho = NULL ;
    
  }
  
  ZPhoTable::~ZPhoTable(void){
    
    Z_SAFE_DEL_AR1(m_pFixPhoState) ;
    Z_SAFE_DEL_AR1(m_pTriPho2FixPho) ;
  }
  
  std::vector<ZPhoBatch*> ZPhoTable::ParseWordTriPhoLst(const char* pWordTriPhoLst){
    
    std::vector<ZPhoBatch*> rst ;
    char* pWord_tmp = Z_SAFE_NEW_AR1(pWord_tmp, char, strlen(pWordTriPhoLst) + 5) ;
    strcpy(pWord_tmp,pWordTriPhoLst);
    
    int iStart = 0 ;
    char* pTriPhoLst = NULL, * pTok = NULL, * pTok1 = NULL ;
    pTriPhoLst = strtok_r(pWord_tmp,";",&pTok);
    while(pTriPhoLst != NULL){
      ZPhoBatch* pPhoBatch = Z_SAFE_NEW(pPhoBatch,ZPhoBatch) ;
      pPhoBatch->iPhoFlag = 0 ;
      
      bool bFlag = false ;
      std::vector<std::string> TriPhoLst ;
      char* pTriPho = strtok_r(pTriPhoLst,"|",&pTok1);
      while(pTriPho != NULL){
        if (strcasecmp(pTriPho,"#") == 0){
          bFlag = true ;
        }else{
          TriPhoLst.push_back(pTriPho);
        }
        pTriPho = strtok_r(NULL,"|",&pTok1);
      }
      
      pPhoBatch->iPhoNum = TriPhoLst.size() ;
      pPhoBatch->pPhoList = Z_SAFE_NEW_AR1(pPhoBatch->pPhoList, ZPhoNode, pPhoBatch->iPhoNum);
      
      for (int i = 0 ; i < pPhoBatch->iPhoNum ; i ++) {
        int iTriPhoId = GetTriPhoId(TriPhoLst[i].c_str()) ;
        int iFixTriPhoId = m_pTriPho2FixPho[iTriPhoId] ;
        
        assert(iFixTriPhoId != -1) ;
        memcpy(pPhoBatch->pPhoList + i, m_pFixPhoState + iFixTriPhoId, sizeof(ZPhoNode));
        //printf("%d ", iFixTriPhoId);
      }
      //printf("#%d\n", bFlag) ;
      rst.push_back(pPhoBatch);
      
      if (bFlag == true){
        rst[iStart]->iPhoFlag |= PHO_FLAG_BLOCK_BEGIN ;
        pPhoBatch->iPhoFlag |= PHO_FLAG_BLOCK_END;
        iStart = rst.size() ;
      }
      
      pTriPhoLst = strtok_r(NULL,";",&pTok);
    }
    
    if (rst.size() > 0)	{
      rst[0]->iPhoFlag |= PHO_FLAG_WORD_BEGIN ;
      rst[rst.size()-1]->iPhoFlag |= PHO_FLAG_WORD_END ;
    }
    
    Z_SAFE_DEL_AR1(pWord_tmp);
    
    return rst ;
  }
  
  std::vector<ZPhoBatch*> ZPhoTable::ParseWordMonoPhoLst(const char* pWordMonoPhoLst, bool bRightSil){
    
    char *pWord_Tmp = Z_SAFE_NEW_AR1(pWord_Tmp,char,strlen(pWordMonoPhoLst) + 5);
    strcpy(pWord_Tmp,pWordMonoPhoLst);
    
    //Get MonoPhtLst Lst
    std::vector<std::vector<std::string> > MonoPhoLst ;
    std::vector<int> MonoPhoState ;
    
    
    //Left Sil
    std::vector<std::string> SilLst ;
    SilLst.push_back("sil");
    SilLst.push_back("<eps>");
    MonoPhoLst.push_back(SilLst) ;
    MonoPhoState.push_back(0) ;
    
    //
    char *pTok1 = NULL , *pTok2 = NULL ;
    char *pMonoPhoLst = strtok_r(pWord_Tmp," \r\t\n",&pTok1);
    while (pMonoPhoLst != NULL) {
      std::vector<std::string> MPlist ;
      int iFlag = 0 ;
      char* pMonoPho = strtok_r(pMonoPhoLst, "|", &pTok2) ;
      while (pMonoPho != NULL) {
        if (strcasecmp(pMonoPho,"#") == 0){
          iFlag = 1 ;
        }else if(strcasecmp(pMonoPho,"##") == 0){
          iFlag = 2 ;
        }else{
          MPlist.push_back(pMonoPho);
        }
        pMonoPho = strtok_r(NULL,"|",&pTok2);
      }
      
      MonoPhoLst.push_back(MPlist);
      MonoPhoState.push_back(iFlag);
      
      pMonoPhoLst = strtok_r(NULL," \r\t\n",&pTok1);
    }
    
    //Right Sil
    if (bRightSil) {
      MonoPhoLst.push_back(m_MonPhoLst) ;
    }else{
      MonoPhoLst.push_back(SilLst) ;
    }
    MonoPhoState.push_back(0) ;
    
    Z_SAFE_DEL_AR1(pWord_Tmp);
    
    
    //Parse MonoPhoLst to TriPhoLst
    std::vector<ZPhoBatch*> rst ;
    int iStart = 0 ;

    for (int i = 1 ;  i < MonoPhoLst.size() - 1 ; i ++) {
      
      //Get Fix Tri Phone Lst
      std::vector<std::string> pre = MonoPhoLst[i-1] ;
      std::vector<std::string> cur = MonoPhoLst[i] ;
      std::vector<std::string> next = MonoPhoLst[i+1] ;
      std::vector<int > FixTriPhoLst ;
      if (MonoPhoState[i-1] == 2) {
        pre.push_back("sil");
        pre.push_back("<eps>");
      }
      if (MonoPhoState[i] == 2) {
        next.push_back("sil");
        next.push_back("<eps>");
      }
      for (int l = 0 ; l < pre.size() ; l ++){
        for (int m = 0 ; m < cur.size() ; m ++){
          for (int n = 0 ; n < next.size() ; n ++){
            int iTriPhoId = GetTriPhoId(pre[l].c_str(), cur[m].c_str(), next[n].c_str()) ;
            int iFixTriPhoId = m_pTriPho2FixPho[iTriPhoId] ;
            
            if (iFixTriPhoId > -1) {
              bool bExist = false ;
              for (int k = 0 ; k < FixTriPhoLst.size() ; k ++) {
                if (FixTriPhoLst[k] == iFixTriPhoId) {
                  bExist = true ;
                  break ;
                }
              }
              
              if (!bExist) {
                FixTriPhoLst.push_back(iFixTriPhoId);
                //printf("%s-%s+%s: %d %d %d\n", pre[l].c_str(), cur[m].c_str(), next[n].c_str(), m_pFixPhoState[iFixTriPhoId].iPhoStateId[0],m_pFixPhoState[iFixTriPhoId].iPhoStateId[1],m_pFixPhoState[iFixTriPhoId].iPhoStateId[2] );
              }
            }else{
              //printf("Filed to Find %s-%s+%s:\n", pre[l].c_str(), cur[m].c_str(), next[n].c_str());
            }
          }
        }
      }
      
      //Gen PhoBatch
      ZPhoBatch* pPhoBatch = Z_SAFE_NEW(pPhoBatch,ZPhoBatch) ;
      pPhoBatch->iPhoFlag = 0 ;
      
      pPhoBatch->iPhoNum = FixTriPhoLst.size() ;
      pPhoBatch->pPhoList = Z_SAFE_NEW_AR1(pPhoBatch->pPhoList, ZPhoNode, pPhoBatch->iPhoNum);
      
      for (int j = 0 ; j < pPhoBatch->iPhoNum ; j ++) {
        //printf("%d ", FixTriPhoLst[j]);
        memcpy(pPhoBatch->pPhoList + j, m_pFixPhoState + FixTriPhoLst[j], sizeof(ZPhoNode));
      }
      //printf("#%d\n", MonoPhoState[i]) ;
      
      rst.push_back(pPhoBatch);
      
      if (MonoPhoState[i] == 1 || MonoPhoState[i] == 2){
        rst[iStart]->iPhoFlag |= PHO_FLAG_BLOCK_BEGIN ;
        pPhoBatch->iPhoFlag |= PHO_FLAG_BLOCK_END;
        iStart = rst.size() ;
        if (MonoPhoState[i] == 2) {
          pPhoBatch->iPhoFlag |= PHO_FLAG_ENABLE_SIL ;
        }
      }
    }
    
    if (rst.size() > 0)	{
      rst[0]->iPhoFlag |= PHO_FLAG_WORD_BEGIN ;
      rst[rst.size()-1]->iPhoFlag |= PHO_FLAG_WORD_END ;
    }
    
 
    return rst ;
    
  }
  
  int ZPhoTable::LoadOldTable(const char* pMonPhoPath, const char* pFixTriPhoPath, const char* pTriPhoMapPath){
    
    //Get MonoPhone Lst
    char MonPhoName[512] ;
    m_MonPhoLst = GetFileContentLst(pMonPhoPath) ;
    m_iMonPhoNum = m_MonPhoLst.size() ;
    for (int i = 0 ; i < m_iMonPhoNum ; i ++) {
      strcpy(MonPhoName, m_MonPhoLst[i].c_str()) ;
      int len = strlen(MonPhoName) ;
      if (MonPhoName[len-1] == 'a') {
        MonPhoName[len-1] = '1' ;
      }
      if (MonPhoName[len-1] == 'b') {
        MonPhoName[len-1] = '2' ;
      }
      if (MonPhoName[len-1] == 'c') {
        MonPhoName[len-1] = '3' ;
      }
      if (MonPhoName[len-1] == 'd') {
        MonPhoName[len-1] = '4' ;
      }
      if (len > 3 && MonPhoName[len-2] =='_' &&  (MonPhoName[len-1] == 'E' || MonPhoName[len-1] == 'B' || MonPhoName[len-1] == 'S')) {
        if (MonPhoName[len-3] == 'a') {
          MonPhoName[len-3] = '1' ;
        }
        if (MonPhoName[len-3] == 'b') {
          MonPhoName[len-3] = '2' ;
        }
        if (MonPhoName[len-3] == 'c') {
          MonPhoName[len-3] = '3' ;
        }
        if (MonPhoName[len-3] == 'd') {
          MonPhoName[len-3] = '4' ;
        }
      }
      m_MonPhoLst[i] = MonPhoName ;
      m_MonPho2Id[MonPhoName] = i ;
    }
    
    //Get Fix TriPhone Lst
    std::vector<std::string> FixTriPhoLst = GetFileContentLst(pFixTriPhoPath);
    
    m_iTriPhoNum_Fix = 0 ;
    m_pFixPhoState = Z_SAFE_NEW_AR1(m_pFixPhoState, ZPhoNode, FixTriPhoLst.size()) ;
    
    std::map<std::string, int> FixPho2Id ;
    for (short i = 0 ; i < FixTriPhoLst.size() ; i ++) {
      std::vector<std::string> FixPhoInfo = z_str_split(FixTriPhoLst[i].c_str(), " ");
      if (FixPhoInfo.size() == 5) {
        
        //triphone name
        const char* PhoName = FixPhoInfo[0].c_str() ;
        FixPho2Id[PhoName] = m_iTriPhoNum_Fix ;
        
        //monophone id
        int iPhoId_Mono = atoi(FixPhoInfo[1].c_str());
        
        //triphone state
        m_pFixPhoState[m_iTriPhoNum_Fix].iPhoStateId[0] = atoi(FixPhoInfo[2].c_str()) ;
        m_pFixPhoState[m_iTriPhoNum_Fix].iPhoStateId[1] = atoi(FixPhoInfo[3].c_str()) ;
        m_pFixPhoState[m_iTriPhoNum_Fix].iPhoStateId[2] = atoi(FixPhoInfo[4].c_str()) ;
        
        m_iTriPhoNum_Fix ++ ;
      }
    }
    
    //Get Pho Map Lst
    m_iTriPhoNum_Total = m_iMonPhoNum * m_iMonPhoNum * m_iMonPhoNum ;
    m_pTriPho2FixPho = Z_SAFE_NEW_AR1(m_pTriPho2FixPho, int, m_iTriPhoNum_Total) ;
    for (int i = 0 ; i < m_iTriPhoNum_Total ; i ++) {
      m_pTriPho2FixPho[i] = -1 ;
    }
    
    std::vector<std::string> TriPhoMapLst = GetFileContentLst(pTriPhoMapPath) ;
    for (int i = 0 ; i < TriPhoMapLst.size() ; i ++) {
      std::string TriPhoMap = TriPhoMapLst[i] ;
      std::vector<std::string> vv = z_str_split(TriPhoMap.c_str(), " ");
      int id = GetTriPhoId(vv[0].c_str()) ;
      
      if (id < 0) {
        continue ;
      }
      
      if (vv.size() == 1) {
        m_pTriPho2FixPho[id] = FixPho2Id[vv[0]] ;
      }else{
        m_pTriPho2FixPho[id] = FixPho2Id[vv[1]] ;
      }
      
    }
    
    return 0 ;
  }
  
  int ZPhoTable::LoadOldTable2(const char* pMonPhoPath, const char* pFixTriPhoIdPath, const char* pFixTriPhoPath, const char* pTriPhoMapPath){
    
    //Get MonoPhone Lst
    char MonPhoName[512] ;
    m_MonPhoLst = GetFileContentLst(pMonPhoPath) ;
    m_MonPhoLst.push_back("<eps>");
    
    m_iMonPhoNum = m_MonPhoLst.size() ;
    for (int i = 0 ; i < m_iMonPhoNum ; i ++) {
      m_MonPho2Id[m_MonPhoLst[i]] = i ;
    }
    
    std::vector<std::string> FixTriPhIdLst = GetFileContentLst(pFixTriPhoIdPath) ;
    std::map<std::string, std::string> FixTriPhIdMap ;
    for (int i = 0 ; i < FixTriPhIdLst.size() ; i ++) {
      std::vector<std::string> ff = z_str_split(FixTriPhIdLst[i].c_str(), "\r\n \t");
      if (ff.size() == 2) {
        FixTriPhIdMap[ff[1]] = ff[0];
      }
    }
    
    //Get Fix TriPhone Lst
    std::vector<std::string> FixTriPhoLst = GetFileContentLst(pFixTriPhoPath);
    
    m_iTriPhoNum_Fix = 0 ;
    m_pFixPhoState = Z_SAFE_NEW_AR1(m_pFixPhoState, ZPhoNode, FixTriPhoLst.size()) ;
    
    FILE* pFile = fopen("/Users/hadoop/Documents/XCode/test/workdir_cn/newphonetable/cn.hmms.fix.full", "wb");
    
    std::map<std::string, int> FixPho2Id ;
    for (short i = 0 ; i < FixTriPhoLst.size() ; i ++) {
      std::vector<std::string> FixPhoInfo = z_str_split(FixTriPhoLst[i].c_str(), " \r\n\t");
      if (FixPhoInfo.size() == 5) {
        
        //triphone name
        
        std::string fixphoid = FixPhoInfo[0] ;
        assert(FixTriPhIdMap.find(fixphoid) != FixTriPhIdMap.end());
        std::string fixphoname = FixTriPhIdMap[fixphoid] ;
        
        fprintf(pFile, "%s %s %s %s %s\n", fixphoname.c_str(), FixPhoInfo[1].c_str(), FixPhoInfo[2].c_str(), FixPhoInfo[3].c_str(), FixPhoInfo[4].c_str());
        
        FixPho2Id[fixphoname] = m_iTriPhoNum_Fix ;
        
        //monophone id
        int iPhoId_Mono = atoi(FixPhoInfo[1].c_str());
        
        //triphone state
        m_pFixPhoState[m_iTriPhoNum_Fix].iPhoStateId[0] = atoi(FixPhoInfo[2].c_str()) ;
        m_pFixPhoState[m_iTriPhoNum_Fix].iPhoStateId[1] = atoi(FixPhoInfo[3].c_str()) ;
        m_pFixPhoState[m_iTriPhoNum_Fix].iPhoStateId[2] = atoi(FixPhoInfo[4].c_str()) ;
        
        m_iTriPhoNum_Fix ++ ;
      }
    }
    
    fclose(pFile);
    
    //Get Pho Map Lst
    m_iTriPhoNum_Total = m_iMonPhoNum * m_iMonPhoNum * m_iMonPhoNum ;
    m_pTriPho2FixPho = Z_SAFE_NEW_AR1(m_pTriPho2FixPho, int, m_iTriPhoNum_Total) ;
    for (int i = 0 ; i < m_iTriPhoNum_Total ; i ++) {
      m_pTriPho2FixPho[i] = -1 ;
    }
    
    std::vector<std::string> TriPhoMapLst = GetFileContentLst(pTriPhoMapPath) ;
    for (int i = 0 ; i < TriPhoMapLst.size() ; i ++) {
      std::string TriPhoMap = TriPhoMapLst[i] ;
      std::vector<std::string> vv = z_str_split(TriPhoMap.c_str(), " ");
      int id = GetTriPhoId(vv[0].c_str()) ;
      
      if (id < 0) {
        continue ;
      }
      
      if (vv.size() == 1) {
        m_pTriPho2FixPho[id] = FixPho2Id[vv[0]] ;
      }else{
        m_pTriPho2FixPho[id] = FixPho2Id[vv[1]] ;
      }
      
    }
    
    return 0 ;
  }
  
  int ZPhoTable::LoadNewTable(const char* pNewTablePath){
    
    ZInput* pInput = Z_SAFE_NEW(pInput, ZFileInput, pNewTablePath, ZIN_RAW_FORMAT);
    
    //MonPho
    pInput->ReadBasicType(&m_iMonPhoNum) ;
    for (int i = 0 ; i < m_iMonPhoNum ; i ++) {
      std::string tok = pInput->ReadToken() ;
      m_MonPhoLst.push_back(tok) ;
      m_MonPho2Id[tok] = i ;
    }
    
    //TriPho Fix
    pInput->ReadBasicType(&m_iTriPhoNum_Fix) ;
    m_pFixPhoState = Z_SAFE_NEW_AR1(m_pFixPhoState, ZPhoNode, m_iTriPhoNum_Fix);
    pInput->ReadArray(m_pFixPhoState, m_iTriPhoNum_Fix) ;
    
    //TriPhoMap
    pInput->ReadBasicType(&m_iTriPhoNum_Total);
    m_pTriPho2FixPho = Z_SAFE_NEW_AR1(m_pTriPho2FixPho, int, m_iTriPhoNum_Total);
    pInput->ReadArray(m_pTriPho2FixPho, m_iTriPhoNum_Total);
    
    Z_SAFE_DEL(pInput) ;
    
    return 0 ;
  }
  
  
  int ZPhoTable::SaveNewTable(const char* pNewTablePath){
    
    
    ZOutput* pOutput = Z_SAFE_NEW(pOutput, ZFileOutput, pNewTablePath, ZOUT_RAW_FORMAT) ;
    
    //MonPho
    pOutput->WriteBasicType(&m_iMonPhoNum) ;
    for (int i = 0 ; i < m_iMonPhoNum ; i ++) {
      pOutput->WriteToken(m_MonPhoLst[i].c_str());
    }
    
    //TriPho Fix
    pOutput->WriteBasicType(&m_iTriPhoNum_Fix) ;
    pOutput->WriteArray(m_pFixPhoState, m_iTriPhoNum_Fix);
    
    //TriPhoMap
    pOutput->WriteBasicType(&m_iTriPhoNum_Total) ;
    pOutput->WriteArray(m_pTriPho2FixPho, m_iTriPhoNum_Total);
    
    Z_SAFE_DEL(pOutput) ;
    
    return 0 ;
  }
  
  int ZPhoTable::GetTriPhoId(const char* pTriPhoneName){
    
    std::vector<std::string> MonPhoLst = z_str_split(pTriPhoneName, "-+");
    if (MonPhoLst.size() == 1) {
      return  -1 ;
    }
    assert(MonPhoLst.size() == 3) ;
    
    int iTriPhoId = m_iMonPhoNum * m_iMonPhoNum * m_MonPho2Id[MonPhoLst[0]]
    + m_iMonPhoNum * m_MonPho2Id[MonPhoLst[1]]
    + m_MonPho2Id[MonPhoLst[2]] ;
    
    //return m_pTriPho2FixPho[TriPhoId] ;
    return iTriPhoId ;
  }
  
  
  int ZPhoTable::GetTriPhoId(const char* pPre, const char* pCur, const char* Next){
    
    int iTriPhoId = m_iMonPhoNum * m_iMonPhoNum * m_MonPho2Id[pPre]
    + m_iMonPhoNum * m_MonPho2Id[pCur]
    + m_MonPho2Id[Next] ;
    
    //return m_pTriPho2FixPho[TriPhoId] ;
    return iTriPhoId ;
    
  }
  
  //ZVtWord--------------------------------------------------------------
  ZVtWord::ZVtWord(void){
    
    m_iWordType = WORD_OTHER ;
    m_strWordContent = "" ;
    
    m_iBlockMinFrmLen = 3 ;
    m_iBlockMaxFrmLen = 30 ;
    
    m_fBlockAvgScore = 3.0f ;
    m_fBlockMinScore = -1.0f ;
    
    m_bLeftSilCheck = false ;
    m_bRightSilCheck = false ;
    
    m_iBlockNum = 0 ;
    m_iVtStateNum = 0 ;
    m_pVtStateLst = NULL ;
    
  }
  
  ZVtWord::~ZVtWord(void){
    
    for (int i = 0 ; i < m_PhoBatchLst.size() ; i ++) {
      Z_SAFE_DEL_AR1(m_PhoBatchLst[i]->pPhoList);
      Z_SAFE_DEL(m_PhoBatchLst[i]);
    }
    m_PhoBatchLst.clear() ;
    
    for (int i = 0 ; i < m_iVtStateNum ; i ++) {
      Z_SAFE_DEL_AR1(m_pVtStateLst[i]->pCurStateLst);
      Z_SAFE_DEL_AR1(m_pVtStateLst[i]->pPreStateLst);
      Z_SAFE_DEL(m_pVtStateLst[i]);
    }
    Z_SAFE_DEL_AR1(m_pVtStateLst) ;
    
  }
  
  bool ZVtWord::CheckNoCmdChange(bool bAec){
    
    if (bAec && m_bRemoteAsrCheckWithAec) {
      ZLOG_INFO("--------------change to cmd in aec condition")
      return true ;
    }
    if ((!bAec) && m_bRemoteAsrCheckWithNoAec) {
      ZLOG_INFO("--------------change to cmd in no aec condition")
      return true ;
    }
    
    return  false ;
    
  }
  
  
  
  int ZVtWord::PrintWordInfo(){
    
    ZLOG_INFO("------------------------------------------------");
    ZLOG_INFO("WordType:           %d",m_iWordType);
    //ZLOG_INFO("WordContent:      %s",m_WordContent.c_str());
    ZLOG_INFO("BlockMinFrmLen:   %d",m_iBlockMinFrmLen);
    ZLOG_INFO("BlockMaxFrmLen:   %d",m_iBlockMaxFrmLen);
    ZLOG_INFO("BlockAvgScore:    %f",m_fBlockAvgScore);
    ZLOG_INFO("BlockMinScore:    %f",m_fBlockMinScore);
    
    ZLOG_INFO("LeftSilCheck:     %d",m_bLeftSilCheck);
    ZLOG_INFO("LeftSilOffset:    %d",m_iLeftSilOffset);
    ZLOG_INFO("LeftSilSkip:      %d",m_iLeftSilSkip);
    ZLOG_INFO("LeftSilShield:    %f",m_fLeftSilShield);
    
    ZLOG_INFO("RightSilCheck:    %d",m_bRightSilCheck);
    ZLOG_INFO("RightSilOffset:   %d",m_iRightSilOffset);
    ZLOG_INFO("RightSilSkip:     %d",m_iRightSilSkip);
    ZLOG_INFO("RightSilShield:   %f",m_fRightSilShield);
    
    ZLOG_INFO("------------------------------------------------");
    
    
    return  0 ;
  }
  
  int ZVtWord::SetPhoBatchLst(std::vector<ZPhoBatch*> PhoBatchLst){
    
    m_PhoBatchLst = PhoBatchLst ;
    
    std::vector<int> LastSubTriPhoStateidLst , CurSubTriPhoStateidLst ;
    std::vector<ZVtState*> VtStateLst ;
    for (int i = 0 ; i < PhoBatchLst.size() ; i ++){
      LastSubTriPhoStateidLst = CurSubTriPhoStateidLst ;
      CurSubTriPhoStateidLst.clear() ;
      
      //printf("%d %s\n",i,pWordTriPhoLst[i]->pTriPhoInfo);
      
      ZPhoBatch* pPhoBatch = PhoBatchLst[i] ;
      
      for (int k = 0 ; k < 3 ; k ++){
        
        int iStateId = VtStateLst.size() ;
        ZVtState* pVtState = Z_SAFE_NEW(pVtState,ZVtState);
        
        pVtState->iPhoId = i ;
        pVtState->iPhoStateId = k ;
        
        //cur
        pVtState->iCurStateNum = pPhoBatch->iPhoNum * 3  + 5 ;
        pVtState->pCurStateLst = Z_SAFE_NEW_AR1(pVtState->pCurStateLst, int, pVtState->iCurStateNum) ;
        
        int cur = 0 ;
        for (int j = 0 ; j < pPhoBatch->iPhoNum ; j ++){
          int stateid = pPhoBatch->pPhoList[j].iPhoStateId[k] ;
          bool bExist = false ;
          for (int m = 0 ; m < cur ; m ++){
            if (pVtState->pCurStateLst[m] == stateid){
              bExist = true ;
              break;
            }
          }
          if (!bExist){
            pVtState->pCurStateLst[cur] = stateid ;
            cur ++ ;
          }
        }
        if (k == 2 && (pPhoBatch->iPhoFlag & PHO_FLAG_ENABLE_SIL )) {
          for (int j = 1 ; j < 6 ; j ++) {
            int stateid = j ;
            bool bExist = false ;
            for (int m = 0 ; m < cur ; m ++){
              if (pVtState->pCurStateLst[m] == stateid){
                bExist = true ;
                break;
              }
            }
            if (!bExist){
              pVtState->pCurStateLst[cur] = stateid ;
              cur ++ ;
            }
          }
        }
//        if (k == 2 || k == 0) {
//          for (int j = 0 ; j < pPhoBatch->iPhoNum ; j ++){
//            int stateid = pPhoBatch->pPhoList[j].iPhoStateId[1] ;
//            bool bExist = false ;
//            for (int m = 0 ; m < cur ; m ++){
//              if (pVtState->pCurStateLst[m] == stateid){
//                bExist = true ;
//                break;
//              }
//            }
//            if (!bExist){
//              pVtState->pCurStateLst[cur] = stateid ;
//              cur ++ ;
//            }
//          }
//        }
        pVtState->iCurStateNum = cur ;
        
        //pre
        pVtState->iStateFlag = 0 ;
        if (k == 0) {
          if (pPhoBatch->iPhoFlag & PHO_FLAG_WORD_BEGIN){
            pVtState->iStateFlag |= PHO_FLAG_WORD_BEGIN ;
          }
          if (pPhoBatch->iPhoFlag & PHO_FLAG_BLOCK_BEGIN){
            pVtState->iStateFlag |= PHO_FLAG_BLOCK_BEGIN ;
          }
        }else if(k == 2){
          if (pPhoBatch->iPhoFlag & PHO_FLAG_WORD_END){
            pVtState->iStateFlag |= PHO_FLAG_WORD_END ;
          }
          if (pPhoBatch->iPhoFlag & PHO_FLAG_BLOCK_END){
            pVtState->iStateFlag |= PHO_FLAG_BLOCK_END ;
          }
          CurSubTriPhoStateidLst.push_back(iStateId);
        }
        
        pVtState->iPreStateNum = 0 ;
        pVtState->pPreStateLst = NULL ;
        
        if (k == 0){
          pVtState->iPreStateNum = LastSubTriPhoStateidLst.size() + 1 ;
          pVtState->pPreStateLst = Z_SAFE_NEW_AR1(pVtState->pPreStateLst, int, pVtState->iPreStateNum);
          pVtState->pPreStateLst[0] = iStateId ;
          for (int m = 1 ; m < pVtState->iPreStateNum ; m ++){
            pVtState->pPreStateLst[m] = LastSubTriPhoStateidLst[m-1] ;
          }
        }else{
          pVtState->iPreStateNum = 2 ;
          pVtState->pPreStateLst = Z_SAFE_NEW_AR1(pVtState->pPreStateLst, int, pVtState->iPreStateNum);
          pVtState->pPreStateLst[0] = iStateId ;
          pVtState->pPreStateLst[1] = iStateId - 1 ;
        }
        VtStateLst.push_back(pVtState);
      }
    }
    
    m_iBlockNum = 0 ;
    m_iVtStateNum = VtStateLst.size() ;
    m_pVtStateLst = Z_SAFE_NEW_AR1(m_pVtStateLst, ZVtState*, m_iVtStateNum);
    for (int i = 0 ; i < m_iVtStateNum ; i ++) {
      m_pVtStateLst[i] = VtStateLst[i];
//      printf("%d %d %d:\n",i/3,i%3, m_pVtStateLst[i]->iStateFlag);
//      printf("CurStateLst %d:", m_pVtStateLst[i]->iCurStateNum);
//      for (int j = 0 ; j < m_pVtStateLst[i]->iCurStateNum ; j ++) {
//        printf(" %d", m_pVtStateLst[i]->pCurStateLst[j]);
//      }
//      printf("\n");
//      
//      printf("PreStateLst %d:", m_pVtStateLst[i]->iPreStateNum);
//      for (int j = 0 ; j < m_pVtStateLst[i]->iPreStateNum ; j ++) {
//        printf(" %d", m_pVtStateLst[i]->pPreStateLst[j]);
//      }
//      printf("\n");
      
      if (m_pVtStateLst[i]->iStateFlag & PHO_FLAG_BLOCK_END) {
        m_iBlockNum ++ ;
      }
    }
    
    return  0 ;
  }
  
};




