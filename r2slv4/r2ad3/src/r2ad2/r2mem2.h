//
//  r2mem2.h
//  r2ad
//
//  Created by hadoop on 10/22/15.
//  Copyright (c) 2015 hadoop. All rights reserved.
//

#ifndef __r2ad__r2mem2__
#define __r2ad__r2mem2__

#include <stdio.h>

#include "r2ad2.h"
#include "r2sto2.h"

#include "../r2math.h"
#include "../r2mem_i.h"
#include "../r2mem_cod.h"
#include "../r2mem_vad2.h"
#include "../r2mem_vbv3.h"
#include "../r2mem_bf.h"

#ifndef __ARM_ARCH_ARM__
#define _MEM2_RAW_FILE
#define _BF_AUDIO
#define _BF_RAW_AUDIO
#define _VAD_AUDIO
//#define _Debug_Opu_Audio
#else
//#define _MEM2_RAW_FILE
//#define _BF_AUDIO
//#define _BF_RAW_AUDIO
//#define _VAD_AUDIO
//#define _Debug_Opu_Audio
#endif

class r2mem2
{
public:
  r2mem2(r2sto2* pSto);
public:
  ~r2mem2(void);
  
  int ProcessData(char* pData_In, int iLen_In, int iAecFlag, int iAwakeFlag, int iSleepFlag, int iAsrFlag, int iHotwordFlag);
  int reset();
  
  r2_mic_info* geterrorinfo(float** pData_Mul, int iLen_Mul);
  
public:
  
  r2sto2* m_pSto ;
  int m_iMicNum ;
  
  //i
  r2mem_i* m_pMem_In ;
  
  //vad sl bf vt
  r2mem_vbv3* m_pMem_Vbv3 ;
  
  //vad2
  r2mem_vad2* m_pMem_Vad2 ;
  
  //bf
  r2mem_bf* m_pMem_Bf ;
  
  //cod
  r2mem_cod* m_pMem_Cod ;
  
  std::string m_str_AwakeInfo ;
  
#ifdef _MEM2_RAW_FILE
  FILE* m_pRawFile ;
#endif
  
  //fix mic
  bool m_bFirstFrm ;
  
  //state
  bool m_bAsr ;
  bool m_bVadStart ;
  bool m_bDataOutput ;
  bool m_bCanceled ;
  bool m_bAwake ;
  
  //debug
  std::string m_strDebugFolder ;
  
  float m_pSlInfo_BK[3] ;
  
#ifdef  _Debug_Opu_Audio
  FILE* m_pDebugOpuFile ;
#endif
  
#ifdef _BF_AUDIO
  FILE* m_pBfFile ;
#endif
  
#ifdef _BF_RAW_AUDIO
  FILE* m_pBfRawFile ;
#endif
  
  
public:
  int GetMsgLst(r2ad_msg_block** &pMsgLst, int &iMsgNum);
  
protected:
  int AddMsg(enum r2ad_msg iMsgId, int iMsgDataLen ,const char* pMsgData);
  int AddMsgWithSlInfo(enum r2ad_msg iMsgId, const char* pSlInfo);
  int AddMsgWithAudio(enum r2ad_msg iMsgId, r2mem_cod* pMem_Cod);
  int PrintMsgInfo(r2ad_msg_block* pMsg);
  int ClearMsgLst();
  int ResetAsr();
  
  //No New
  int m_iMsgNum_Cur ;
  int m_iMsgNum_Total ;
  r2ad_msg_block**    m_pMsgLst ;
  
  //CurentStatus
  int m_iFrmSize ;
  int m_iLastDuration ;
  int m_iAecFlag ;
  int m_iAwakeFlag ;
  int m_iSleepFlasg ;
  int m_iAsrFlag ;
  
  void PrintStatus(int iLen_In, int iAecFlag, int iAwakeFlag, int iSleepFlag, int iAsrFlag);
  
  int m_iAsrMsgCheckFlag ;
  
  //No New
  int m_iCol_NoNew ;
  float** m_pData_NoNew ;
  
};

#endif /* defined(__r2ad__r2mem2__) */
