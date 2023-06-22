//
//  main.cpp
//  testvt4
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include <iostream>

#include "../../r2vt4/src/vt/zvtapi.h"
#include "../../r2vt4/src/aud/zaudapi.h"
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

//r2_cf_htask cfhandel = NULL ;
int g_iCn = 0 ;
r2_vt_htask sss = NULL ;

bool z_isdir(std::string path)
{
  struct stat sb;
  if (stat(path.c_str(), &sb) == -1)
    return false;
  return S_ISDIR(sb.st_mode);
}

bool z_check_suffix(const char* str, const char* suffix){
  
  const char* pf = strrchr(str,'.');
  if(pf != NULL && strcasecmp(pf+1,suffix) == 0) {
    return true ;
  }
  return false ;
  
}

std::vector<std::string> z_getfilelist(const char* pFolder,const char* suffix) {
  
  
  DIR              *pDir ;
  struct dirent    *ent  ;
  std::vector<std::string>  FileList ;
  char FullPath[512];
  pDir = opendir(pFolder);
  if(pDir == NULL) {
    fprintf(stderr,"cannot open directory: %s\n", pFolder);
    return FileList;
  }
  
  while((ent = readdir(pDir) )!= NULL) {
    if(strcasecmp(".",ent->d_name) == 0 ||
       strcasecmp("..",ent->d_name) == 0 )
      continue;
    
    sprintf(FullPath,"%s/%s", pFolder,ent->d_name);
    if(z_isdir(FullPath)) {
      std::vector<std::string>  fffff = z_getfilelist(FullPath,suffix) ;
      for (int i = 0 ; i < fffff.size() ; i ++) {
        FileList.push_back(fffff[i]);
      }
    } else {
      char* pf = strrchr(ent->d_name,'.');
      if(pf != NULL && strcasecmp(pf+1,suffix) == 0) {
        FileList.push_back(FullPath);
      }
    }
  }
  closedir(pDir);
  return FileList ;
  
}

int new_r2_vt_sysinit(const char* pWorkdir){
  
  std::string NnetPath = pWorkdir ;
  std::string PhoneTablePath = pWorkdir ;
  std::string CfPath = pWorkdir ;
  
  NnetPath =  NnetPath + "/final.svd.mod" ;
  PhoneTablePath = PhoneTablePath + "/phonetable" ;
  CfPath = CfPath + "/final.ruoqi.mod" ;
  
  r2_vt_sysinit(NnetPath.c_str(), PhoneTablePath.c_str());
  
  r2_cf_sysinit();
  
  //cfhandel = r2_cf_create(CfPath.c_str());
  
  
  return  0 ;
  
}

r2_vt_htask new_r2_vt_create(int iCn){
  
  if (iCn != g_iCn){
    if (sss != NULL) {
      r2_vt_free(sss);
    }
    sss = r2_vt_create(iCn);
    g_iCn = iCn ;
  }

  int iWordNum = 1 ;
  WordInfo * pWordLst = new WordInfo[iWordNum] ;
  
  pWordLst[0].iWordType = WORD_AWAKE ;
  strcpy(pWordLst[0].pWordContent_UTF8, "若琪") ;
  strcpy(pWordLst[0].pWordContent_PHONE, "r|l|r_B|l_B|# w o4|o4_E|## q|q_B|# i2|i2_E|##");
  //strcpy(pWordLst[0].pWordContent_PHONE, "h|h_B|# A1|E1 Y|Y_E|## x|x_B y|y_B|# a3 W|W_E|## m|m_B|# a4 W|W_E|##");
  pWordLst[0].fBlockAvgScore = 3.7 ;
  pWordLst[0].fBlockMinScore = -10.0f ;
  
  pWordLst[0].bLeftSilDet = true ;
  pWordLst[0].bRightSilDet = true ;
  
  pWordLst[0].bRemoteAsrCheckWithAec = false ;
  pWordLst[0].bRemoteAsrCheckWithNoAec = false ;
  
  pWordLst[0].bLocalClassifyCheck = false ;
  pWordLst[0].fClassifyShield = -0.1f ;
  strcpy(pWordLst[0].pLocalClassifyNnetPath, "/Users/hadoop/Documents/XCode/test/workdir_cn/nnet_iter08_lr3.125e-07_tr97.15_cv95.37");
  
  r2_vt_setwords(sss, pWordLst, iWordNum) ;
  
  
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


int z_mkdir_with_sep(const char* path){
  
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



float ProcessVtFile(const char* pSrcPath, const char * pDesPath, int iWavOffset, bool bFloat, bool bVtDet){
  
  float res = 0.0f ;
  
  r2_aud* pAud_In = NULL ;

  //Read Data
  if (iWavOffset != 0 || z_check_suffix(pSrcPath, "wav")) {
    
    pAud_In = r2_aud_in(pSrcPath, 16000) ;
    
    //r2_aud_out("/Users/hadoop/16k.wav", pAud_In);
    
  }else{
    
    int iLen = 0 ;
    char* pData = NULL ;
    
    
    FILE* pSrcFile = fopen(pSrcPath,"rb");
    if (pSrcFile == NULL) {
      return -1000.0f ;
    }
    fseek(pSrcFile,0,SEEK_END);
    iLen = ftell(pSrcFile) ;
    
    fseek(pSrcFile,0,SEEK_SET);
    pData = new char[iLen] ;
    
    fread(pData,sizeof(char),iLen,pSrcFile);
    
    fclose(pSrcFile) ;
    
    if (bFloat) {
      iLen = iLen / sizeof(float) ;
      float* pData_Float = (float*) pData ;
      
      pAud_In = r2_aud_malloc(1, 16000, iLen) ;
      
      for (int i = 0 ; i < iLen ; i ++) {
        pAud_In->data[0][i] = pData_Float[i] ;
      }
    }else{
      iLen = iLen / sizeof(short) ;
      short* pData_Short = (short*) pData ;
      
      pAud_In = r2_aud_malloc(1, 16000, iLen) ;
      
      for (int i = 0 ; i < iLen ; i ++) {
        pAud_In->data[0][i] = pData_Short[i] ;
      }
    }
    
    delete pData ;
    
  }
  
  printf("Read From %s with %d cn %d data\n", pSrcPath, pAud_In->cn, pAud_In->len) ;
  
  if (pAud_In->cn == 6 ) {
    pAud_In->cn = 4 ;
  }
  
  if (pAud_In->cn == 8 ) {
    pAud_In->cn = 6 ;
  }
  
  if (pAud_In->cn == 10 ) {
    pAud_In->cn = 8  ;
    for (int i = 0 ; i < pAud_In->cn ; i ++) {
      memcpy(pAud_In->data[i], pAud_In->data[i + 2], sizeof(float) * pAud_In->len);
    }
  }
  
  r2_vt_htask sss = new_r2_vt_create(pAud_In->cn);
  const WordInfo* pWordInfo = NULL ;
  const WordDetInfo* pWordDetInfo = NULL ;
  

  
  int iFrmSize = 160 ;
  int nBlock = iFrmSize * 12 ;
  int iStart = 0 , iEnd = 0 ;
  
  int iLen = (pAud_In->len / nBlock + 1) * nBlock  + iFrmSize * 36 ;
  
  
  if (iLen < nBlock ) {
    r2_vt_free(sss) ;
    r2_aud_free(pAud_In) ;
    return  - 1000.0f ;
  }
  
  float ** pDataBuff = new float*[pAud_In->cn] ;
  float ** pWavbuff = new float*[pAud_In->cn] ;
  for (int i = 0 ; i < pAud_In->cn ; i ++) {
    pWavbuff[i] =  new float[iLen] ;
    memset(pWavbuff[i], 0, sizeof(float) * iLen);
    memcpy(pWavbuff[i], pAud_In->data[i], sizeof(float) * pAud_In->len);
  }
  
  if (bVtDet) {
    int rt = 0 ;
    //require VtDet
    for (int i = 0 ; i + nBlock <= iLen ; i += nBlock) {
      for (int j = 0 ; j < pAud_In->cn ; j ++) {
        pDataBuff[j] = pWavbuff[j] + i ;
      }
      if (i + nBlock * 2 > iLen) {
        rt = r2_vt_process(sss, (const float**)pDataBuff,nBlock,1);
      }else{
        rt = r2_vt_process(sss, (const float**)pDataBuff,nBlock,0);
      }
      if (rt & R2_VT_WORD_DET_BEST) {
        res = 1.0f ;
        r2_vt_getdetwordinfo(sss, &pWordInfo, &pWordDetInfo, 0) ;
      
        iStart = i + nBlock - (pWordDetInfo->iWordPos_Start + 5 * iFrmSize) ;
        iEnd = i + nBlock - (pWordDetInfo->iWordPos_End - 5 * iFrmSize ) ;
        if (iStart < 0 ) {
          iStart = 0 ;
        }
        if (iEnd > iLen) {
          iEnd = iLen ;
        }
        break ;
      }
    }
  }else{
    
    //Without VtDet
    for (int i = 0 ; i < iLen ; i += nBlock) {
      for (int j = 0 ; j < pAud_In->cn ; j ++) {
        pDataBuff[j] = pWavbuff[j] + i ;
      }
      res = r2_vt_getbestscore_eer(sss,(const float**)pDataBuff,nBlock);
      //printf("%f \n", res) ;
    }
    r2_vt_getdetwordinfo(sss, &pWordInfo, &pWordDetInfo, 0) ;
    
    
    iStart = iLen - (iLen % nBlock) - (pWordDetInfo->iWordPos_Start  + 10 * iFrmSize)  ;
    iEnd = iLen - (iLen % nBlock) - (pWordDetInfo->iWordPos_End - 10 * iFrmSize) ;
    
    if (iStart < 0 ) {
      iStart = 0 ;
    }
    if (iEnd > iLen) {
      iEnd = iLen ;
    }

    //float cfscore = r2_cf_check_buff(cfhandel, pWavbuff[0] + iStart, iEnd - iStart, 0);
    
    //printf("%f %f %d %d %s ---------\n",cfscore, res, iStart/iFrmSize, iEnd / iFrmSize, pSrcPath);
    
  }
  
  if (iEnd > iStart && pDesPath != NULL && res > 3.0f) {
    r2_aud* pAud_Out = r2_aud_malloc(pAud_In->cn, pAud_In->sr, iEnd - iStart) ;
    for (int i = 0 ; i < pAud_Out->cn ; i ++) {
      memcpy(pAud_Out->data[i], pWavbuff[i] + iStart, sizeof(float) * pAud_Out->len) ;
    }
    r2_aud_out(pDesPath, pAud_Out);
    
    r2_aud_free(pAud_Out) ;
  }

  for (int i = 0 ; i < pAud_In->cn ; i ++) {
    delete [] pWavbuff[i] ;
  }
  delete [] pDataBuff ;
  delete [] pWavbuff ;
  
  r2_aud_free(pAud_In) ;
  
  //r2_vt_free(sss);
  r2_vt_reset(sss);
  
  return res ;
  
}



int testbatchfile(const char* pWorkDir, const char* pFileLstPath,  const char* pSrcFolder, const char* pDesFolder, int iThread){
  
  printf("WorkDir:      %s\n", pWorkDir);
  printf("pFileLstPath: %s\n", pFileLstPath);
  printf("SrcFolder:    %s\n", pSrcFolder);
  printf("DesFolder:    %s\n", pDesFolder);
  printf("ThreadNum:    %d\n", iThread);
  
  new_r2_vt_sysinit(pWorkDir);
  
  char DesPath[1024] ;
  
  std::vector<std::string>  FileLst  ;
  
  if (pFileLstPath != NULL) {
    FileLst = GetFileContentLst(pFileLstPath);
  }
  
  if (FileLst.size() == 0) {
    FileLst = z_getfilelist(pSrcFolder,"wav");
  }
  
  for (int i = 0 ;  i < FileLst.size() ; i ++) {
    if (pDesFolder != NULL) {
      sprintf(DesPath,"%s/%s",pDesFolder, FileLst[i].c_str() + strlen(pSrcFolder) + 1);
      if (access(DesPath, F_OK) == 0) {
        printf("%d/%d\t%s Already Exist, Skip\n",i, FileLst.size(), DesPath);
      }else{
        z_mkdir_with_sep(DesPath);
        float res = ProcessVtFile(FileLst[i].c_str(),DesPath,44,false,false);
        printf("%d/%d/%f\t%s\n",i, FileLst.size(), res, DesPath);
      }
    }else{
      float res = ProcessVtFile(FileLst[i].c_str(),NULL,44,false,false);
    }
  }
  
  r2_vt_sysexit();
  
  return 0 ;
}

int testsinglefile(const char* pWorkDir, const char* pSrcPath, const char* pDesPath){
  
  new_r2_vt_sysinit(pWorkDir);
  
  //for (int i = 0 ; i < 100 ; i ++) {
    ProcessVtFile(pSrcPath, pDesPath, 44, false, false) ;
  //}

  
  r2_vt_sysexit();
  r2vt_mem_print();
  
  return  0  ;
  
}

//cut for CSV File
int cutcsvfile(const char* pWorkDir, const char* pCSVFilePath,  const char* pSrcFolder, const char* pDesFolder){
  
  std::vector<std::string> FileLst = GetFileContentLst(pCSVFilePath);
  
  new_r2_vt_sysinit(pWorkDir);
  
  char line[1000], SrcPath[1000], DesPath[1000];
  
  float res = 0.0f ;
  
  for (int i = 0 ; i < FileLst.size() ; i ++) {
    strcpy(line, FileLst[i].c_str());
    char* pPath = strtok(line, " \t");
    char* pTag = strtok(NULL, "\r\n");
    
    sprintf(SrcPath, "%s/%s", pSrcFolder,pPath );
    sprintf(DesPath, "%s/%s", pDesFolder,pPath );
    
    if (access(SrcPath, F_OK) == 0 && strlen(pTag) == 1) {
      
      z_mkdir_with_sep(DesPath);
      res = ProcessVtFile(SrcPath,DesPath,44,false,false);
      printf("%d/%d %s\n", i, FileLst.size(), DesPath);
    }
  }
  r2_vt_sysexit();
  
  return 0 ;
}


//cut for asr File
int cutasrfile(const char* pWorkDir, const char* pPcmFilePath,  const char* pSrcFolder, const char* pDesFolder){
  
  std::vector<std::string> FileLst = GetFileContentLst(pPcmFilePath);
  
  new_r2_vt_sysinit(pWorkDir);
  
  char line[1000], DesPath[1000];
  
  float res = 0.0f ;
  
  for (int i = 0 ; i < FileLst.size() ; i ++) {
    strcpy(line, FileLst[i].c_str());
    char* pPath = strtok(line, " \t");
    
    sprintf(DesPath, "%s%s.wav", pDesFolder,pPath + strlen(pSrcFolder) );
    if (access(pPath, F_OK) == 0) {
      
      z_mkdir_with_sep(DesPath);
      res = ProcessVtFile(pPath,DesPath,0,false,false);
      printf("%d/%d %s\n", i, FileLst.size(), DesPath);
    }
  }
  r2_vt_sysexit();
  
  return 0 ;
}

//calculate score for CSV File
int scorecsvfile(const char* pWorkDir, const char* pCSVFilePath,  const char* pSrcFolder, const char* pResFilePath){
  
  std::vector<std::string> FileLst = GetFileContentLst(pCSVFilePath);
  
  new_r2_vt_sysinit(pWorkDir);
  
  FILE* pResFile = fopen(pResFilePath, "wb");
  char line[1000], SrcPath[1000], DesPath[1000];
  
  float res = 0.0f ;
  
  for (int i = 0 ; i < FileLst.size() ; i ++) {
    strcpy(line, FileLst[i].c_str());
    char* pPath = strtok(line, " \t");
    char* pTag = strtok(NULL, "\r\n");
    
    sprintf(SrcPath, "%s/%s", pSrcFolder,pPath );
    if (access(SrcPath, F_OK) == 0 && strlen(pTag) == 1) {
      res = ProcessVtFile(SrcPath,NULL,44,false,false);
      fprintf(pResFile, "%s %f %s\n", pPath, res, pTag) ;
      //printf("%d/%d:%s %f %s\n",i, FileLst.size(), pPath, res, pTag);
    }
  }
  //r2_cf_free(cfhandel);
  r2_cf_sysexit();
  r2_vt_sysexit();
  
  fclose(pResFile);
  return 0 ;
}

//calculate score for CSV File
int scorefile(const char* pWorkDir, const char* pFilePath){
  
  std::vector<std::string> FileLst = GetFileContentLst(pFilePath);
  
  new_r2_vt_sysinit(pWorkDir);
  
  char line[1000];
  
  float res = 0.0f ;
  
  for (int i = 0 ; i < FileLst.size() ; i ++) {
    strcpy(line, FileLst[i].c_str());
    char* pPath = strtok(line, " \t");
    
    if (access(pPath, F_OK) == 0) {
      res = ProcessVtFile(pPath,NULL,0,false,false);
      //printf("%d/%d:%s %f %s\n",i, FileLst.size(), pPath, res, pTag);
    }
  }
  //r2_cf_free(cfhandel);
  r2_cf_sysexit();
  r2_vt_sysexit();
  
  return 0 ;
}


int convertfiles(const char* pFileLstPath, const char* pSrcFolder, const char* pDesFolder){
  
  std::vector<std::string> FileLst = GetFileContentLst(pFileLstPath);
  char DesPath[1000];
  
  
  for (int i = 0 ; i < FileLst.size() ; i ++) {
    sprintf(DesPath, "%s%s.wav", pDesFolder,FileLst[i].c_str() + strlen(pSrcFolder) );
    z_mkdir_with_sep(DesPath);
    
    r2_aud* pAud_In = r2_aud_in(FileLst[i].c_str(), 16000) ;
    
    if (pAud_In != NULL) {
      if (pAud_In->cn == 6 ) {
        pAud_In->cn = 4 ;
      }
      
      if (pAud_In->cn == 8 ) {
        pAud_In->cn = 6 ;
      }
      
      if (pAud_In->cn == 10 ) {
        pAud_In->cn = 8  ;
        for (int i = 0 ; i < pAud_In->cn ; i ++) {
          memcpy(pAud_In->data[i], pAud_In->data[i + 2], sizeof(float) * pAud_In->len);
        }
      }
      
      pAud_In->cn = 1 ;
      
      r2_aud_out(DesPath, pAud_In) ;
      
      r2_aud_free(pAud_In) ;
      
      
    }
    printf("%05d/%05d:%s\n",i, FileLst.size(), DesPath) ;
  }
  
  return 0 ;
}


int main(int argc, const char * argv[]) {
  
  const char * pWorkDir = "/Users/hadoop/Documents/XCode/test/workdir_cn" ;
  const char * pSrcPath = "/Users/hadoop/Documents/XCode/test/debug/2017-03-29_18-32-23_8_cf_cb_2.pcm" ;
  //const char * pSrcPath = "/Users/hadoop/3.wav" ;
  //const char * pSrcPath = "/Users/hadoop/1009_F_0115.-1.wav" ;
  //const char * pSrcPath = "/Users/hadoop/010115000010-001e3352-2cb4-4c20-bd0e-119ddf76176a.wav" ;
  const char * pDesPath = "/Users/hadoop/cut.pcm";
  
  const char * pSrcFolder = "/Users/hadoop/wavfolder/111" ;
  const char * pDesFolder = "/Users/hadoop/wavfolder/222" ;
  
  if (argc == 6) {
    if (strcasecmp(argv[1], "scorecsv") == 0) {
      scorecsvfile(argv[2],argv[3],argv[4],argv[5]);
    }else if(strcasecmp(argv[1], "cutcsv") == 0){
      cutcsvfile(argv[2],argv[3],argv[4],argv[5]);
    }else if(strcasecmp(argv[1], "cutasr") == 0){
      cutasrfile(argv[2],argv[3],argv[4],argv[5]);
    }else{
      printf("error format");
      return  0 ;
    }
    
  }else if (argc == 4){
    convertfiles(argv[1], argv[2], argv[3]);
  }else if (argc == 3){
    scorefile(argv[1], argv[2]);
  }else{
    testsinglefile(pWorkDir,pSrcPath,pDesPath);
    //scorefile(pWorkDir, "/Users/hadoop/Documents/XCode/test/1111/FileLst.txt", "/Users/hadoop/Documents/XCode/test/1111/res.txt");
  }
  
  return 0;
}
