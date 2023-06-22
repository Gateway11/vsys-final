//
//  r2mem2.cpp
//  r2ad
//
//  Created by hadoop on 10/22/15.
//  Copyright (c) 2015 hadoop. All rights reserved.
//

#include "r2mem2.h"
#include <unistd.h>

r2mem2::r2mem2(r2sto2* pSto){
  
  m_pSto = pSto ;
  m_iMicNum = pSto->m_iMicNum ;
  
  //In
  
  m_pMem_In = R2_SAFE_NEW(m_pMem_In, r2mem_i, m_pSto->m_iMicNum, r2_in_float_32, m_pSto->m_pMicInfo_In);
  
//  //vbv
  m_pMem_Vbv3 = R2_SAFE_NEW(m_pMem_Vbv3, r2mem_vbv3, m_pSto->m_iMicNum,m_pSto->m_pMicPos, m_pSto->m_pMicI2sDelay,
                            m_pSto->m_pMicInfo_Bf, m_pSto->m_strVtNnetPath.c_str(), m_pSto->m_strVtPhoneTablePath.c_str()) ;

  m_pMem_Vbv3->SetWords(m_pSto->m_pWordLst, m_pSto->m_iWordNum);
  
  
  //bf
  m_pMem_Bf = R2_SAFE_NEW(m_pMem_Bf, r2mem_bf, m_pSto->m_iMicNum, m_pSto->m_pMicPos, m_pSto->m_pMicI2sDelay, m_pSto->m_pMicInfo_Bf);
  
  //vad2
  m_pMem_Vad2 = R2_SAFE_NEW(m_pMem_Vad2, r2mem_vad2, m_pSto->m_fBaseRange, m_pSto->m_fMinDynaRange, m_pSto->m_fMaxDynaRange) ;
  
  //cod
  if (m_pSto->m_bCod) {
    m_pMem_Cod = R2_SAFE_NEW(m_pMem_Cod, r2mem_cod, r2ad_cod_opu);
  }else{
    m_pMem_Cod = R2_SAFE_NEW(m_pMem_Cod, r2mem_cod, r2ad_cod_pcm);
  }
  
  //state
  m_bAsr = false ;
  m_bVadStart = false ;
  m_bDataOutput = false ;
  m_bCanceled = false ;
  m_bAwake = false ;
  
  //debug
  m_strDebugFolder =  DEBUG_FILE_LOCATION;
  r2_mkdir(m_strDebugFolder.c_str());
  
#ifdef _MEM2_RAW_FILE
  char rawpath[256];
  sprintf(rawpath, "%s/raw2.pcm",m_strDebugFolder.c_str());
  m_pRawFile = fopen(rawpath, "wb");
#endif
  
#ifdef _BF_AUDIO
  char bfpath[256];
  sprintf(bfpath,"%s/bf.pcm",m_strDebugFolder.c_str());
  m_pBfFile = fopen(bfpath, "wb");
#endif
  
#ifdef _BF_RAW_AUDIO
  char bfrawpath[256];
  sprintf(bfrawpath,"%s/bfraw.pcm",m_strDebugFolder.c_str());
  m_pBfRawFile = fopen(bfrawpath, "wb");
#endif
  
#ifdef _VAD_AUDIO
  char vadpath[256];
  sprintf(vadpath,"%s/vad/",m_strDebugFolder.c_str());
  r2_mkdir(vadpath);
#endif
  
#ifdef  _Debug_Opu_Audio
  char opupath[256];
  sprintf(opupath,"%s/opu/",m_strDebugFolder.c_str());
  r2_mkdir(opupath);
  m_pDebugOpuFile = NULL ;
#endif
  
  //MsgLst
  m_iMsgNum_Cur = 0 ;
  m_iMsgNum_Total = 1000 ;
  m_pMsgLst =  R2_SAFE_NEW_AR1(m_pMsgLst, r2ad_msg_block*, m_iMsgNum_Total) ;
  
  
  m_iCol_NoNew = R2_AUDIO_SAMPLE_RATE * 5  ;
  m_pData_NoNew = R2_SAFE_NEW_AR2(m_pData_NoNew, float, m_iMicNum, m_iCol_NoNew) ;
  
  //CurentStatus
  m_iFrmSize = R2_AUDIO_SAMPLE_RATE / 1000 * R2_AUDIO_FRAME_MS ;
  m_iLastDuration = 0 ;
  m_iAecFlag = -1 ;
  m_iAwakeFlag = -1 ;
  m_iSleepFlasg = -1 ;
  m_iAsrFlag = -1 ;
  
  m_bFirstFrm = true ;
  
  m_iAsrMsgCheckFlag = 0 ;
  
}

r2mem2::~r2mem2(void)
{
  
  R2_SAFE_DEL(m_pMem_In);
  
  R2_SAFE_DEL(m_pMem_Vbv3);
  
  R2_SAFE_DEL(m_pMem_Bf);
  
  R2_SAFE_DEL(m_pMem_Vad2);
  
  R2_SAFE_DEL(m_pMem_Cod);
  
#ifdef _BF_AUDIO
  fclose(m_pBfFile);
#endif
  
#ifdef _BF_RAW_AUDIO
  fclose(m_pBfRawFile);
#endif
  
#ifdef _MEM2_RAW_FILE
  fclose(m_pRawFile);
#endif
  
  //MsgLst
  ClearMsgLst() ;
  
  R2_SAFE_DEL_AR1(m_pMsgLst) ;
  R2_SAFE_DEL_AR2(m_pData_NoNew) ;
  
  
}

int r2mem2::ProcessData(char* pData_In, int iLen_In, int iAecFlag, int iAwakeFlag, int iSleepFlag, int iAsrFlag, int iHotwordFlag){
  
  assert(iLen_In == 0 || (iLen_In > 0 && pData_In != NULL)) ;
  
#ifdef _MEM2_RAW_FILE
  fwrite(pData_In, sizeof(char), iLen_In , m_pRawFile);
#endif
  
  ClearMsgLst() ;

  if (iLen_In == 0) {
    return  0 ;
  }
  
  bool bAec = (iAecFlag == 1) ;
  bool bAsr = (iAsrFlag == 1)   ;
  bool bAwake = (iAwakeFlag == 1) ;
  bool bSleep = (iSleepFlag == 1) ;
  bool bHotWord = (iHotwordFlag == 1) ;
  
  if (bAsr ) {
    m_bAsr = true ;
  }
  
  //prepare data
  int rt = 0 ;
  float datatmp = 0.0f ;
  
  float** pData_Mul = NULL , *pData_Sig = NULL;
  int iLen_Mul = 0 , iLen_Sig = 0 ;
  
  //In
  m_pMem_In->process(pData_In, iLen_In, pData_Mul, iLen_Mul);
  
  
  //fix mic
  if (m_bFirstFrm && iLen_Mul > 0) {
    m_bFirstFrm = false ;
    r2_mic_info* pMicErr = geterrorinfo(pData_Mul, iLen_Mul) ;
    if (r2_fixerrmix(m_pSto->m_pMicInfo_Bf, pMicErr) == 1) {
      R2_SAFE_DEL(m_pMem_Vbv3);
      R2_SAFE_DEL(m_pMem_Bf);
      
      m_pMem_Vbv3 = R2_SAFE_NEW(m_pMem_Vbv3, r2mem_vbv3, m_pSto->m_iMicNum,m_pSto->m_pMicPos, m_pSto->m_pMicI2sDelay,
                                m_pSto->m_pMicInfo_Bf, m_pSto->m_strVtNnetPath.c_str(), m_pSto->m_strVtPhoneTablePath.c_str()) ;
      m_pMem_Bf = R2_SAFE_NEW(m_pMem_Bf, r2mem_bf, m_pSto->m_iMicNum, m_pSto->m_pMicPos, m_pSto->m_pMicI2sDelay, m_pSto->m_pMicInfo_Bf);
      m_pMem_Vbv3->SetWords(m_pSto->m_pWordLst, m_pSto->m_iWordNum);
      r2_free_micinfo(pMicErr);
    }
  }
  
  PrintStatus(iLen_Mul, iAecFlag, iAwakeFlag, iSleepFlag, iAsrFlag);

  //bf
  m_pMem_Bf->process(pData_Mul, iLen_Mul, pData_Sig, iLen_Sig);

#ifdef  _BF_AUDIO
  for (int k = 0 ; k < iLen_Sig; k ++) {
    datatmp = pData_Sig[k] ;
    fwrite(&datatmp,sizeof(float),1,m_pBfFile);
  }
#endif
  
#ifdef  _BF_RAW_AUDIO
  for (int k = 0 ; k < iLen_Sig; k ++) {
    datatmp = pData_Sig[k] ;
    fwrite(&datatmp,sizeof(float),1,m_pBfRawFile);
  }
#endif
  
  
  //vbv
  int rt_vbv = m_pMem_Vbv3->Process(pData_Mul, iLen_Mul, m_bDataOutput, bAec, bAwake, bSleep, bHotWord);
  //int rt_vbv = 0 ;
  
  if ((rt_vbv & R2_VT_WORD_CANCEL) != 0) {
    assert((rt_vbv & R2_VT_WORD_PRE) != 0 ) ;
  }
  
  bool bPre = ((rt_vbv & R2_VT_WORD_PRE) != 0 && (rt_vbv & R2_VT_WORD_CANCEL) == 0) ;
  bool bAwakePre = bAwake && bPre && (m_pMem_Vbv3->m_pWordInfo->iWordType == WORD_AWAKE) ;
  bool bAwakeNoCmd = bAwake && ((rt_vbv & R2_VT_WORD_DET_NOCMD) != 0 && m_pMem_Vbv3->m_pWordInfo->iWordType == WORD_AWAKE) ;
  bool bAwakeCmd = bAwake && ((rt_vbv & R2_VT_WORD_DET_CMD) != 0 && m_pMem_Vbv3->m_pWordInfo->iWordType == WORD_AWAKE) ;
  
  bool bSleepNoCmd = bSleep && ((rt_vbv & R2_VT_WORD_DET_NOCMD) != 0 && m_pMem_Vbv3->m_pWordInfo->iWordType == WORD_SLEEP) ;
  bool bSleepCmd = bSleep && ((rt_vbv & R2_VT_WORD_DET_CMD) != 0 && m_pMem_Vbv3->m_pWordInfo->iWordType == WORD_SLEEP) ;
  
  bool bHotwordNoCmd = bHotWord && ((rt_vbv & R2_VT_WORD_DET_NOCMD) != 0 && m_pMem_Vbv3->m_pWordInfo->iWordType == WORD_HOTWORD) ;
  bool bHotwordCmd = bHotWord && ((rt_vbv & R2_VT_WORD_DET_CMD) != 0 && m_pMem_Vbv3->m_pWordInfo->iWordType == WORD_HOTWORD) ;
  
  bool bCmd = bAwakeCmd || bSleepCmd || bHotwordCmd ;
  bool bNoCmd = bAwakeNoCmd || bSleepNoCmd || bHotwordNoCmd ;
  
  
  int iForceStart = 0 ;
  if (bPre) {
    
    ResetAsr() ;
    
    memcpy(m_pSlInfo_BK, m_pMem_Bf->m_fSlInfo, sizeof(float) * 3) ;
    m_pMem_Bf->steer(m_pMem_Vbv3->m_pWordDetInfo->fWordSlInfo[0], m_pMem_Vbv3->m_pWordDetInfo->fWordSlInfo[1]);
    
    //Reset
    int iStart = m_pMem_Vbv3->m_pWordDetInfo->iWordPos_Start , iEnd = m_pMem_Vbv3->m_pWordDetInfo->iWordPos_End ;
    
    //need 50ms for vad bug
    iStart += 20 * m_iFrmSize ;
    
    if (iStart > m_iCol_NoNew) {
      m_iCol_NoNew = iStart * 2 ;
      R2_SAFE_DEL_AR2(m_pData_NoNew);
      m_pData_NoNew = R2_SAFE_NEW_AR2(m_pData_NoNew, float, m_iMicNum, m_iCol_NoNew) ;
    }
    
    m_pMem_Vbv3->GetLastAudio(m_pData_NoNew, iStart, 0) ;
    m_pMem_Bf->process(m_pData_NoNew, iStart, pData_Sig, iLen_Sig);
    
    pData_Sig += 15 * m_iFrmSize ;
    iLen_Sig -= 15 * m_iFrmSize ;
    
    iForceStart = 1 ;
  }
  
  int rt_vad2 = m_pMem_Vad2->process(pData_Sig, iLen_Sig, 0, 0, iForceStart,  pData_Sig, iLen_Sig);
  
  if (rt_vad2 & r2vad_audio_begin){
    
    //ZLOG_INFO("----------%f",m_pMem_Vad2->getenergy_Threshold());
    
    m_bAsr = bAsr ;
    m_bVadStart = true ;
    m_bDataOutput = false ;
    m_bCanceled = false ;
    m_bAwake = false ;
    
    m_pMem_Cod->reset() ;
    
    if (bAwakePre) {
      m_bAwake = true ;
      AddMsgWithSlInfo(r2ad_awake_pre, m_pMem_Bf->getinfo_sl());
      m_str_AwakeInfo = m_pMem_Vbv3->m_pWordInfo->pWordContent_UTF8 ;
    }
    
#ifdef  _BF_AUDIO
    datatmp = 10000000.0f ;
    fwrite(&datatmp,sizeof(float),1,m_pBfFile);
    for (int k = 0 ; k < iLen_Sig; k ++) {
      datatmp = pData_Sig[k] ;
      fwrite(&datatmp,sizeof(float),1,m_pBfFile);
    }
#endif
    
  }
  
  if (bNoCmd) {
    if (bSleepNoCmd) {
      AddMsgWithSlInfo(r2ad_sleep, m_pMem_Bf->getinfo_sl());
    }
    if (bAwakeNoCmd) {
      AddMsgWithSlInfo(r2ad_awake_nocmd, m_pMem_Bf->getinfo_sl());
    }
    if (bHotwordNoCmd) {
      AddMsgWithSlInfo(r2ad_hotword, m_pMem_Bf->getinfo_sl());
    }
    
    m_pMem_Vad2->setvadendparam(-1);
    
  }
  
  if (bCmd) {
    if (bSleepCmd) {
      //AddMsgWithSlInfo(r2ad_sleep, m_pMem_Bf->getinfo_sl());
    }
    if (bAwakeCmd) {
      AddMsgWithSlInfo(r2ad_awake_cmd, m_pMem_Bf->getinfo_sl());
    }
    if (bHotwordCmd) {
      //AddMsgWithSlInfo(r2ad_hotword, m_pMem_Bf->getinfo_sl());
    }
    
    m_pMem_Vad2->setvadendparam(-1);
  }

  
  
  if (m_bVadStart) {
    
    m_pMem_Cod->process(pData_Sig, iLen_Sig);
    
    //Check Asr Output
    if (!m_bDataOutput ) {
      if (bCmd | bNoCmd) {
        m_bDataOutput = true ;
        if (bCmd) {
          ZLOG_INFO("---------------------Vad Output with awake pre");
          AddMsgWithSlInfo(r2ad_vad_start, m_str_AwakeInfo.c_str());
          //AddMsgWithSlInfo(r2ad_vad_start, m_pMem_Bf->getinfo_sl());

        }else{
          m_bCanceled = true ;
          m_pMem_Cod->pause();
        }
      }else if(m_bAsr && !m_bAwake) {
        //ZLOG_INFO("---------------------Vad Output with asr");
        float SlInfo[3] ;
        m_pMem_Vbv3->GetRealSl(36, SlInfo);
        if (m_pMem_Bf->check(SlInfo[0] , SlInfo[1])) {
          ZLOG_INFO("---------------------Vad Output with asr + check sl");
          m_bDataOutput = true ;
          AddMsgWithSlInfo(r2ad_vad_start, "");
          //AddMsgWithSlInfo(r2ad_vad_start, m_pMem_Bf->getinfo_sl());
        }
      }
    }
    
    //Check Asr Cancel
    if (m_bDataOutput ) {
      
      //mid  cancel
      if (!m_bCanceled) {
        
        //Cancel with too long
        if (!m_bAwake && m_pMem_Cod->istoolong()) {
          m_bCanceled = true ;
          ZLOG_INFO("---------------------Vad Cancel ASR Too Long") ;
        }
        
        if (!m_bAwake && !bAsr) {
          m_bCanceled = true ;
          ZLOG_INFO("---------------------Vad Cancel With No Asr Flag ") ;
        }
        
        if (m_bCanceled) {
          m_pMem_Cod->pause();
          AddMsg(r2ad_vad_cancel, 0, NULL);
        }
        
      }
      
      if (m_bCanceled && m_bAwake) {
        if (m_pMem_Cod->isneedresume()) {
          ZLOG_INFO("---------------------Vad Output with awake and too long");
          m_bCanceled = false ;
          m_pMem_Cod->resume() ;
          AddMsgWithSlInfo(r2ad_vad_start, m_str_AwakeInfo.c_str());
          //AddMsgWithSlInfo(r2ad_vad_start, m_pMem_Bf->getinfo_sl());
        }
      }
      
      if (!m_bCanceled) {
        if (!bPre && (m_pMem_Cod->getdatalen() > 100)) {
          AddMsgWithAudio(r2ad_vad_data, m_pMem_Cod);
        }
        if (rt_vad2 & r2vad_audio_end) {
          if (m_pMem_Cod->getdatalen() > 0) {
            AddMsgWithAudio(r2ad_vad_data, m_pMem_Cod);
          }
        }
      }
      
    }
    
  }
  
  if (rt_vad2 & r2vad_audio_end) {

    m_pMem_Vad2->setvadendparam(-1);
    
    if (m_bDataOutput && !m_bCanceled) {
      AddMsg(r2ad_vad_end, 0, NULL);
    }
    
    if (m_bAwake) {
      m_pMem_Cod->pause() ;
      m_pMem_Cod->resume() ;
      AddMsgWithAudio(r2ad_debug_audio, m_pMem_Cod);
    }
    
    m_bAwake = false ;
    m_bAsr = false ;
    m_bVadStart = false ;
    m_bDataOutput = false ;
    m_bCanceled = false ;
    
#ifdef  _BF_AUDIO
    datatmp = -10000000.0f ;
    fwrite(&datatmp,sizeof(float),1,m_pBfFile);
#endif
    
  }
  
  return rt ;
}

int r2mem2::reset(){
  
  //vbv
  //m_pMem_Vbv->reset() ;
  
  //bf
  //m_pMem_Bf->reset() ;
  
  //vad2
  m_pMem_Vad2->reset() ;
  
  //cod
  m_pMem_Cod->reset();
  
  //state
  m_bAsr = false ;
  m_bDataOutput = false;
  m_bVadStart  = false ;
  
  return 0 ;
}

int r2mem2::GetMsgLst(r2ad_msg_block** &pMsgLst, int &iMsgNum){
  
  pMsgLst = m_pMsgLst ;
  iMsgNum = m_iMsgNum_Cur ;
  
  for (int i = 0 ; i < m_iMsgNum_Cur ; i ++) {
    PrintMsgInfo(m_pMsgLst[i]);
  }
  
  
  return  0 ;
}

int r2mem2::AddMsg(enum r2ad_msg iMsgId, int iMsgDataLen ,const char* pMsgData){
  
  
  if (m_iMsgNum_Cur + 1 > m_iMsgNum_Total) {
    m_iMsgNum_Total = (m_iMsgNum_Cur + 1) * 2 ;
    r2ad_msg_block** pMsgLst = R2_SAFE_NEW_AR1(pMsgLst, r2ad_msg_block*, m_iMsgNum_Total);
    memcpy(pMsgLst, m_pMsgLst, sizeof(r2ad_msg_block*) * m_iMsgNum_Cur) ;
    R2_SAFE_DEL_AR1(m_pMsgLst) ;
    m_pMsgLst = pMsgLst ;
  }
  
  //add New msg
  r2ad_msg_block* pNewMsg = R2_SAFE_NEW(pNewMsg, r2ad_msg_block);
  pNewMsg->iMsgId = iMsgId ;
  pNewMsg->iMsgDataLen = iMsgDataLen ;
  
  if (iMsgDataLen > 0) {
    pNewMsg->pMsgData = R2_SAFE_NEW_AR1(pNewMsg->pMsgData, char, iMsgDataLen);
    memcpy(pNewMsg->pMsgData, pMsgData, sizeof(char) * iMsgDataLen) ;
  }else{
    pNewMsg->pMsgData = NULL ;
  }
  
  
  m_pMsgLst[m_iMsgNum_Cur] = pNewMsg ;
  m_iMsgNum_Cur ++ ;
  
  return  0 ;
}

int r2mem2::AddMsgWithSlInfo(enum r2ad_msg iMsgId, const char* pSlInfo){
  
  assert(pSlInfo != NULL);
  
  return  AddMsg(iMsgId, strlen(pSlInfo) + 1, pSlInfo) ;
}

int r2mem2::AddMsgWithAudio(enum r2ad_msg iMsgId, r2mem_cod* pMem_Cod){
  
  char* pData = NULL ;
  int iLen = 0 ;
  pMem_Cod->getdata2(pData, iLen);
  return  AddMsg(iMsgId, iLen, pData) ;
}

int r2mem2::PrintMsgInfo(r2ad_msg_block* pMsg){
  
  char opupath[256];
  std::string dt = r2_getdatatime() ;
  
  switch (pMsg->iMsgId) {
    case r2ad_vad_start:
      ZLOG_INFO("r2ad_vad_start: %s",pMsg->pMsgData);
      if (m_iAsrMsgCheckFlag != 0) {
        ZLOG_INFO("---------------------ASRFLAG ERRORERROR");
      }
      m_iAsrMsgCheckFlag = 1;
#ifdef  _Debug_Opu_Audio
      assert(m_pDebugOpuFile == NULL);
      sprintf(opupath,"%s/opu/%s_Debug.pcm", m_strDebugFolder.c_str(),dt.c_str());
      m_pDebugOpuFile = fopen(opupath, "wb");
#endif
      break;
    case r2ad_vad_data:
      ZLOG_INFO("r2ad_vad_data DataLen: %d",pMsg->iMsgDataLen);
      if (m_iAsrMsgCheckFlag != 1) {
        ZLOG_INFO("---------------------ASRFLAG ERRORERROR");
      }
#ifdef  _Debug_Opu_Audio
      assert(m_pDebugOpuFile != NULL);
      fwrite(pMsg->pMsgData, 1, pMsg->iMsgDataLen, m_pDebugOpuFile);
#endif
      break;
    case r2ad_vad_end:
      ZLOG_INFO("r2ad_vad_end ");
      if (m_iAsrMsgCheckFlag != 1) {
        ZLOG_INFO("---------------------ASRFLAG ERRORERROR");
      }
      m_iAsrMsgCheckFlag = 0 ;
#ifdef  _Debug_Opu_Audio
      assert(m_pDebugOpuFile != NULL);
      fclose(m_pDebugOpuFile);
      m_pDebugOpuFile = NULL ;
#endif
      break;
    case r2ad_vad_cancel:
      ZLOG_INFO("r2ad_vad_cancel ");
      if (m_iAsrMsgCheckFlag != 1) {
        ZLOG_INFO("---------------------ASRFLAG ERRORERROR");
      }
      m_iAsrMsgCheckFlag = 0 ;
#ifdef  _Debug_Opu_Audio
      assert(m_pDebugOpuFile != NULL);
      fclose(m_pDebugOpuFile);
      m_pDebugOpuFile = NULL ;
#endif
      break;
    case r2ad_awake_vad_start:
      ZLOG_INFO("r2ad_awake_vad_start ");
      break;
    case r2ad_awake_vad_data:
      ZLOG_INFO("r2ad_awake_vad_data DataLen: %d",pMsg->iMsgDataLen);
      break;
    case r2ad_awake_vad_end:
      ZLOG_INFO("r2ad_awake_vad_end ");
      break;
    case r2ad_awake_pre:
      ZLOG_INFO("r2ad_awake_pre SL: %s",pMsg->pMsgData);
      break;
    case r2ad_awake_nocmd:
      ZLOG_INFO("r2ad_awake_nocmd SL: %s",pMsg->pMsgData);
      break;
    case r2ad_awake_cmd:
      ZLOG_INFO("r2ad_awake_cmd SL: %s",pMsg->pMsgData);
      break;
    case r2ad_awake_cancel:
      ZLOG_INFO("r2ad_awake_cancel SL: %s",pMsg->pMsgData);
      break;
    case r2ad_sleep:
      ZLOG_INFO("r2ad_sleep ");
      break;
    case r2ad_hotword:
      ZLOG_INFO("r2ad_hotword hotword: %s",pMsg->pMsgData);
      break;
    case r2ad_debug_audio:
      ZLOG_INFO("r2ad_debug_audio DataLen: %d",pMsg->iMsgDataLen);
      break;
    default:
      break;
  }
  
  return 0 ;
}

int r2mem2::ClearMsgLst(){
  
  for (int i = 0 ; i < m_iMsgNum_Cur ; i ++) {
    R2_SAFE_DEL_AR1(m_pMsgLst[i]->pMsgData);
    R2_SAFE_DEL(m_pMsgLst[i]) ;
  }
  
  m_iMsgNum_Cur = 0 ;
  
  return  0 ;
}

int r2mem2::ResetAsr(){
  
  //Reset
  if (m_bDataOutput && !m_bCanceled) {
    AddMsg(r2ad_vad_cancel, 0, NULL);
  }
  
  m_bAsr = false ;
  m_bVadStart = false ;
  m_bDataOutput = false ;
  m_bCanceled = false ;
  m_bAwake = false ;
  
  m_pMem_Vad2->reset() ;
  m_pMem_Cod->reset() ;
  m_pMem_Bf->reset() ;
  
  return  0 ;
  
}


void r2mem2::PrintStatus(int iLen_In, int iAecFlag, int iAwakeFlag, int iSleepFlag, int iAsrFlag){
  
  if ( iAwakeFlag != m_iAwakeFlag || iSleepFlag != m_iSleepFlasg || iAsrFlag != m_iAsrFlag || iAecFlag != m_iAecFlag) {
    
    m_iLastDuration += 0 ;
    m_iAecFlag = iAecFlag ;
    m_iAwakeFlag = iAwakeFlag ;
    m_iSleepFlasg = iSleepFlag ;
    m_iAsrFlag = iAsrFlag ;
    
    ZLOG_INFO("r2status Changed ---- AwakeFlag:%s SleepFlag:%s AsrFlag:%s AecFlag:%s",
              ((iAwakeFlag == 1) ? "true" : "false"),
              ((iSleepFlag == 1) ? "true" : "false"),
              ((iAsrFlag == 1) ? "true" : "false"),
              ((iAecFlag == 1) ? "true" : "false"));
  }else{
    m_iLastDuration += iLen_In ;
    if (m_iLastDuration > m_iFrmSize * 5000) {
      ZLOG_INFO("r2status Cur ---- AwakeFlag:%s SleepFlag:%s AsrFlag:%s AecFlag:%s",
                ((iAwakeFlag == 1) ? "true" : "false"),
                ((iSleepFlag == 1) ? "true" : "false"),
                ((iAsrFlag == 1) ? "true" : "false"),
                ((iAecFlag == 1) ? "true" : "false"));
      m_iLastDuration = 0 ;
    }
  }
  
}

r2_mic_info* r2mem2::geterrorinfo(float** pData_Mul, int iLen_Mul){
  
  
  std::vector<int> ErrorMic ;
  for (int i = 0 ; i < m_pSto->m_pMicInfo_In->iMicNum ; i ++) {
    int iMicId = m_pSto->m_pMicInfo_In->pMicIdLst[i] ;
    if (fabs(pData_Mul[iMicId][0]) < 0.5f) {
      ErrorMic.push_back(iMicId);
    }
  }
  
  if (ErrorMic.size() > 0) {
    r2_mic_info* pMicErr = r2_getdefaultmicinfo(ErrorMic.size());
    for (int i = 0 ; i < ErrorMic.size() ; i ++) {
      pMicErr->pMicIdLst[i] = ErrorMic[i] ;
      ZLOG_INFO("Detect Err Mic %d", ErrorMic[i]);
    }
    return  pMicErr ;
  }
  
  return NULL ;
  
}
