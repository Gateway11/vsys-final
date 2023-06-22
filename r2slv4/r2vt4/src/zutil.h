//
//  zutil.h
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zutil__
#define __r2vt4__zutil__

#include "zmath.h"

namespace __r2vt4__ {

  std::string     z_getkey(const char *path,const char *title,const char *key);
  std::string     z_getkey_path(const char* workdir, const char *path,const char *title,const char *key);
  int             z_getkey_int(const char *path,const char *title,const char *key, int df);
  float           z_getkey_float(const char* path, const char* title, const char *key, float df);
  bool            z_getkey_bool(const char *path,const char *title,const char *key, bool df);
  
  std::vector<std::string> z_str_split(const char* str, const char* split);
  bool CheckID(char *idPar, char A, char B, char C, char D);
  
  std::vector<std::string> z_str_split(const char* str, const char* split);
  std::vector<std::string> GetFileContentLst(const char* pFilePath);
  
  const char* GetFileName(const char* path);
  std::vector<std::string> z_getfilelist(const char* pFolder,const char* suffix);
  
  int z_storedata_mat(const char* pFilePath, float** pData, int row, int col);
  int z_storedata_vec(const char* pFilePath, float* pData, int size);
  int z_storedata_vec2(const char* pFileFolder, int iCount, float* pData, int size);
  
  std::string z_getdatatime();
  int z_mkdir(const char* path);
  
  
#ifdef  Z_MEM_DEBUG
  void z_mem_insert(void* pData, int iSize, const char* pSource, int iLine);
  void z_mem_erase(void* pData);
  void z_mem_print();
#endif
  
  char*   z_new_ar1(size_t size,size_t dim1);
  char**	z_new_ar2(size_t size,size_t dim1,size_t dim2);
  char*   z_new_sse_ar1(size_t size, size_t dim1, int align);
  char**	z_new_sse_ar2(size_t size,size_t dim1,size_t dim2, int align);
  
  int z_gcd(int m, int n);
  int z_lcm(int m, int n);
  float z_randuniform() ;
  float z_randgauss() ;
  float z_log_add(float x, float y);
  
  struct z_mic_info{
    int iMicNum ;
    int* pMicIdLst ;
  };
  z_mic_info* z_getmicinfo(const char* path, const char* title, const char* key);
  z_mic_info* z_getdefaultmicinfo(int iMicNum);
  int z_free_micinfo(z_mic_info* &pMicInfo);
  z_mic_info* z_copymicinfo(z_mic_info* pMicInfo_Old);
  
  std::string  z_getslinfo(float* pSlInfo);
  
};


#endif /* __r2vt4__zutil__ */
