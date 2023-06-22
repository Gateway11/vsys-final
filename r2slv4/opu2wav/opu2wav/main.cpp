//
//  main.cpp
//  opu2wav
//
//  Created by hadoop on 8/8/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include <iostream>
#include "opus/opus.h"
#include "../../r2vt4/src/aud/zaudapi.h"

#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


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

int pcm2wav(const char* pPcmPath, const char* pWavPath){
  
  r2_aud* pAudIn = r2_pcm_in(pPcmPath, 0, 48000, 6, format_int32);
  
  r2_aud_out(pWavPath, pAudIn);
  
  r2_aud_free(pAudIn);
  
  return 0 ;
}


int opu2wav(const char* pOpuPath, const char* pWavPath){
  
  
  FILE* pOpuFile = fopen(pOpuPath, "rb");
  fseek(pOpuFile, 0, SEEK_END);
  long nSize = ftell(pOpuFile);
  char* pBytes = new char[nSize] ;
  fseek(pOpuFile, 0, SEEK_SET);
  fread(pBytes, 1, nSize, pOpuFile);
  fclose(pOpuFile);
  
  
  int duration = 20 ;
  int sample_rate = 16000;
  int channels = 1 ;
  
  int error = 0 ;
  OpusDecoder	*pOpusDec = opus_decoder_create(sample_rate, channels, &error);
  
  short* pData_Out = new short[sample_rate * 100] ;
  int iOutLen = 0 ;
  int output_samples = sample_rate / 1000 * duration ;
  
  int cur = 0 ;
  while (cur < nSize) {
    char* pData = pBytes + cur + 1 ;
    int nPacketSize = pBytes[cur] ;
    if (cur + nPacketSize + 1 <= nSize) {
      int decret = opus_decode(pOpusDec, (const unsigned char*)pData, nPacketSize, pData_Out + iOutLen, output_samples, 0);
      iOutLen += decret ;
    }
    cur += nPacketSize + 1 ;
  }
  
  opus_decoder_destroy(pOpusDec);
  
  
  r2_aud* pAud_Out = r2_aud_malloc(1, sample_rate, iOutLen);
  for (int i = 0 ; i < iOutLen ; i ++) {
    pAud_Out->data[0][i] = pData_Out[i] ;
  }
  r2_aud_out(pWavPath, pAud_Out) ;
  
  r2_aud_free(pAud_Out);
  
  delete pData_Out ;
  delete pBytes ;
  
  return 0 ;
  
}


int batchconvert(const char* pSrc, const char* pDes){
  
  char des[1024] ;
  
  std::vector<std::string> filelst =  z_getfilelist(pSrc, "pcm") ;
  
  for (int i = 0 ; i < filelst.size() ; i ++) {
    sprintf(des,"%s/%s",pDes, filelst[i].c_str() + strlen(pSrc) + 1);
    int len = strlen(des) ;
    des[len - 3] = '\0' ;
    strcat(des, "wav");
    //opu2wav(filelst[i].c_str(), des);
    pcm2wav(filelst[i].c_str(), des);
    
    printf("%d:%s\n",i,  filelst[i].c_str());
  }
  return 0 ;
}

int main(int argc, const char * argv[]) {
  // insert code here...
  //opu2wav("/Users/hadoop/opu/2017-08-08_19-30-06_230_Debug.pcm", "/Users/hadoop/wav/2017-08-08_19-30-06_230_Debug.wav");
  
  //batchconvert("/Users/hadoop/Jack_M", "/Users/hadoop/wav") ;
  pcm2wav("/Users/hadoop/Downloads/raw_data.pcm", "/Users/hadoop/Downloads/sss.wav");
  return 0;
}
