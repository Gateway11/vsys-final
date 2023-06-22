//
//  main.cpp
//  r2ad_offline
//
//  Created by hadoop on 7/28/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

#include <iostream>

#include "r2mem_audio.h"
#include "r2mem_rs.h"
#include "r2mem_vad3.h"
#include "r2mem_aec.h"
#include "r2mem_ns.h"

//#define USE_AEC

using namespace __r2ad_offline__ ;


bool bAec = true ;
bool bRs = true ;
bool bNs = true ;

int g_iMicNum = 0 ;
float * g_pMicPos = NULL;

r2_mic_info* g_pMicInfo_Rs = NULL ;

r2_mic_info* g_pMicInfo_Aec = NULL ;
r2_mic_info* g_pMicInfo_AecRef = NULL ;

r2mem_audio* g_pAudio_Raw = NULL ;
r2mem_audio* g_pAudio_Rs = NULL ;
r2mem_audio* g_pAudio_Aec = NULL ;
r2mem_audio* g_pAudio_Ns = NULL ;
r2mem_audio* g_pAudio_Out = NULL ;

r2mem_rs* g_pRs = NULL ;
r2mem_aec* g_pAec = NULL ;
r2mem_ns* g_pNs = NULL ;


int Process(const char* pFileInPath, const char* pFileOutPath){
  
  //Reset
  g_pAudio_Raw->Reset() ;
  g_pAudio_Rs->Reset() ;
  g_pAudio_Aec->Reset() ;
  g_pAudio_Ns->Reset() ;
  g_pAudio_Out->Reset() ;
  
  if (bRs) {
    g_pRs->reset() ;
  }
  if (bAec) {
    g_pAec->reset() ;
  }
  if (bNs) {
    g_pNs->reset();
  }
  
  r2_mkdir(pFileOutPath);

  int rt = -1 ;
  if (g_iMicNum  == 6) {
    rt = g_pAudio_Raw->ReadFormSingleFile(pFileInPath, r2_in_int_32, 0);
  }if (g_iMicNum  == 8) {
    rt = g_pAudio_Raw->ReadFormSingleFile(pFileInPath, r2_in_int_32, 44);
  }else if(g_iMicNum == 10){
    rt = g_pAudio_Raw->ReadFormSingleFile(pFileInPath, r2_in_int_32_10, 0);
  }
  if (rt != 0) {
    printf("xxxxx Failed ReadFormSingleFile %s\n ", pFileInPath) ;
    return -1 ;
  }else{
    printf("Read %d %d From %s\n", g_pAudio_Raw->m_iCn, g_pAudio_Raw->m_iDataLen, pFileInPath) ;
  }
  
  //g_pAudio_Raw->WriteToWavFile(pFileOutPath);
  
  //remove dc
  for (int i = 0 ; i < g_pMicInfo_Aec->iMicNum ; i ++) {
    int iMicId = g_pMicInfo_Aec->pMicIdLst[i] ;
    double mean = 0.0 ;
    for (int j = 0 ; j < g_pAudio_Raw->m_iDataLen; j ++) {
      mean += g_pAudio_Raw->m_pData[iMicId][j] ;
    }
    mean = mean / g_pAudio_Raw->m_iDataLen ;
    
    for (int j = 0 ; j < g_pAudio_Raw->m_iDataLen; j ++) {
      g_pAudio_Raw->m_pData[iMicId][j] -= mean ;
    }
  }
  
  //g_pAudio_Raw->WriteToWavFile(pFileOutPath);
  
  float** pData_Out_Mul = NULL , *pData_Out_Sig = NULL  ;
  int iLen_Out = 0 ;
  
  //Rs
  if (bRs) {
    g_pRs->process(g_pAudio_Raw->m_pData, g_pAudio_Raw->m_iDataLen, pData_Out_Mul, iLen_Out);
    g_pAudio_Rs->AddData(pData_Out_Mul, iLen_Out);
  }else{
    g_pAudio_Rs->AddData(g_pAudio_Raw) ;
  }
  
  //printf("Rs Ok\n") ;
  
  //g_pAudio_Rs->WriteToWavFile(pFileOutPath);

  //Aec
  if (bAec) {
    rt = g_pAec->process(g_pAudio_Rs->m_pData, g_pAudio_Rs->m_iDataLen, pData_Out_Mul, iLen_Out);
    g_pAudio_Aec->AddData(pData_Out_Mul, iLen_Out);
  }else{
    g_pAudio_Aec->AddData(g_pAudio_Rs);
  }
  
  //g_pAudio_Aec->WriteToWavFile(pFileOutPath);
  
  //printf("Aec Ok\n") ;
  
  //Ns
  if (bNs) {
    g_pNs->process(g_pAudio_Aec->m_pData, g_pAudio_Aec->m_iDataLen, pData_Out_Mul, iLen_Out);
    g_pAudio_Ns->AddData(pData_Out_Mul, iLen_Out);
  }else{
    g_pAudio_Ns->AddData(g_pAudio_Aec);
  }
  
  //g_pAudio_Ns->WriteToWavFile(pFileOutPath);
  //printf("Ns Ok\n") ;
  
  //Out
  g_pAudio_Out->AddData(g_pAudio_Ns);
  
  //Sl
  //g_pSl->putdata(g_pAudio_Ns->m_pData, g_pAudio_Ns->m_iDataLen) ;
  //printf("SL Ok\n") ;
  
  //Bf
  //g_pBf->steer(3.1415965 * (270 - 180) / 180 , 0.0f);
  //g_pBf->steer(fAzimuth, fElevation);
  //g_pBf->process(g_pAudio_Ns->m_pData, g_pAudio_Ns->m_iDataLen, pData_Out_Sig, iLen_Out);
  //memcpy(g_pAudio_Out->m_pData[g_pMicInfo_AecRef->pMicIdLst[0]], pData_Out_Sig, sizeof(float)*iLen_Out) ;
  //printf("BF Ok\n") ;
  
  //g_pSlBf->process(g_pAudio_Ns->m_pData, g_pAudio_Ns->m_iDataLen, true, pData_Out_Sig, iLen_Out);
  //memcpy(g_pAudio_Out->m_pData[g_pMicInfo_AecRef->pMicIdLst[1]], pData_Out_Sig, sizeof(float)*iLen_Out) ;
  //printf("SLBF Ok\n") ;
  
  for (int i = 0;  i < g_pMicInfo_Aec->iMicNum ; i ++) {
    if (g_pMicInfo_Aec->pMicIdLst[i] != i) {
      memcpy(g_pAudio_Out->m_pData[i], g_pAudio_Out->m_pData[g_pMicInfo_Aec->pMicIdLst[i]], sizeof(float) * g_pAudio_Out->m_iDataLen) ;
    }
  }
  g_pAudio_Out->m_iCn = g_pMicInfo_Aec->iMicNum ;
  
  g_pAudio_Out->WriteToWavFile(pFileOutPath);
  
  g_pAudio_Out->m_iCn = g_iMicNum ;
  
  return rt ;
}

int InitWithConfig(const char* pConfig){
  
  char line[256] ;
  std::string vv ;
  std::vector<std::string> vs ;
  
  g_iMicNum = r2_getkey_int(pConfig,"r2ssp","r2ssp.mic.num", 8);
  
  g_pMicPos = R2_SAFE_NEW_AR1(g_pMicPos, float, g_iMicNum * 3);
  for (int i = 0 ; i < g_iMicNum ; i ++){
    sprintf(line,"r2ssp.mic.pos.%d",i);
    vv = r2_getkey(pConfig,"r2ssp",line);
    vs = r2_strsplit(vv.c_str(),",");
    assert(vs.size() == 3);
    for (int j = 0 ; j < 3 ; j ++){
      g_pMicPos[i*3+j] = atof(vs[j].c_str());
    }
  }
  
  g_pMicInfo_Rs = r2_getdefaultmicinfo(g_iMicNum);
  
  g_pAudio_Raw = R2_SAFE_NEW(g_pAudio_Raw, r2mem_audio, g_iMicNum, 48000);
  g_pAudio_Rs = R2_SAFE_NEW(g_pAudio_Rs, r2mem_audio, g_iMicNum, 16000);
  g_pAudio_Aec = R2_SAFE_NEW(g_pAudio_Aec, r2mem_audio, g_iMicNum, 16000);
  g_pAudio_Ns = R2_SAFE_NEW(g_pAudio_Ns, r2mem_audio, g_iMicNum, 16000);
  g_pAudio_Out = R2_SAFE_NEW(g_pAudio_Out, r2mem_audio, g_iMicNum, 16000);
  
  if (bRs) {
    g_pRs = R2_SAFE_NEW(g_pRs, r2mem_rs, g_iMicNum,48000,g_pMicInfo_Rs);
  }
  
  if (bAec) {
    g_pMicInfo_Aec = r2_getmicinfo(pConfig,"r2ssp","r2ssp.aec.mics");
    g_pMicInfo_AecRef = r2_getmicinfo(pConfig,"r2ssp","r2ssp.aec.ref.mics");
    g_pAec = R2_SAFE_NEW(g_pAec, r2mem_aec, g_iMicNum,g_pMicInfo_Aec, g_pMicInfo_AecRef);
  }
  
  if (bNs) {
    g_pNs = R2_SAFE_NEW(g_pNs, r2mem_ns, g_iMicNum, g_pMicInfo_Aec, 2);
  }
  
  
  
  
  return 0 ;
  
}

int ExitWithConfig(){
  
  
  R2_SAFE_DEL_AR1(g_pMicPos);
  
  R2_SAFE_DEL(g_pAudio_Raw);
  R2_SAFE_DEL(g_pAudio_Rs);
  R2_SAFE_DEL(g_pAudio_Aec);
  R2_SAFE_DEL(g_pAudio_Ns);
  R2_SAFE_DEL(g_pAudio_Out);
  
  R2_SAFE_DEL(g_pRs);

  R2_SAFE_DEL(g_pAec);
  R2_SAFE_DEL(g_pNs);
  
  r2_free_micinfo(g_pMicInfo_Rs);
  r2_free_micinfo(g_pMicInfo_Aec);
  r2_free_micinfo(g_pMicInfo_AecRef);

  R2_PRINT_MEM_INFO() ;
  
  return  0 ;
}

//chenfangjie2_M_0069.ch1.pcm
std::string GetName(const char* pRawPath, const char* pLastFolder, const char* pNewFolder){
  
  char Path[512] ;
  sprintf(Path, "%s%s",pNewFolder,pRawPath + strlen(pLastFolder));
  *strrchr(Path, '.') = '\0' ;
  strcat(Path, ".wav");
  
  return  Path ;
}

std::vector<std::string> GetPathLst(const char* pFilePath){
  
  char line[256] ;
  std::vector<std::string> FileLst ;
  FILE* pFile = fopen(pFilePath, "rb");
  while (!feof(pFile)) {
    line[0] = '\0' ;
    fgets(line, 256, pFile);
    strtok(line, "\r\n");
    if (strlen(line) > 10) {
      FileLst.push_back(line) ;
    }
  }
  fclose(pFile) ;
  
  
  return  FileLst ;
}

std::vector<std::string> z_str_split(const char* str, const char* split){
  
  std::vector<std::string> rt ;
  char *pTmp = new char[strlen(str) + 5] ;
  char *pTok = NULL;
  strcpy(pTmp,str);
  char* pV = strtok_r(pTmp,split,&pTok);
  while(pV != NULL){
    rt.push_back(pV);
    pV = strtok_r(NULL,split,&pTok);
  }
  delete pTmp ;
  
  return rt ;
}

int TestBatchFile(const char* pCfgPath, const char* pFileLstPath, const char* pSrcFolder, const char* pDesFolder, const char* pResLstPath){
  
  //Init
  InitWithConfig(pCfgPath);
  
  r2_mkdir(pResLstPath);
  
  FILE* pRes = fopen(pResLstPath, "wb");
  
  //GetFileLst
  std::vector<std::string> FileLst = GetPathLst(pFileLstPath) ;
  for (int i = 0 ; i < FileLst.size() ; i ++) {
    std::vector<std::string> ll = z_str_split(FileLst[i].c_str(), "\t\r\n") ;
    
    std::string name = GetName(ll[0].c_str(), pSrcFolder, pDesFolder);
    
    int rt = Process(ll[0].c_str(), name.c_str());
    if (rt == -1) {
      continue ;
    }
    
    std::string res = name ;
    if (rt == 1) {
      res += "\t" ;
      res += "aec" ;
    }else{
      res += "\t" ;
      res += "noaec" ;
    }
    for (int j = 1 ; j < ll.size() ; j ++) {
      res += "\t" ;
      res += ll[j] ;
    }
    ZLOG_INFO("%d/%d %s", i, FileLst.size(), res.c_str()) ;
    fprintf(pRes, "%s\n", res.c_str()) ;
  }
  
  fclose(pRes) ;
  
  ExitWithConfig();
  
  return 0 ;
}

int TestSingleFile(const char* pCfgPath, const char* pSrcFilePath, const char* pDesFilePath ){
  
  InitWithConfig(pCfgPath);
  
  Process(pSrcFilePath, pDesFilePath);
  
  ExitWithConfig();

  return  0 ;
}


int main(int argc, const char * argv[]) {
  
  const char* pCfgPath = "/Users/hadoop/Documents/XCode/test/workdir_cn/r2ssp-8.cfg" ;
  const char* pFileLstPath = "/Users/hadoop/wav.scp" ;
  const char* pSrcFolder = "/Users/hadoop" ;
  const char* pDesFolder = "/Users/hadoop" ;
  const char* pResLstPath = "/Users/hadoop/FileLst-8.txt" ;
  
  const char* pSrcFilePath = "/Users/hadoop/Documents/XCode/test/sl/sl2.wav" ;
  const char* pDesFilePath = "/Users/hadoop/Documents/XCode/test/sl/sl2-ns-16k.wav" ;
  
  
  
  if (argc == 6) {
    return TestBatchFile(argv[1],argv[2],argv[3],argv[4], argv[5]);
  }else{
    //return TestBatchFile(pCfgPath,pFileLstPath,pSrcFolder,pDesFolder, pResLstPath);
    return TestSingleFile(pCfgPath, pSrcFilePath, pDesFilePath);
  }
  
 
}
