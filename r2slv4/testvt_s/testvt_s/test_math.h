//
//  test_math.h
//  testvt_s
//
//  Created by hadoop on 9/27/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __testvt_s__test_math__
#define __testvt_s__test_math__


#include <string>
#include <vector>
#include <map>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>





namespace __testvt_s__ {

#ifndef t_max
#define t_max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
  
#ifndef t_min
#define t_min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
  
  std::string t_getkey(const char *path,const char *title,const char *key);
  
  int t_getkey_int(const char *path,const char *title,const char *key, int di);
  float t_getkey_float(const char* path, const char* title, const char *key, float df);
  bool t_getkey_bool(const char *path,const char *title,const char *key);
  
  std::vector<std::string> t_getkeylst(const char *path,const char *title,const char *key);
  
  std::vector<std::string> t_strsplit(const char* str, const char* split);
  int t_mkdir(const char* path);
  
  struct t_mic_info{
    int iMicNum ;
    int* pMicIdLst ;
  };
  t_mic_info* t_getmicinfo(const char* path, const char* title, const char* key);
  t_mic_info* t_getdefaultmicinfo(int iMicNum);
  int t_free_micinfo(t_mic_info* &pMicInfo);
  t_mic_info* t_copymicinfo(t_mic_info* pMicInfo_Old);
  
  
};


#endif /* __testvt_s__test_math__ */
