//
//  main.cpp
//  testvt_s
//
//  Created by hadoop on 9/26/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include <iostream>

#include "../../r2vt4/src/vt/zvtapi.h"
#include "../../r2vt4/src/vbv/zvbvapi.h"
#include "../../r2vt4/src/aud/zaudapi.h"
#include "../../r2vt4/src/cf/zcfapi.h"

#include "test_math.h"

using namespace __testvt_s__ ;

bool g_bGvt = false ;
bool g_bCf = true ;

int g_iCn = 0 ;
r2_vbv_htask sss = NULL ;

int g_iMicNum = 0 ;
float* g_pMicPos = NULL ;
float* g_pMicDelay = NULL ;
std::string g_NnetPath = "" ;
std::string g_VtPhoneTablePath = "" ;
std::string g_CfNetPath = "" ;


int t_sys_init(const char* pWorkdir, const char* pMicConfig){
  
  //Mic Init
  
  int iTotalMicNum = t_getkey_int(pMicConfig, "r2ssp","r2ssp.mic.num", 8);
  float* pTotalMicPos = new float[iTotalMicNum * 3] ;
  float* pTotalMicDelay = new float[iTotalMicNum] ;
  
  char line[256] ;
  std::string vv ;
  std::vector<std::string> vs ;
  
  for (int i = 0 ; i < iTotalMicNum ; i ++){
    sprintf(line,"r2ssp.mic.pos.%d",i);
    vv = t_getkey(pMicConfig,"r2ssp",line);
    vs = t_strsplit(vv.c_str(),",");
    assert(vs.size() == 3);
    for (int j = 0 ; j < 3 ; j ++){
      pTotalMicPos[i*3+j] = atof(vs[j].c_str());
    }
  }
  
  for (int i = 0 ; i < iTotalMicNum ; i ++) {
    if (iTotalMicNum == 6 && i % 2 == 0) {
      pTotalMicDelay[i] = 1.0f / 96000.0f ;
    }else if (iTotalMicNum == 4 && i % 2 == 0) {
      pTotalMicDelay[i] = 1.0f / 96000.0f ;
    }else{
      pTotalMicDelay[i] = 0.0f ;
    }
  }
  
  t_mic_info* pMicInfo_Aec = t_getmicinfo(pMicConfig,"r2ssp","r2ssp.aec.mics");
  g_iMicNum = pMicInfo_Aec->iMicNum ;
  g_pMicPos = new float[g_iMicNum * 3]  ;
  g_pMicDelay = new float[g_iMicNum] ;
  for (int i = 0 ; i < g_iMicNum ; i ++) {
    int iMicId = pMicInfo_Aec->pMicIdLst[i] ;
    memcpy(g_pMicPos + i * 3, pTotalMicPos + iMicId * 3, sizeof(float) * 3) ;
    g_pMicDelay[i] = pTotalMicDelay[iMicId] ;
  }
  
  delete [] pTotalMicDelay ;
  delete [] pTotalMicPos ;
  
  
  //Engine Init
  g_NnetPath = pWorkdir ;
  g_VtPhoneTablePath = pWorkdir ;
  g_CfNetPath = pWorkdir ;
  
  
  if (!g_bGvt) {
    g_NnetPath =  g_NnetPath + "/final.svd.mod" ;
    g_VtPhoneTablePath = g_VtPhoneTablePath + "/phonetable" ;
  }else{
    g_NnetPath =  g_NnetPath + "/rasr.emb.ini" ;
  }
  
  
  if (g_bCf) {
    g_CfNetPath = g_CfNetPath += "/final.ruoqi.mod" ;
  }
  
  return  0 ;
  
}

int t_sys_exit(){
  
  return  0 ;
}



r2_vt_htask t_engine_create(int iCn){
  
  if (sss != NULL && iCn != g_iMicNum) {
    assert(0);
    return sss ;
  }
  
  if (iCn == 1) {
    g_iMicNum = 1;
  }
  
  if(sss != NULL){
    return sss ;
  }
  
  sss = r2_vbv_create(g_iMicNum, g_pMicPos, g_pMicDelay, g_NnetPath.c_str(), g_VtPhoneTablePath.c_str()) ;
  
  int iWordNum = 1 ;
  WordInfo * pWordLst = new WordInfo[iWordNum] ;
  
  pWordLst[0].iWordType = WORD_AWAKE ;
  strcpy(pWordLst[0].pWordContent_UTF8, "若琪") ;
  if (!g_bGvt) {
    strcpy(pWordLst[0].pWordContent_PHONE, "r|l|r_B|l_B|# w o4|o4_E|## q|q_B|# i2|i2_E|##");
    //strcpy(pWordLst[0].pWordContent_PHONE, "er2_S|## y_B|y|# i2_E|i2|## t_B|t|# A4 Y_E|Y|##");
  }else{
    strcpy(pWordLst[0].pWordContent_PHONE, "ruo qi");
  }
  pWordLst[0].fBlockAvgScore = 4.2 ;
  pWordLst[0].fBlockMinScore = 2.7 ;
  
  pWordLst[0].bLeftSilDet = false ;
  pWordLst[0].bRightSilDet = false ;
  
  pWordLst[0].bRemoteAsrCheckWithAec = false ;
  pWordLst[0].bRemoteAsrCheckWithNoAec = false ;
  
  pWordLst[0].bLocalClassifyCheck = g_bCf ;
  pWordLst[0].fClassifyShield = -0.3f ;
  strcpy(pWordLst[0].pLocalClassifyNnetPath, g_CfNetPath.c_str());
  
  r2_vbv_setwords(sss, pWordLst, iWordNum) ;
  
  return  sss ;
  
}


std::vector<std::string> GetFileContentLst(const char* pFilePath){
  
  std::vector<std::string> FileContentList ;
  
  FILE * pFile = fopen(pFilePath,"rb");
  if (pFile == NULL) {
    printf("Failed to load file %s",pFilePath);
    return FileContentList ;
  }
  
  char *line = new char[5120];
  while(!feof(pFile)) {
    line[0] = '\0';
    fgets(line,5120,pFile);
    strtok(line,"\r\n");
    if(strlen(line) > 3) {
      while (line[strlen(line) - 1] == ' ' || line[strlen(line) - 1] == '\t') {
        line[strlen(line) - 1] = '\0' ;
      }
    }
    if(strlen(line) > 3) {
      FileContentList.push_back(line);
    }
  }
  fclose(pFile);
  
  delete line ;
  
  return FileContentList ;
  
}


int  ProcessVtFile(const char* pSrcPath){
  
  r2_aud* pAud_In = r2_aud_in(pSrcPath, 16000) ;
  if (pAud_In == NULL) {
    return  -1 ;
  }
  
  t_engine_create(pAud_In->cn);
  
  int iFrmSize = 160 ;
  int nBlock = iFrmSize * 12 ;
  
  int iLen_Ext = iFrmSize * 288 ;
  int iLen = pAud_In->len + iLen_Ext * 2 ;
  
  
  if (pAud_In->len < nBlock ) {
    r2_aud_free(pAud_In) ;
    return  - 1000.0f ;
  }
  
  float ** pDataBuff = new float*[pAud_In->cn] ;
  float ** pWavbuff = new float*[pAud_In->cn] ;
  for (int i = 0 ; i < pAud_In->cn ; i ++) {
    pWavbuff[i] =  new float[iLen] ;
    memset(pWavbuff[i], 0, sizeof(float) * iLen);
    memcpy(pWavbuff[i] + iLen_Ext, pAud_In->data[i], sizeof(float) * pAud_In->len);
    for (int j = 0 ; j < iLen_Ext ; j ++) {
      pWavbuff[i][j] = rand() * 1.0f / RAND_MAX ;
    }
    for (int j = pAud_In->len + iLen_Ext  ; j < iLen ; j ++) {
      pWavbuff[i][j] = rand() * 1.0f / RAND_MAX ;
    }
  }
  
  int rt = 0 ;

  for (int i = 0 ; i + nBlock <= iLen ; i += nBlock) {
    for (int j = 0 ; j < pAud_In->cn ; j ++) {
      pDataBuff[j] = pWavbuff[j] + i ;
    }
    int tmp = 0 ;
    if (i + nBlock * 2 > iLen) {
      tmp = r2_vbv_process(sss, (const float**)pDataBuff,nBlock,1,true);
    }else{
      tmp = r2_vbv_process(sss, (const float**)pDataBuff,nBlock,0,true);
    }
    if (tmp & R2_VT_WORD_DET) {
      rt = t_max(rt, 1) ;
    }
  }
  
  for (int i = 0 ; i < pAud_In->cn ; i ++) {
    delete [] pWavbuff[i] ;
  }
  delete [] pDataBuff ;
  delete [] pWavbuff ;
  
  r2_aud_free(pAud_In) ;
  
  r2_vbv_reset(sss);
  
  //r2_vt_free(sss);
  //r2_vt_reset(sss);
  
  return rt ;
  
}

std::vector<std::string> zz_str_split(const char* str, const char* split){
  
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

int testbatchfile(const char* pWorkDir, const char *pMicConfig, const char* pFileLstPath,  const char* pResLstPath){
  
  printf("WorkDir:        %s\n", pWorkDir);
  printf("MicConfig:      %s\n", pMicConfig);
  printf("FileLstPath:    %s\n", pFileLstPath);
  printf("ResLstPath:     %s\n", pResLstPath);
  
  t_sys_init(pWorkDir, pMicConfig);
  
  
  std::vector<std::string>  FileLst = GetFileContentLst(pFileLstPath);
  FILE* pFile = fopen(pResLstPath, "wb");
  
  for (int i = 0 ;  i < FileLst.size() ; i ++) {
    
    std::vector<std::string> ll = zz_str_split(FileLst[i].c_str(), "\t\r\n") ;
    
    int rt = ProcessVtFile(ll[0].c_str());
    
    if (rt == 3) {
      fprintf(pFile, "cf-ok\t%s\n", FileLst[i].c_str()) ;
      printf("Det Keyword in %s with cf ok\n", ll[0].c_str());
    }else if(rt == 2){
      fprintf(pFile, "cf-fail\t%s\n", FileLst[i].c_str()) ;
      printf("Det Keyword in %s with cf fail\n", ll[0].c_str());
    }else if (rt == 1) {
      fprintf(pFile, "%s\n", FileLst[i].c_str()) ;
      printf("Det Keyword in %s\n", ll[0].c_str());
    }else{
      printf("Failed to Det Keyword in %s\n", ll[0].c_str());
    }
    
  }
  fclose(pFile);
  r2_vt_sysexit();
  
  return 0 ;
}

int main(int argc, const char * argv[]) {
  
  const char* pWorkDir  = "/Users/hadoop/Documents/XCode/test/workdir_cn" ;
  const char *pMicConfig = "/Users/hadoop/Documents/XCode/test/workdir_cn/r2ssp.cfg" ;
  const char* pFileLstPath = "/Users/hadoop/Documents/XCode/test/xxxx/1.lst" ;
  const char* pResLstPath = "/Users/hadoop/Documents/XCode/test/xxxx/2.lst" ;
  
  
  if (argc == 5) {
    testbatchfile(argv[1], argv[2], argv[3], argv[4]);
  }else{
    testbatchfile(pWorkDir,pMicConfig,pFileLstPath,pResLstPath);
  }
  return 0;
}
