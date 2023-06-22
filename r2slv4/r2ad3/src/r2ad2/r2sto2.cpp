//
//  r2sto2.cpp
//  r2ad
//
//  Created by hadoop on 10/22/15.
//  Copyright (c) 2015 hadoop. All rights reserved.
//

#include "r2sto2.h"

#include "r2ssp.h"
#include "NNVadIntf.h"
#include "../../../r2vt4/src/vt/zvtapi.h"

r2sto2::r2sto2(const char* pWorkDir)
{
  
  char config[256], line[256];
  sprintf(config,"%s/r2ssp.cfg",pWorkDir);
  
  std::string vv ;
  std::vector<std::string> vs ;
  
  //mic
  m_iMicNum = r2_getkey_int(config,"r2ssp","r2ssp.mic.num", 16);
  m_pMicPos = R2_SAFE_NEW_AR1(m_pMicPos, float, m_iMicNum * 3);
  for (int i = 0 ; i < m_iMicNum ; i ++){
    sprintf(line,"r2ssp.mic.pos.%d",i);
    vv = r2_getkey(config,"r2ssp",line);
    vs = r2_strsplit(vv.c_str(),",");
    assert(vs.size() == 3);
    for (int j = 0 ; j < 3 ; j ++){
      m_pMicPos[i*3+j] = atof(vs[j].c_str());
    }
  }
  
  m_pMicI2sDelay = R2_SAFE_NEW_AR1(m_pMicI2sDelay, float, m_iMicNum);
  if ((m_iMicNum == 8) || (m_iMicNum == 6)) {
    for (int i = 0 ; i < m_iMicNum; i ++) {
      if (i % 2 == 1) {
        m_pMicI2sDelay[i] = 1.0f / 96000.0f ;
      }
    }
  }
  
  //In
  m_pMicInfo_In = r2_getmicinfo(config,"r2ssp","r2ssp.aec.mics");
  
  //bf
  m_pMicInfo_Bf = r2_getmicinfo(config,"r2ssp","r2ssp.sl.mics");
  
  //cod
  m_bCod = r2_getkey_bool(config,"r2ssp","r2ssp.codec",true) ;
  
  
  VAD_SysInit();
  ZLOG_INFO("---------------------------------------------Vad initok");

  r2ssp_ssp_init();
  ZLOG_INFO("---------------------------------------------r2ssp initok");
  
  //Comparable
  InitPath(pWorkDir);
  
}

r2sto2::~r2sto2(void)
{
  r2_free_micinfo(m_pMicInfo_In);
  r2_free_micinfo(m_pMicInfo_Bf);
  
  R2_SAFE_DEL_AR1(m_pMicPos);
  R2_SAFE_DEL_AR1(m_pMicI2sDelay);
  
  R2_SAFE_DEL_AR1(m_pWordLst);
  
  r2ssp_ssp_exit();
  VAD_SysExit();
}


int r2sto2::InitPath(const char* pWorkDir){
  
  char CfgPath[512], Tag[50] ;
  sprintf(CfgPath, "%s/%s", pWorkDir,"vt.new.cfg");
  
  m_strVtNnetPath =  r2_getkey_path(pWorkDir, CfgPath, "system", "vt.new.dnn");
  m_strVtPhoneTablePath = r2_getkey_path(pWorkDir, CfgPath, "system", "vt.new.pho");
  
  m_iWordNum = r2_getkey_int(CfgPath,"system","vt.words.num",0) ;
  m_pWordLst = R2_SAFE_NEW_AR1(m_pWordLst, WordInfo, m_iWordNum);
  
  int iWordCur = 0 ;
  for (int i = 0 ; i < m_iWordNum ; i ++) {
    WordInfo*pVtWord = m_pWordLst + iWordCur ;
    
    sprintf(Tag, "vt.word.%d.id",i);
    int iWordType = r2_getkey_int(CfgPath,"system",Tag,-1) ;
    if (iWordType == 0) {
      pVtWord->iWordType = WORD_AWAKE ;
    }else if(iWordType == 1){
      pVtWord->iWordType = WORD_SLEEP ;
    }else{
      continue ;
    }
    
    sprintf(Tag, "vt.word.%d.content",i) ;
    strcpy(pVtWord->pWordContent_UTF8, r2_getkey(CfgPath,"system",Tag).c_str()) ;
    
    sprintf(Tag, "vt.word.%d.pholst",i);
    strcpy(pVtWord->pWordContent_PHONE, r2_getkey(CfgPath,"system",Tag).c_str()) ;
    
    sprintf(Tag, "vt.word.%d.block.score.avg",i);
    pVtWord->fBlockAvgScore = r2_getkey_float(CfgPath,"system",Tag,3.0f) ;
    
    sprintf(Tag, "vt.word.%d.block.score.min",i);
    pVtWord->fBlockMinScore = r2_getkey_float(CfgPath,"system",Tag,-1.0f) ;
    
    sprintf(Tag, "vt.word.%d.sil.left.check",i);
    pVtWord->bLeftSilDet = r2_getkey_bool(CfgPath,"system",Tag,false);
    
    sprintf(Tag, "vt.word.%d.sil.right.check",i);
    pVtWord->bRightSilDet = r2_getkey_bool(CfgPath,"system",Tag,false) ;
    
    if (iWordType == 0) {
      pVtWord->bRemoteAsrCheckWithAec = true ;
      pVtWord->bRemoteAsrCheckWithNoAec = false ;
    }else if (iWordType == 1){
      pVtWord->bRemoteAsrCheckWithAec = false ;
      pVtWord->bRemoteAsrCheckWithNoAec = false ;
    }else{
      pVtWord->bRemoteAsrCheckWithAec = true ;
      pVtWord->bRemoteAsrCheckWithNoAec = true ;
    }
    
    sprintf(Tag, "vt.word.%d.classify.check",i);
    pVtWord->bLocalClassifyCheck = r2_getkey_bool(CfgPath,"system",Tag,false) ;
    
    sprintf(Tag, "vt.word.%d.classify.model.shield",i);
    pVtWord->fClassifyShield = r2_getkey_float(CfgPath,"system",Tag,-0.3f) ;
    
    sprintf(Tag, "vt.word.%d.classify.model.name",i);
    strcpy(pVtWord->pLocalClassifyNnetPath, r2_getkey_path(pWorkDir, CfgPath, "system", Tag).c_str()) ;
    
    iWordCur ++ ;
  }
  
  m_iWordNum = iWordCur ;

  
  return 0 ;
  
}
