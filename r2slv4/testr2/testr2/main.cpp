//
//  main.cpp
//  testssp
//
//  Created by hadoop on 6/9/15.
//  Copyright (c) 2015 hadoop. All rights reserved.
//

#include <iostream>

#include "../../r2ad3/src/r2ad1/r2ad1.h"
#include "../../r2ad3/src/r2ad2/r2ad2.h"
#include "../../r2ad3/src/r2ad3/r2ad3.h"

#include "../../r2vt4/src/vt/zvtapi.h"

#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <math.h>

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
    strtok(line,"\r\n\t");
    if(strlen(line) > 3) {
      while (line[strlen(line) - 1] == ' ') {
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

std::string  ProcessRawFile(r2ad1_htask hTaskAd1, r2ad2_htask hTaskAd2 , const char* pRawFilePath){
  
  printf("pRawFilePath: %s\n",pRawFilePath);
  
  std::string res = pRawFilePath ;
  res += "####" ;
  
  char Info[256];
  
  int iBlockSize = 4 * 8 * 160;
  int iExtBlockNum = 100 ;
  
  int iOffset = 0 ;
  if (strstr(pRawFilePath, ".wav") != NULL) {
    iOffset = 44 ;
  }
  
  FILE * pFile = fopen(pRawFilePath,"rb");
  fseek(pFile, 0, SEEK_END);
  int iBlockNum = (ftell(pFile) - iOffset) / iBlockSize ;
  //iBlockNum = 100000 ;
  

  int iShiftBlock = 10 ;
  char* pData = new char[iBlockSize * iShiftBlock] ;
  //memset(pData, 0, iBlockSize * iShiftBlock) ;
  
  time_t t1 = time(NULL);
  
  for (int iter = 0 ; iter < 1 ; iter ++) {
    
    fseek(pFile, iOffset, SEEK_SET);
    
    for (long i = 0 ; i < iBlockNum ; i +=iShiftBlock) {
      
      fread(pData, iBlockSize, iShiftBlock, pFile);
      
      char* pData2 = NULL ;
      int iLen2 = 0 ;
      
      int iAecFlag = r2ad1_putdata2(hTaskAd1, pData ,iBlockSize * iShiftBlock , pData2, iLen2);
      if (iLen2 == 0) {
        continue ;
      }
      //printf("%d %d\n",iAecFlag,iLen2);
      //continue ;
      
      r2ad2_putaudiodata2(hTaskAd2, pData2, iLen2, iAecFlag, true, false, false);
      
      r2ad_msg_block** pMsgLst = NULL ;
      int iMsgNum = 0 ;
      
      r2ad2_getmsg2(hTaskAd2, &pMsgLst, &iMsgNum);
      
      for (int j = 0 ; j < iMsgNum ; j ++) {
        
        switch (pMsgLst[j]->iMsgId) {
          case r2ad_awake_pre:
            res = pRawFilePath ;
            res += "####" ;
            sprintf(Info, "SL:%s;", pMsgLst[j]->pMsgData);
            res += Info ;
            res += "VT_PRE;" ;
            break;
          case r2ad_awake_nocmd:
            res += "VT_NOCMD;" ;
            break;
          case r2ad_awake_cmd:
            res += "VT_CMD;" ;
            break;
          case r2ad_awake_cancel:
            res += "VT_CANCEL;" ;
            break;
          case r2ad_hotword:
            res += "NO_VAD" ;
            res += pMsgLst[j]->pMsgData ;
          default:
            break;
        }
      }
      
    }
  }
  
  fclose(pFile);
  
  printf("\nCompletely Const Time %d\n",time(NULL) - t1);
  
  delete  pData ;
  
  return res ;
}

std::string  ProcessRawFile2( const char* pRawFilePath){
  
  printf("pRawFilePath: %s\n",pRawFilePath);
  
  r2ad1_htask hTaskAd1 = r2ad1_create() ;
  r2ad2_htask hTaskAd2 = r2ad2_create() ;
  
  std::string res = "" ;
  for (int i = 0 ; i < 10000 ; i ++) {
     res = ProcessRawFile(hTaskAd1,hTaskAd2,pRawFilePath) ;
  }
  
  r2ad1_free(hTaskAd1);
  r2ad2_free(hTaskAd2);
  
  return res ;
}



int testbatchfile(const char* pWorkDir, const char* pFileLstPath, const char* pResultLst, int iThreadNum){
  
  printf("WorkDir:      %s\n", pWorkDir);
  printf("pFileLstPath: %s\n", pFileLstPath);
  printf("ResultLst:    %s\n", pResultLst);
  
  std::vector<std::string> FileLst = GetFileContentLst(pFileLstPath);
  
  r2ad1_sysinit(pWorkDir);
  r2ad2_sysinit(pWorkDir);
  
  FILE* pFile_Res = fopen(pResultLst, "wb");
  
  for (int i = 0 ; i < FileLst.size() ; i ++) {
    
    r2ad1_htask hTaskAd1 = r2ad1_create() ;
    r2ad2_htask hTaskAd2 = r2ad2_create() ;
    
    std::string res = ProcessRawFile(hTaskAd1,hTaskAd2,FileLst[i].c_str()) ;
    
    fprintf(pFile_Res, "%s\n", res.c_str()) ;
    printf("%d/%d %s\n",i, FileLst.size(), FileLst[i].c_str());
    
    r2ad1_free(hTaskAd1);
    r2ad2_free(hTaskAd2);
    
  }
  
  fclose(pFile_Res);
  
  
  
  r2ad1_sysexit();
  r2ad2_sysexit();
  
  return  0  ;
  
}


int testdebug(const char* pWorkDir, const char* pFileLstPath){
  
  r2ad1_sysinit(pWorkDir);
  r2ad2_sysinit(pWorkDir);
  
  std::vector<std::string> TaskLst = GetFileContentLst(pFileLstPath);
  
  r2ad1_htask hTaskAd1 = r2ad1_create() ;
  r2ad2_htask hTaskAd2 = r2ad2_create() ;
  
  for (int i = 0 ; i < TaskLst.size() ; i ++) {
    ProcessRawFile(hTaskAd1,hTaskAd2,TaskLst[i].c_str()) ;
  }
  
  r2ad1_free(hTaskAd1);
  r2ad2_free(hTaskAd2);
  
  r2ad1_sysexit();
  r2ad2_sysexit();
  
  return  0 ;
}


int testsinglefile(const char* pWorkDir, const char* pDataPath){
  
  if (strstr(pDataPath, ".lst") != NULL) {
    return testdebug(pWorkDir,pDataPath) ;
  }
  
  //const char* pDataPath = "/Users/hadoop/xiongdawei_M_0032.10.pcm" ;
  //const char* pWorkDir = "/Users/hadoop/Documents/XCode/test/workdir_cn" ;
  
  r2ad1_sysinit(pWorkDir);
  r2ad2_sysinit(pWorkDir);
  r2ad3_sysinit(pWorkDir);
  
  time_t t1 = time(NULL);
  
  std::string res = ProcessRawFile2(pDataPath);
  
  printf("\nCompletely Const Time %d\n",time(NULL) - t1);
  
  
  r2ad3_sysexit();
  r2ad2_sysexit();
  r2ad1_sysexit();
  
  r2vt_mem_print();
  r2ad_mem_print();
  
  return  0  ;
  
}

int testagcfile(const char* pWorkDir, const char* pRawFilePath, const char* pOutPath){
  
  r2ad1_sysinit(pWorkDir);
  r2ad3_sysinit(pWorkDir);
  
  r2ad1_htask hTaskAd1 = r2ad1_create() ;
  r2ad3_htask hTaskAd3 = r2ad3_create() ;
  
  
  int iBlockSize = 4 * 8 ;
  int iExtBlockNum = 100 ;
  
  FILE * pFile = fopen(pRawFilePath,"rb");
  fseek(pFile, 0, SEEK_END);
  int iBlockNum = (ftell(pFile) - 44) / iBlockSize ;
  char* pData = new char[iBlockSize * (iBlockNum + iExtBlockNum)] ;
  memset(pData, 0, iBlockSize * (iBlockNum + iExtBlockNum));
  fseek(pFile, 44, SEEK_SET);
  fread(pData, iBlockSize, iBlockNum, pFile);
  fclose(pFile);
  
  pFile = fopen(pOutPath, "wb");
  int iShiftBlock = 10 ;
  
  for (int i = 0 ; i < iBlockNum + iExtBlockNum - iShiftBlock; i +=iShiftBlock) {
    char* pData2 = NULL ;
    int iLen2 = 0 ;
    
    r2ad1_putdata2(hTaskAd1, pData + i * iBlockSize, iBlockSize * iShiftBlock , pData2, iLen2);
    
    char* pData3 = NULL ;
    int iLen3 = 0 ;
    r2ad3_putdata2(hTaskAd3, pData2, iLen2, pData3, iLen3);
    
    fwrite(pData3, sizeof(char), iLen3, pFile);
  }
  
  fclose(pFile) ;
  
  delete  pData ;
  
  r2ad1_free(hTaskAd1);
  r2ad3_free(hTaskAd3);
  
  r2ad1_sysexit();
  r2ad3_sysexit();
  
  return 0 ;
  
}

int GetMicPos(float dim, int iMicNum){
  
  for (int i = 0 ; i < iMicNum ; i ++) {
    float alpha = 2 * 3.1415926 / iMicNum * i ;
    printf("r2ssp.mic.pos.%d=%0.8f,%0.8f,%0.8f\n",i,cos(alpha) * dim / 2 , sin(alpha) * dim / 2, 0.0f );
  }
  printf("15度－－－－－－－－－－－－－－\n");
  
  float beta = 3.1415926 * 15.0f / 180 ;
  for (int i = 0 ; i < iMicNum ; i ++) {
    float alpha = 2 * 3.1415926 / iMicNum * i ;
    float x = cos(alpha) * dim / 2 ;
    float y = sin(alpha) * dim / 2 ;
    float z = 0.0f ;
    float yy = y * cos(beta) + z * sin(beta);
    float zz = y * cos(beta) - z * sin(beta);
    printf("r2ssp.mic.pos.%d=%0.8f,%0.8f,%0.8f\n",i,x, yy, zz );
  }
  return  0 ;
  
}


int main(int argc, const char * argv[]) {
  
  
  if (argc == 5) {
    testbatchfile(argv[1],argv[2],argv[3],atoi(argv[4]));
  }else if(argc == 3){
    testsinglefile(argv[1],argv[2]);
  }else{
    //testsinglefile("/Users/hadoop/Documents/XCode/test/workdir_cn","/Users/hadoop/girl_voice_rokid.pcm");
    
    //8mic pebble
    testsinglefile("/Users/hadoop/Documents/XCode/test/workdir_cn","/Users/hadoop/raw_ok_mic.pcm");
    //testsinglefile("/Users/hadoop/Documents/XCode/test/workdir_cn","/Users/hadoop/raw-bug.pcm");
    
    //近距离7mic
    //testsinglefile("/Users/hadoop/Documents/XCode/test/workdir_cn","/Users/hadoop/debug0.pcm");
    //testsinglefile("/Users/hadoop/Documents/XCode/test/workdir_cn","/Users/hadoop/aec_cap1.wav");
    
    //testsinglefile("/data/debug/workdir_cn","/data/debug/jiaoshou.wav");
    //testsinglefile("/Users/hadoop/Documents/XCode/test/workdir_cn","/Users/hadoop/jiaoshou.wav");
    //testsinglefile("/Users/hadoop/Documents/XCode/test/workdir_cn","/Users/hadoop/Downloads/AEC_test_65dB_1m_side.wav");
    
    //testagcfile("/Users/hadoop/Documents/XCode/test/workdir_cn","/Users/hadoop/Downloads/debug0.pcm","/Users/hadoop/Downloads/agc.pcm");
    //GetMicPos(0.075f, 6);
  }
  
  
  return 0;
}
