//
//  main.cpp
//  testcf
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "../../r2vt4/src/cf/zcfapi.h"
#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int GetFileLst2(const char* pSrcPath, std::vector<std::string> &TagLst ,std::vector<std::string> &PathLst){
  
  
  TagLst.clear() ;
  PathLst.clear() ;
  
  FILE * pFile = fopen(pSrcPath,"rb");
  if (pFile == NULL) {
    printf("Failed to load file %s",pSrcPath);
    return 1 ;
  }
  
  char *line = new char[5120];
  while(!feof(pFile)) {
    line[0] = '\0';
    fgets(line,5120,pFile);
    if (strlen(line) < 10) {
      continue ;
    }
    char* pPath = strtok(line," \t");
    char* pTag = strtok(NULL, "\r\n");
    
    if(pPath != NULL &&  strlen(pPath) > 3) {
      while (pPath[strlen(pPath) - 1] == ' ') {
        pPath[strlen(pPath) - 1] = '\0' ;
      }
    }
    if(strlen(pPath) > 3) {
      TagLst.push_back(pTag) ;
      PathLst.push_back(pPath);
    }
  }
  fclose(pFile);
  
  delete line ;
  
  return 0 ;
  
}





int testsinglefile(const char* pNnetPath, const char* pSrcPath){
  
  r2_cf_sysinit();
  r2_cf_htask sss = r2_cf_create(pNnetPath, 6);
  
  
  float score = r2_cf_check_file(sss, pSrcPath, 0);
  
  r2_cf_free(sss);
  
  r2_cf_sysexit();
  
  return  0  ;
  
}


int testbatchfile(const char* pNnetPath, const char* pWavScpPath, const char* pWavResPath){
  
  r2_cf_sysinit();
  r2_cf_htask sss = r2_cf_create(pNnetPath, 1);
  
  std::vector<std::string> TagLst, PathLst ;
  GetFileLst2(pWavScpPath, TagLst, PathLst);
  
  int iTotalNum = TagLst.size() ;
  
  FILE* pFile = fopen(pWavResPath, "wb");
  for (int i = 0 ; i < iTotalNum ; i ++) {
    float score = r2_cf_check_file(sss, PathLst[i].c_str(), 0);
    fprintf(pFile, "%s %0.5f %s\n", PathLst[i].c_str(), score, TagLst[i].c_str());
    printf("%05d/%05d %0.5f %s %s\n",i,iTotalNum, score, TagLst[i].c_str(), PathLst[i].c_str());
  }
  
  fclose(pFile) ;
  return 0 ;
  
}

int main(int argc, const char * argv[]) {
  // insert code here...
  
  const char * pNnetPath = "/Users/hadoop/Documents/XCode/test/workdir_cn/final.ruoqi.mod";
  //const char * pNnetPath = "/Users/hadoop/Documents/XCode/test/workdir_cn/iter011_lr0.003906_trn0.9519_0.1387_cv0.9508_0.1406.mod";
  const char * pSrcPath = "/Users/hadoop/cb/2017-08-26_01-07-12_0_cf_cb.wav";
  
  const char * pWavScpPath = "/Users/hadoop/Documents/XCode/test/nnet-forward/wav_train.scp" ;
  const char * pWavResPath = "/Users/hadoop/Documents/XCode/test/nnet-forward/wav_res.txt";
  
  
  if (argc == 4) {
    testbatchfile(argv[1],argv[2],argv[3]);
  }else if(argc == 2){
    testsinglefile(argv[1],argv[2]);
  }else{
    testsinglefile(pNnetPath, pSrcPath);
    //testbatchfile(pNnetPath,pWavScpPath,pWavResPath);
  }
  
  return 0;
}

