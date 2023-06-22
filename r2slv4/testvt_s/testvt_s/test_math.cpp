//
//  test_math.cpp
//  testvt_s
//
//  Created by hadoop on 9/27/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "test_math.h"

namespace __testvt_s__ {
  
  std::string t_getkey(const char *path,const char *title,const char *key){
    
    int ll = 10240 ;
    char* szLine = new char[ll];
    std::string strtitle = "[" ;
    strtitle += title ;
    strtitle += "]" ;
    
    FILE *fp = fopen(path, "r");
    if(fp == NULL) {
      return "" ;
    }
    bool flag = false ;
    while(!feof(fp)) {
      fgets(szLine,ll,fp);
      strtok(szLine,"\r\n");
      if (strlen(szLine) > 0 && szLine[0] == '[') {
        if (strcasecmp(szLine,strtitle.c_str()) == 0) {
          flag = true ;
        } else {
          flag = false ;
        }
        continue;
      }
      if(flag) {
        char * pTemp = strchr(szLine,'=') ;
        if (pTemp != NULL) {
          *pTemp = '\0';
          pTemp ++ ;
          char* kk = strtok(szLine,"\r\n\t");
          char* vv = strtok(pTemp,"\r\n\t");
          while(*kk == ' ') {
            kk ++ ;
          }
          while(strlen(kk) > 0 && *(kk+strlen(kk) - 1) == ' ') {
            *(kk+strlen(kk) - 1) = '\0';
          }
          while(*vv == ' ') {
            vv ++ ;
          }
          while(strlen(vv) > 0 && *(vv+strlen(vv) - 1) == ' ') {
            *(vv+strlen(vv) - 1) = '\0';
          }
          if(strcasecmp(kk,key) ==0) {
            fclose(fp);
            strtitle = vv ;
            delete szLine ;
            return strtitle ;
          }
        }
      }
    }
    fclose(fp);
    delete szLine ;
    
    return "" ;
  }
  
  int t_getkey_int(const char *path,const char *title,const char *key, int di){
    
    std::string vv = t_getkey(path,title,key);
    if (vv.size() == 0) {
      return di ;
    }
    return atoi(vv.c_str());
  }
  
  float t_getkey_float(const char* path, const char* title, const char *key, float df){
    
    std::string vv = t_getkey(path, title, key);
    if (vv.size() == 0) {
      return  df;
    }
    return atof(vv.c_str());
  }
  
  bool t_getkey_bool(const char *path,const char *title,const char *key){
    
    std::string vv = t_getkey(path,title,key);
    return atoi(vv.c_str()) > 0;
  }
  
  std::vector<std::string> t_getkeylst(const char *path,const char *title,const char *key){
    
    std::vector<std::string> res ;
    
    int ll = 10240 ;
    char* szLine = new char[ll];
    std::string strtitle = "[" ;
    strtitle += title ;
    strtitle += "]" ;
    
    FILE *fp = fopen(path, "r");
    if(fp == NULL) {
      return res ;
    }
    bool flag = false ;
    while(!feof(fp)) {
      fgets(szLine,ll,fp);
      strtok(szLine,"\r\n");
      if (strlen(szLine) > 0 && szLine[0] == '[') {
        if (strcasecmp(szLine,strtitle.c_str()) == 0) {
          flag = true ;
        } else {
          flag = false ;
        }
        continue;
      }
      if(flag) {
        char * pTemp = strchr(szLine,'=') ;
        if (pTemp != NULL) {
          *pTemp = '\0';
          pTemp ++ ;
          char* kk = strtok(szLine,"\r\n\t");
          char* vv = strtok(pTemp,"\r\n\t");
          while(*kk == ' ') {
            kk ++ ;
          }
          while(strlen(kk) > 0 && *(kk+strlen(kk) - 1) == ' ') {
            *(kk+strlen(kk) - 1) = '\0';
          }
          while(*vv == ' ') {
            vv ++ ;
          }
          while(strlen(vv) > 0 && *(vv+strlen(vv) - 1) == ' ') {
            *(vv+strlen(vv) - 1) = '\0';
          }
          if(strcasecmp(kk,key) ==0) {
            res.push_back(vv);
          }
        }
      }
    }
    fclose(fp);
    delete szLine ;
    return res ;
  }
  
  std::vector<std::string> t_strsplit(const char* str, const char* split){
    
    std::vector<std::string> rt ;
    char * pTmp = new char[strlen(str) + 5] , *pTok = NULL;
    strcpy(pTmp,str);
    char* pV = strtok_r(pTmp,split,&pTok);
    while(pV != NULL){
      rt.push_back(pV);
      pV = strtok_r(NULL,split,&pTok);
    }
    delete pTmp ;
    return rt ;
  }
  
  int t_mkdir(const char* path){
    
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

  t_mic_info* t_getmicinfo(const char* path, const char* title, const char* key){
    
    std::string vv = t_getkey(path,title,key);
    std::vector<std::string> vs = t_strsplit(vv.c_str(),",");
    if (vs.size() > 0) {
      t_mic_info* pMicInfo = new t_mic_info();
      pMicInfo->iMicNum = vs.size() ;
      pMicInfo->pMicIdLst = new int[pMicInfo->iMicNum];
      for (int i = 0 ; i < pMicInfo->iMicNum; i ++) {
        pMicInfo->pMicIdLst[i] = atof(vs[i].c_str());
      }
      return pMicInfo ;
    }else {
      return 0 ;
    }
  }
  
  t_mic_info* t_getdefaultmicinfo(int iMicNum){
    
    t_mic_info* pMicInfo = new t_mic_info();
    pMicInfo->iMicNum = iMicNum ;
    pMicInfo->pMicIdLst = new int[pMicInfo->iMicNum];
    for (int i = 0 ; i < pMicInfo->iMicNum; i ++) {
      pMicInfo->pMicIdLst[i] = i ;
    }
    return pMicInfo ;
  }
  
  t_mic_info* t_copymicinfo(t_mic_info* pMicInfo_Old){
    
    t_mic_info* pMicInfo_New = new t_mic_info();
    pMicInfo_New->iMicNum = pMicInfo_Old->iMicNum ;
    pMicInfo_New->pMicIdLst = new int[pMicInfo_New->iMicNum];
    for (int i = 0 ; i < pMicInfo_New->iMicNum; i ++) {
      pMicInfo_New->pMicIdLst[i] = pMicInfo_Old->pMicIdLst[i] ;
    }
    return pMicInfo_New ;
  }
  
  int t_free_micinfo(t_mic_info* &pMicInfo){
    
    if (pMicInfo != NULL) {
      delete [] pMicInfo->pMicIdLst ;
      delete pMicInfo ;
      pMicInfo = NULL ;
    }
    return  0 ;
  }
  
  
};




