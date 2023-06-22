//
//  r2sto1.cpp
//  r2ad
//
//  Created by hadoop on 10/22/15.
//  Copyright (c) 2015 hadoop. All rights reserved.
//

#include "r2sto1.h"
#include "r2ssp.h"

r2sto1::r2sto1(const char* pWorkDir)
{
  
  char config[256];
  sprintf(config,"%s/r2ssp.cfg",pWorkDir);
  
  //
  m_iMicNum = r2_getkey_int(config,"r2ssp","r2ssp.mic.num",16);
  m_iSampleRate = r2_getkey_int(config,"r2ssp","r2ssp.audio.rate",48000);
  m_iSampleBytes = r2_getkey_int(config,"r2ssp","r2ssp.audio.bytes",4);
  
  //In
  m_pMicInfo_In = r2_getdefaultmicinfo(m_iMicNum);
  
  //rs
  m_pMicInfo_Rs = r2_getmicinfo(config,"r2ssp","r2ssp.rs.mics");
  if (m_pMicInfo_Rs == NULL) {
    m_pMicInfo_Rs = r2_getdefaultmicinfo(m_iMicNum);
  }
  
  //aec
  m_bAec = r2_getkey_bool(config, "r2ssp", "r2ssp.aec", true);
  m_fAecShield = r2_getkey_float(config, "r2ssp", "r2ssp.aec.shield", 200.0f) ;
  m_pMicInfo_Aec = r2_getmicinfo(config,"r2ssp","r2ssp.aec.mics");
  m_pMicInfo_AecRef = r2_getmicinfo(config,"r2ssp","r2ssp.aec.ref.mics");
  
  //cpus
  m_pCpuInfo_Aec = r2_getmicinfo(config,"r2ssp","r2ssp.cpus.aec");
  if (m_pCpuInfo_Aec == NULL) {
    m_pCpuInfo_Aec = r2_getdefaultmicinfo(1);
    m_pCpuInfo_Aec->pMicIdLst[0] = 3 ;
  }
  
  m_pCpuInfo_Mat = r2_getmicinfo(config,"r2ssp","r2ssp.cpus.mat");
  if (m_pCpuInfo_Mat == NULL) {
    m_pCpuInfo_Mat = r2_getdefaultmicinfo(2);
    m_pCpuInfo_Mat->pMicIdLst[0] = 2 ;
    m_pCpuInfo_Mat->pMicIdLst[1] = 3 ;
  }
  
  r2ssp_ssp_init();
  
}

r2sto1::~r2sto1(void)
{
  
  r2_free_micinfo(m_pMicInfo_In);
  r2_free_micinfo(m_pMicInfo_Rs);
  r2_free_micinfo(m_pMicInfo_Aec);
  r2_free_micinfo(m_pMicInfo_AecRef);
  
  r2_free_micinfo(m_pCpuInfo_Aec);
  r2_free_micinfo(m_pCpuInfo_Mat);
  
  r2ssp_ssp_exit();
}