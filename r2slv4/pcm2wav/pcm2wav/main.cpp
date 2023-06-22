//
//  main.cpp
//  pcm2wav
//
//  Created by hadoop on 9/26/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include <iostream>

#include "aud/zaudapi.h"
#include <string>
#include <vector>
#include <map>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

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

int r2_mkdir(const char* path){
  
  char tmpPath[512];
  memset(tmpPath,0,sizeof(char)*512);
  for (int i = 0 ; i < strlen(path) ; i ++) {
    if (*(path+i) == '/') {
      mkdir(tmpPath,0777);
    }
    tmpPath[i] = path[i] ;
  }
  //mkdir(tmpPath,0777);
  return 0 ;
}


int SingleTrans(const char* pInFilePath, const char* pOutFilePath){
  
  r2_mkdir(pOutFilePath);
  
  //pcm2wav
  //  r2_aud* pAudIn = r2_pcm_in(pInFilePath, 0, 16000, 1, format_int16) ;
  //  if (pAudIn != NULL) {
  //    printf("Read From %s with %d cn %d data\n", pInFilePath, pAudIn->cn, pAudIn->len) ;
  //    r2_aud_out(pOutFilePath, pAudIn);
  //    r2_aud_free(pAudIn);
  //    return  0 ;
  //  }
  
  //multicn to 1
  r2_aud* pAudIn = r2_aud_in(pInFilePath, 0);
  if (pAudIn != NULL && pAudIn->cn > 1) {
    for (int i = 0 ; i < pAudIn->len ; i ++) {
      for (int j = 1 ; j < pAudIn->cn ; j ++) {
        pAudIn->data[0][i] += pAudIn->data[j][i];
      }
      pAudIn->data[0][i] = pAudIn->data[0][i] / pAudIn->cn ;
    }
    pAudIn->cn = 1 ;
    r2_aud_out(pOutFilePath, pAudIn);
    r2_aud_free(pAudIn);
    return 0 ;
  }
  
  return  1 ;
  
  
}

int BatchTrans(const char* pInFileLst, const char* pInFolder, const char* pOutFolder, const char* pOutFileLst){
  
  std::vector<std::string> FileList = GetPathLst(pInFileLst);
  
  r2_mkdir(pOutFileLst);
  FILE* pFile = fopen(pOutFileLst, "wb");
  
  for (int i = 0 ; i < FileList.size(); i ++) {
    std::vector<std::string> ll = z_str_split(FileList[i].c_str(), "\t\r\n") ;
    
    std::string name = GetName(ll[0].c_str(), pInFolder, pOutFolder) ;
    
    SingleTrans(ll[0].c_str(), name.c_str());
    
    std::string res = name ;
    for (int j = 1 ; j < ll.size() ; j ++) {
      res += "\t" ;
      res += ll[j] ;
    }
    
    fprintf(pFile, "%s\n", res.c_str());
    printf("%d/%d\n", i, FileList.size()) ;
  }
  
  fclose(pFile);
  
  return 0 ;
}

int main(int argc, const char * argv[]) {
  
  // insert code here...
  if (argc == 5) {
    BatchTrans(argv[1], argv[2], argv[3], argv[4]);
  }
  
  return 0;
}
