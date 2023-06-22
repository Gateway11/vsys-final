//
//  r2sto3.cpp
//  r2ad2
//
//  Created by hadoop on 8/4/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

#include "r2sto3.h"
#include "r2ssp.h"

r2sto3::r2sto3(const char* pWorkDir)
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
  m_iNoBfMicId = m_pMicInfo_In->pMicIdLst[m_pMicInfo_In->iMicNum / 2 ];
  
  //out
  m_pMicInfo_Out = r2_getdefaultmicinfo(1);
  
  m_bBf = r2_getkey_bool(config, "r2ssp", "r2ssp.android.audio.bf", true) ;
  m_bAgc = r2_getkey_bool(config, "r2ssp", "r2ssp.android.audio.agc", true) ;
  
  //r2ssp
  r2ssp_ssp_init();
  ZLOG_INFO("---------------------------------------------r2ssp initok");
  
}

r2sto3::~r2sto3(void)
{
  r2_free_micinfo(m_pMicInfo_In);
  r2_free_micinfo(m_pMicInfo_Bf);
  r2_free_micinfo(m_pMicInfo_Out);
  
  R2_SAFE_DEL_AR1(m_pMicPos);
  R2_SAFE_DEL_AR1(m_pMicI2sDelay);
  
  r2ssp_ssp_exit();
}




