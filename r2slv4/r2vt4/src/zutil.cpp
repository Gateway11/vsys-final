//
//  zutil.cpp
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zutil.h"

namespace __r2vt4__ {

  std::string z_getkey(const char *path,const char *title,const char *key){
    
    int ll = 10240 ;
    char* szLine = Z_SAFE_NEW_AR1(szLine, char, ll);
    std::string strtitle = "[" ;
    strtitle += title ;
    strtitle += "]" ;
    
    FILE *fp = fopen(path, "r");
    if(fp == NULL) {
      Z_SAFE_DEL_AR1(szLine);
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
            Z_SAFE_DEL_AR1(szLine);
            return strtitle ;
          }
        }
      }
    }
    fclose(fp);
    Z_SAFE_DEL_AR1(szLine);
    
    ZLOG_INFO("Failed to get cfg key %s %s",title,key);
    return "" ;
  }
  
  std::string z_getkey_path(const char* workdir, const char *path,const char *title,const char *key){
    
    std::string value = z_getkey(path,title,key) ;
    if (value.size() > 0) {
      char pp[1024];
      sprintf(pp, "%s/%s",workdir,value.c_str());
      value = pp ;
      return  value ;
    }else{
      return "" ;
    }
  }
  
  int z_getkey_int(const char *path,const char *title,const char *key, int df){
    
    std::string vv = z_getkey(path,title,key);
    if (vv.size() == 0) {
      return  df;
    }
    return atoi(vv.c_str());
  }
  
  float z_getkey_float(const char* path, const char* title, const char *key, float df){
    
    std::string vv = z_getkey(path, title, key);
    if (vv.size() == 0) {
      return  df;
    }
    return atof(vv.c_str());
  }
  
  bool z_getkey_bool(const char *path,const char *title,const char *key, bool df){
    
    std::string vv = z_getkey(path,title,key);
    if (vv.size() == 0) {
      return  df;
    }
    if ((strcasecmp(vv.c_str(),"true") == 0) || (atoi(vv.c_str()) > 0)) {
      return true ;
    }else{
      return false ;
    }
  }
  
  std::vector<std::string> z_str_split(const char* str, const char* split){
    
    std::vector<std::string> rt ;
    char *pTmp = Z_SAFE_NEW_AR1(pTmp, char, strlen(str) + 5) ;
    char *pTok = NULL;
    strcpy(pTmp,str);
    char* pV = strtok_r(pTmp,split,&pTok);
    while(pV != NULL){
      rt.push_back(pV);
      pV = strtok_r(NULL,split,&pTok);
    }
    Z_SAFE_DEL_AR1(pTmp) ;
    
    return rt ;
  }
  
  bool CheckID(char *idPar, char A, char B, char C, char D) {
    
    return(	(idPar[0] == A) &&	(idPar[1] == B) &&
           (idPar[2] == C) &&	(idPar[3] == D)	);
  }
  
  std::vector<std::string> GetFileContentLst(const char* pFilePath){
    
    std::vector<std::string> FileContentList ;
    
    FILE * pFile = fopen(pFilePath,"rb");
    if (pFile == NULL) {
      ZLOG_ERROR("Failed to load file %s",pFilePath);
      return FileContentList ;
    }
    
    char *line = Z_SAFE_NEW_AR1(line, char, 5120);
    while(!feof(pFile)) {
      line[0] = '\0';
      fgets(line,5120,pFile);
      strtok(line,"\r\n");
      if(strlen(line) > 0) {
        while (line[strlen(line) - 1] == ' ') {
          line[strlen(line) - 1] = '\0' ;
        }
      }
      if(strlen(line) > 0) {
        FileContentList.push_back(line);
      }
    }
    fclose(pFile);
    
    Z_SAFE_DEL_AR1(line);
    
    return FileContentList ;
    
  }
  
  const char* GetFileName(const char* path) {
    
    const char* pTemp = strrchr(path,'\\');
    if (pTemp == NULL) {
      pTemp = strrchr(path,'/');
    }
    if (pTemp == NULL) {
      pTemp = path ;
    } else {
      pTemp ++ ;
    }
    return pTemp ;
  }
  
  std::vector<std::string> z_getfilelist(const char* pFolder,const char* suffix) {
    
#ifdef _WIN32
    char pPattern[512];
    sprintf(pPattern,"%s%c*.%s",pFolder,m_cSep,suffix);
    std::vector<std::string>  FileList ;
    _finddata_t c_file;
    long hFile;
    if((hFile = _findfirst(pPattern, &c_file )) != -1L) {
      if (c_file.attrib != _A_SUBDIR) {
        FileList.push_back(c_file.name);
      }
      while(_findnext(hFile, &c_file ) == 0 )	{
        if (c_file.attrib != _A_SUBDIR) {
          FileList.push_back(c_file.name);
        }
      }
      _findclose(hFile);
    }
    return FileList ;
#else
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
      if(ent->d_type & DT_DIR) {
        //dir ignore
      } else {
        char* pf = strrchr(ent->d_name,'.');
        if(pf != NULL && strcasecmp(pf+1,suffix) == 0) {
          sprintf(FullPath,"%s/%s", pFolder,ent->d_name);
          FileList.push_back(FullPath);
        }
      }
    }
    closedir(pDir);
    return FileList ;
#endif
    
  }
  
  int z_storedata_mat(const char* pFilePath, float** pData, int row, int col){
    
    FILE* pFile = fopen(pFilePath, "wb");
    for (int i = 0 ; i < row ; i ++) {
      fwrite(pData[i], sizeof(float), col, pFile);
    }
    fclose(pFile);
    return  0 ;
  }
  
  int z_storedata_vec(const char* pFilePath, float* pData, int size){
    
    //    FILE* pFile = fopen(pFilePath, "wb");
    //    for (int i = 0 ; i < row ; i ++) {
    //        fwrite(pData[i], sizeof(float), col, pFile);
    //    }
    //    fclose(pFile);
    
    FILE* pFile = fopen(pFilePath, "wb");
    fwrite(pData, sizeof(float), size, pFile);
    fclose(pFile);
    return  0 ;
  }
  
  int z_storedata_vec2(const char* pFileFolder, int iCount, float* pData, int size){
    
    char FilePath[256];
    sprintf(FilePath, "%s/%d.dat",pFileFolder,iCount);
    
    FILE* pFile = fopen(FilePath, "wb");
    for (int i = 0 ; i < size ; i ++) {
      //fwrite(pData[i], sizeof(float), col, pFile);
      fprintf(pFile, " %0.6f\n",pData[i]);
    }
    fclose(pFile);
    
    return  0 ;
  }
  
  static int g_count = 0 ;
  
  std::string z_getdatatime(){
    
    time_t now;
    struct tm *tm_now;
    char    datetime[200];
    
    time(&now);
    tm_now = localtime(&now);
    strftime(datetime, 200, "%Y-%m-%d_%H-%M-%S_", tm_now);
    std::string dt = datetime ;
    
    sprintf(datetime, "%d", g_count ++ );
    dt += datetime ;
    
    return  dt ;
  }
  
  int z_mkdir(const char* path){
    
    char tmpPath[512];
    memset(tmpPath,0,sizeof(char)*512);
    for (int i = 0 ; i < strlen(path) ; i ++) {
      if (*(path+i) == '/') {
        mkdir(tmpPath,0777);
      }
      tmpPath[i] = path[i] ;
    }
    mkdir(tmpPath,0777);
    return 0 ;
  }
  
  char* z_new_ar1(size_t size,size_t dim1){
    
    if (dim1 == 0) {
      return NULL ;
    }
    char * pData = new char[dim1*size];
    if (pData == NULL) {
      ZLOG_ERROR("Error in z_new_ar1");
      return NULL ;
    }
    
    memset(pData,0,size*dim1);
    
    return pData ;
  }
  
  char** z_new_ar2(size_t size,size_t dim1,size_t dim2){
    
    if (dim1 == 0 || dim2 == 0) {
      return NULL ;
    }
    
    char** pData = (char**)z_new_ar1(sizeof(char*), dim1);
    if( NULL == pData ) {
      ZLOG_ERROR("Error in z_new_ar2_1");
      return NULL ;
    }
    
    pData[0] = z_new_ar1(sizeof(char), size * dim1 * dim2);
    if( NULL == pData[0] ) {
      ZLOG_ERROR("Error in z_new_ar2_2");
      Z_SAFE_DEL_AR1(pData);
      return NULL ;
    }
    
    memset(pData[0],0,size * dim1 * dim2);
    for(int k=1; k<dim1; k++) {
      pData[k] = pData[k-1] + dim2 * size;
    }
    
    return pData;
  }
  
  
  char*   z_new_sse_ar1(size_t size, size_t dim1, int align){
    
    //assert(size % 4 == 0);
    assert(dim1 % 4 == 0);
    assert((size * dim1) % align == 0);
    
    if (dim1 == 0) {
      return NULL ;
    }
    
#ifdef __ARM_NEON
    void* pData = NULL ;
    posix_memalign(&pData, align, size * dim1);
#else
    void * pData = (char*)_mm_malloc(size * dim1,align);
#endif
    
    if (pData == NULL) {
      ZLOG_ERROR("Error in z_new_ar1");
      return NULL ;
    }
    
    memset(pData,0,size*dim1);
    
    return (char*)pData ;
  }
  
  
  char**	z_new_sse_ar2(size_t size,size_t dim1,size_t dim2,int align){
    
    //assert(size % 4 == 0);
    assert((size * dim2) % align == 0);
    
    if (dim1 == 0 || dim2 == 0) {
      return NULL ;
    }
    
    char** pData = (char**)z_new_ar1(sizeof(char*), dim1);
    if( NULL == pData ) {
      ZLOG_ERROR("Error in z_new_ar2_1");
      return NULL ;
    }
    
    pData[0] = (char*)z_new_sse_ar1(size, dim1 * dim2, align);
    if( NULL == pData[0] ) {
      ZLOG_ERROR("Error in z_new_ar2_2");
      Z_SAFE_DEL_AR1(pData);
      return NULL ;
    }
    
    memset(pData[0],0,size * dim1 * dim2);
    for(int k=1; k<dim1; k++) {
      pData[k] = pData[k-1] + dim2 * size;
    }
    
    return pData;
    
  }
  
#ifdef  Z_MEM_DEBUG
  
  static std::map<std::string, std::string> z_mem_lst ;
  static std::map<std::string, int> z_mem_size_lst ;
  int iTotalSize = 0 ;
  
  void z_mem_insert(void* pData, int iSize, const char* pSource, int iLine){
    
    char kk[1024];
    char vv[1024];
    
    if(pData != NULL){
      sprintf(kk, "%X",pData);
      sprintf(vv, "%s:%d:%dBytes",pSource, iLine, iSize);
      
      z_mem_lst[kk] = vv ;
      z_mem_size_lst[kk] = iSize ;
      iTotalSize += iSize ;
      if(iSize == 1){
        ZLOG_INFO("r2vt4 new  ------ %s",vv) ;
        ZLOG_INFO("CurSize: %d", iTotalSize);
      }
      
    }
    
  }
  
  void z_mem_erase(void* pData){
    
    char kk[1024];
    if (pData != NULL) {
      sprintf(kk, "%X",pData);
      assert(z_mem_lst.find(kk) != z_mem_lst.end());
      ZLOG_INFO("r2signal free ------ %s",z_mem_lst[kk].c_str()) ;
      z_mem_lst.erase(kk);
      iTotalSize -= z_mem_size_lst[kk] ;
      z_mem_size_lst.erase(kk) ;
      ZLOG_INFO("CurSize: %d", iTotalSize);
    }
  }
  
  void z_mem_print(){
    
    std::map<std::string, std::string>::iterator it;
    for(it=z_mem_lst.begin();it!=z_mem_lst.end();++it){
      ZLOG_ERROR("MemLeak: %s",it->second.c_str());
    }
    ZLOG_ERROR("MemLeak Check Completely vt4");
  }
  
#endif
  
  int z_gcd(int m, int n){
    
    while (1) {
      m %= n;
      if (m == 0) return (n > 0 ? n : -n);
      n %= m;
      if (n == 0) return (m > 0 ? m : -m);
    }
  }
  
  int z_lcm(int m, int n){
    
    int gcd = z_gcd(m, n);
    return gcd * (m/gcd) * (n/gcd);
  }
  
  float z_randuniform() {  // random between 0 and 1.
    return (float)((rand() + 1.0) / (RAND_MAX+2.0));
  }
  
  float z_randgauss() {
    return (float)(sqrt (-2 * log(z_randuniform()))
                   * cos(Z_2PI*z_randuniform()));
  }
  
  float z_log_add(float x, float y) { //	z = log( exp(x) + exp(y) )
    float z;
    if( x < y ) {
      z = x;
      x = y;
      y = z;
    }
    if( (z = y-x) < LEXPZ )	{
      return (x < LSMALL ? LZERO : x);
    }
    z = x + (float)log(1.0 + exp(z));
    return (z < LSMALL ? LZERO : z);
  }
  
  
  z_mic_info* z_getmicinfo(const char* path, const char* title, const char* key){
    
    std::string vv = z_getkey(path,title,key);
    std::vector<std::string> vs = z_str_split(vv.c_str(),",");
    if (vs.size() > 0) {
      z_mic_info* pMicInfo = Z_SAFE_NEW(pMicInfo, z_mic_info);
      pMicInfo->iMicNum = vs.size() ;
      pMicInfo->pMicIdLst = Z_SAFE_NEW_AR1(pMicInfo->pMicIdLst, int, pMicInfo->iMicNum);
      for (int i = 0 ; i < pMicInfo->iMicNum; i ++) {
        pMicInfo->pMicIdLst[i] = atof(vs[i].c_str());
      }
      return pMicInfo ;
    }else {
      return 0 ;
    }
  }
  
  
  z_mic_info* z_getdefaultmicinfo(int iMicNum){
    
    z_mic_info* pMicInfo = Z_SAFE_NEW(pMicInfo, z_mic_info);
    pMicInfo->iMicNum = iMicNum ;
    pMicInfo->pMicIdLst = Z_SAFE_NEW_AR1(pMicInfo->pMicIdLst, int, pMicInfo->iMicNum);
    for (int i = 0 ; i < pMicInfo->iMicNum; i ++) {
      pMicInfo->pMicIdLst[i] = i ;
    }
    return pMicInfo ;
  }
  
  
  int z_free_micinfo(z_mic_info* &pMicInfo){
    
    if (pMicInfo != NULL) {
      Z_SAFE_DEL_AR1(pMicInfo->pMicIdLst);
      Z_SAFE_DEL(pMicInfo);
    }
    return  0 ;
  }
  
  z_mic_info* z_copymicinfo(z_mic_info* pMicInfo_Old){
    
    z_mic_info* pMicInfo_New = Z_SAFE_NEW(pMicInfo_New, z_mic_info);
    pMicInfo_New->iMicNum = pMicInfo_Old->iMicNum ;
    pMicInfo_New->pMicIdLst = Z_SAFE_NEW_AR1(pMicInfo_New->pMicIdLst, int, pMicInfo_New->iMicNum);
    for (int i = 0 ; i < pMicInfo_New->iMicNum; i ++) {
      pMicInfo_New->pMicIdLst[i] = pMicInfo_Old->pMicIdLst[i] ;
    }
    return pMicInfo_New ;
  }
  
  std::string  z_getslinfo(float* pSlInfo){
    
    char info[256];
    int iAzimuth = (pSlInfo[0] - 3.1415936f) * 180 / 3.1415936f + 0.1f ;
    while (iAzimuth < 0) {
      iAzimuth += 360 ;
    }
    while (iAzimuth >= 360) {
      iAzimuth -= 360 ;
    }
    
    sprintf(info,"%5f %5f",(float)iAzimuth , pSlInfo[1] * 180 / 3.1415936f);
    std::string strSlInfo = info ;
    return  strSlInfo ;
  }

};




