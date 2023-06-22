//
//  zaudapi.cpp
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zaudapi.h"
#include "../zmath.h"
#include "../rs/zrsapi.h"

using namespace __r2vt4__ ;

namespace __r2vt4__ {
  
  struct WAVEFORM {
    unsigned short wFormatTag;
    unsigned short nChannels;
    unsigned int nSamplesPerSec;
    unsigned int nAvgBytesPerSec;
    unsigned short nBlockAlign;
    unsigned short wBitsPerSample;
  };
  
  struct RIFF {
    char riffID[4];
    unsigned int riffSIZE;
    char riffFORMAT[4];
  };
  
  struct FMT {
    char fmtID[4];
    unsigned int fmtSIZE;
    WAVEFORM fmtFORMAT;
  };
  
  struct DATA {
    char dataID[4];
    unsigned int dataSIZE;
  };
  
  
  const short ulaw2Linear_SG[256] = {
    -32124,-31100,-30076,-29052,-28028,-27004,-25980,-24956,
    -23932,-22908,-21884,-20860,-19836,-18812,-17788,-16764,
    -15996,-15484,-14972,-14460,-13948,-13436,-12924,-12412,
    -11900,-11388,-10876,-10364, -9852, -9340, -8828, -8316,
    -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
    -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
    -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
    -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
    -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
    -1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924,
    -876,  -844,  -812,  -780,  -748,  -716,  -684,  -652,
    -620,  -588,  -556,  -524,  -492,  -460,  -428,  -396,
    -372,  -356,  -340,  -324,  -308,  -292,  -276,  -260,
    -244,  -228,  -212,  -196,  -180,  -164,  -148,  -132,
    -120,  -112,  -104,   -96,   -88,   -80,   -72,   -64,
    -56,   -48,   -40,   -32,   -24,   -16,    -8,     0,
    32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
    23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
    15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
    11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316,
    7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140,
    5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092,
    3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004,
    2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980,
    1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436,
    1372,  1308,  1244,  1180,  1116,  1052,   988,   924,
    876,   844,   812,   780,   748,   716,   684,   652,
    620,   588,   556,   524,   492,   460,   428,   396,
    372,   356,   340,   324,   308,   292,   276,   260,
    244,   228,   212,   196,   180,   164,   148,   132,
    120,   112,   104,    96,    88,    80,    72,    64,
    56,    48,    40,    32,    24,    16,     8,     0
  };
  
  const short Alaw2Linear_SG[256] = {
    -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736,
    -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784,
    -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368,
    -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
    -22016,-20992,-24064,-23040,-17920,-16896,-19968,-18944,
    -30208,-29184,-32256,-31232,-26112,-25088,-28160,-27136,
    -11008,-10496,-12032,-11520, -8960, -8448, -9984, -9472,
    -15104,-14592,-16128,-15616,-13056,-12544,-14080,-13568,
    -344,  -328,  -376,  -360,  -280,  -264,  -312,  -296,
    -472,  -456,  -504,  -488,  -408,  -392,  -440,  -424,
    -88,   -72,  -120,  -104,   -24,    -8,   -56,   -40,
    -216,  -200,  -248,  -232,  -152,  -136,  -184,  -168,
    -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184,
    -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
    -688,  -656,  -752,  -720,  -560,  -528,  -624,  -592,
    -944,  -912, -1008,  -976,  -816,  -784,  -880,  -848,
    5504,  5248,  6016,  5760,  4480,  4224,  4992,  4736,
    7552,  7296,  8064,  7808,  6528,  6272,  7040,  6784,
    2752,  2624,  3008,  2880,  2240,  2112,  2496,  2368,
    3776,  3648,  4032,  3904,  3264,  3136,  3520,  3392,
    22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944,
    30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136,
    11008, 10496, 12032, 11520,  8960,  8448,  9984,  9472,
    15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568,
    344,   328,   376,   360,   280,   264,   312,   296,
    472,   456,   504,   488,   408,   392,   440,   424,
    88,    72,   120,   104,    24,     8,    56,    40,
    216,   200,   248,   232,   152,   136,   184,   168,
    1376,  1312,  1504,  1440,  1120,  1056,  1248,  1184,
    1888,  1824,  2016,  1952,  1632,  1568,  1760,  1696,
    688,   656,   752,   720,   560,   528,   624,   592,
    944,   912,  1008,   976,   816,   784,   880,   848
  };
};

struct r2_int24 {
  unsigned char m_Internal[3] ;
  
  int toint(){
    if ((m_Internal[2] & 0x80) != 0){
      return  ((m_Internal[0] & 0xff) | (m_Internal[1] & 0xff) << 8 | (m_Internal[2] & 0xff) << 16 | (-1 & 0xff) << 24);
    }else{
      return  ((m_Internal[0] & 0xff) | (m_Internal[1] & 0xff) << 8 | (m_Internal[2] & 0xff) << 16 | (0 & 0xff) << 24);
    }
  }
};


r2_aud* ReadWav(const char * filepath);
r2_aud* ReadPcm(const char * filepath);



r2_aud* r2_aud_malloc(int cn, int sr, int len){
  
  r2_aud* pAud = Z_SAFE_NEW(pAud, r2_aud) ;
  pAud->cn = cn ;
  pAud->sr = sr ;
  pAud->len = len ;
  pAud->data = Z_SAFE_NEW_AR2(pAud->data, float, cn, len) ;
  return  pAud ;
}


int r2_aud_free(r2_aud* pAud){
  
  Z_SAFE_DEL_AR2(pAud->data) ;
  Z_SAFE_DEL(pAud) ;
  return  0 ;
}


r2_aud* r2_aud_rs(const r2_aud* pAud, int iSr){
  
  assert(pAud != NULL && pAud->sr != iSr) ;
  
  r2_rs_htask hRs = r2_rs_create(pAud->cn, pAud->sr, iSr, iSr / 1000 * 10) ;
  
  float** pData_Out = NULL ;
  int iLen_Out = 0 ;
  r2_rs_process_float(hRs, (const float**)pAud->data, pAud->len, pData_Out, iLen_Out) ;
  
  r2_aud* pAud_New = r2_aud_malloc(pAud->cn, iSr, iLen_Out) ;
  
  for (int i = 0 ; i < pAud->cn ; i ++) {
    memcpy(pAud_New->data[i], pData_Out[i], sizeof(float) * iLen_Out) ;
  }
  
  r2_rs_free(hRs) ;
  
  return pAud_New ;
  
}

r2_aud* r2_aud_in(const char* pAudPath, int iSr){
  
  //
  char line[1024];
  strcpy(line,pAudPath);
  strtok(line,"\r\n\t");
  
  //strlwr(line);
  size_t ssize = strlen(line);
  while (ssize > 0 && line[ssize-1] == ' ') {
    ssize -- ;
  }
  line[ssize] = '\0';
  
  r2_aud* pAud = NULL ;
  
  char * temp = strrchr(line,'.') + 1 ;
  if (strcasecmp(temp,"pcm") == 0 ) {
    pAud =  ReadPcm(line);
  } else if (strcasecmp(temp,"wav") == 0 ) {
    pAud =  ReadWav(line);
  } else {
    ZLOG_ERROR("Error in Read Audio File, Not Support Format %s",temp);
  }
  
  //重采样
  if ((pAud != NULL) && (iSr != 0) && (pAud->sr != iSr)) {
    r2_aud* pAud_New = r2_aud_rs(pAud, iSr) ;
    r2_aud_free(pAud) ;
    pAud = pAud_New ;
  }
  
  
  return pAud ;
  
}


r2_aud* r2_pcm_in(const char* pPcmPath, int iOffset, int iSr, int iCn, pcm_format iFormat){
  
  //
  char line[1024];
  strcpy(line,pPcmPath);
  strtok(line,"\r\n\t");
  
  //strlwr(line);
  size_t ssize = strlen(line);
  while (ssize > 0 && line[ssize-1] == ' ') {
    ssize -- ;
  }
  line[ssize] = '\0';
  
  int iBit = 0 ;
  switch (iFormat) {
    case format_int16:
      iBit = 2 ;
      break;
    case format_int24:
      iBit = 3 ;
      break;
    case format_int32:
      iBit = 4 ;
      break;
    case format_float32:
      iBit = 4 ;
      break;
    default:
      return  NULL;
  }
  
  FILE* pFile = fopen(line, "rb");
  if (pFile == NULL) {
    return NULL ;
  }
  fseek(pFile, 0, SEEK_END) ;
  int iLen = (ftell(pFile) - iOffset) / iCn / iBit ;
  r2_aud* pAud = r2_aud_malloc(iCn, iSr, iLen);
  char* pData = Z_SAFE_NEW_AR1(pData, char, iLen * iCn * iBit) ;
  fseek(pFile, iOffset, SEEK_SET);
  fread(pData, sizeof(char), iLen * iCn * iBit, pFile) ;
  fclose(pFile);
  
  if (iFormat == format_int16) {
    short* pData_Int16 = (short*) pData ;
    for (int i = 0; i < iCn ; i ++) {
      for (int j = 0 ; j < iLen ; j ++) {
        pAud->data[i][j] = pData_Int16[j * iCn + i] ;
      }
    }
  }else if (iFormat == format_int24){
    r2_int24* pData_Int24 = (r2_int24*) pData ;
    for (int i = 0; i < iCn ; i ++) {
      for (int j = 0 ; j < iLen ; j ++) {
        pAud->data[i][j] = pData_Int24[j * iCn + i].toint() ;
      }
    }
  }else if (iFormat == format_int32){
    int* pData_Int32 = (int*) pData ;
    for (int i = 0; i < iCn ; i ++) {
      for (int j = 0 ; j < iLen ; j ++) {
        pAud->data[i][j] = pData_Int32[j * iCn + i] ;
      }
    }
  }else if (iFormat == format_float32){
    float* pData_Float32 = (float*) pData ;
    for (int i = 0; i < iCn ; i ++) {
      for (int j = 0 ; j < iLen ; j ++) {
        pAud->data[i][j] = pData_Float32[j * iCn + i] ;
      }
    }
  }else{
    Z_SAFE_DEL_AR1(pData) ;
    return NULL ;
  }
  
  Z_SAFE_DEL_AR1(pData) ;
  return pAud ;
  
}




int r2_aud_out(const char* pAudPath, r2_aud* pAud){
  
  FILE * pFile = fopen(pAudPath,"wb");
  if(pFile == NULL) {
    return -1 ;
  }
  
  RIFF riff ;
  FMT fmt ;
  DATA data ;
  float maxdata = 1.0f, scale = 1.0f ;
  
  for (int i = 0 ; i < pAud->cn ; i ++) {
    float aaa = 0.0f ;
    for (int j = 0 ; j < pAud->len ; j ++) {
      aaa += pAud->data[i][j] ;
    }
    aaa = aaa / pAud->len ;
    for (int j = 0 ; j < pAud->len ; j ++) {
      pAud->data[i][j] = pAud->data[i][j] - aaa ;
    }
  }
  
  for (int i = 0 ; i < pAud->len ; i ++) {
    for (int j = 0 ; j < pAud->cn ; j ++) {
      if (maxdata < fabs(pAud->data[j][i])) {
        maxdata = fabs(pAud->data[j][i]);
      }
    }
  }
  if (maxdata > 32000.0f) {
    scale = 32000.0f / maxdata ;
  }else{
    scale = 1.0f ;
  }
  
  int size = pAud->cn * pAud->len * sizeof(short);
  short *pData = Z_SAFE_NEW_AR1(pData, short,pAud->cn * pAud->len);
  for (int i = 0 ; i < pAud->len ; i ++) {
    for (int j = 0 ; j < pAud->cn ; j ++) {
      pData[i * pAud->cn + j] = pAud->data[j][i] * scale;
    }
  }
  
  strncpy(riff.riffID, "RIFF", 4);
  riff.riffSIZE = sizeof(RIFF) + sizeof(FMT) + sizeof(DATA) + size - 8;
  strncpy(riff.riffFORMAT, "WAVE", 4);
  fwrite(&riff,sizeof(RIFF),1,pFile);
  
  //FMT
  strncpy(fmt.fmtID, "fmt ", 4);
  fmt.fmtSIZE = sizeof(WAVEFORM);
  fmt.fmtFORMAT.wFormatTag = 1;
  fmt.fmtFORMAT.nChannels = pAud->cn;
  fmt.fmtFORMAT.nSamplesPerSec = pAud->sr;
  fmt.fmtFORMAT.nBlockAlign = 2*pAud->cn;
  fmt.fmtFORMAT.nAvgBytesPerSec = fmt.fmtFORMAT.nBlockAlign*pAud->sr;
  fmt.fmtFORMAT.wBitsPerSample = 16;
  fwrite(&fmt,sizeof(FMT),1,pFile);
  
  //DATA
  strncpy(data.dataID, "data", 4);
  data.dataSIZE = size ;
  fwrite(&data,sizeof(DATA),1,pFile);
  fwrite(pData,1,size,pFile);
  
  fclose(pFile);
  
  Z_SAFE_DEL_AR1(pData);
  
  return 0 ;
}


r2_aud* ReadWav(const char * pWavPath){
  
  RIFF riff ;
  FMT fmt ;
  DATA data ;
  
  FILE * pFile = fopen(pWavPath,"rb");
  if (pFile == NULL) {
    return NULL ;
  }
  fread(&riff,sizeof(RIFF),1,pFile);
  fread(&fmt,sizeof(FMT),1,pFile);
  if (!CheckID( riff.riffID, 'R', 'I', 'F', 'F')||!CheckID( riff.riffFORMAT, 'W', 'A', 'V', 'E')
      ||!CheckID( fmt.fmtID, 'f', 'm', 't', ' ')) {
    fclose(pFile);
    return NULL ;
  }
  if (fmt.fmtSIZE == 18) {
    fseek(pFile,2,SEEK_CUR);
  }
  fread(&data,sizeof(DATA),1,pFile);
  if (CheckID( data.dataID, 'f', 'a', 'c', 't')) {
    fseek(pFile,data.dataSIZE,SEEK_CUR);
    fread(&data,sizeof(DATA),1,pFile);
  }
  
  if (!CheckID( data.dataID, 'd', 'a', 't', 'a')) {
    fclose(pFile);
    return NULL ;
  }
  
  int sr = fmt.fmtFORMAT.nSamplesPerSec ;
  int cn = fmt.fmtFORMAT.nChannels ;
  int bits = fmt.fmtFORMAT.wBitsPerSample ;
  int tag = fmt.fmtFORMAT.wFormatTag ;
  
  int len = data.dataSIZE/(bits/8*cn) ;
  r2_aud* pAud = r2_aud_malloc(cn, sr, len);
  
  unsigned char* pBuff = Z_SAFE_NEW_AR1(pBuff, unsigned char, data.dataSIZE) ;
  fread(pBuff, 1, data.dataSIZE, pFile) ;
  fclose(pFile) ;
  
  if (bits == 8) {
    if (tag == 6) {
      for (int i = 0 ; i < cn ; i ++) {
        for (int j = 0 ; j < len ; j ++) {
          pAud->data[i][j] = Alaw2Linear_SG[pBuff[j*cn + i]] ;
        }
      }
    }else if(tag == 7){
      for (int i = 0 ; i < cn ; i ++) {
        for (int j = 0 ; j < len ; j ++) {
          pAud->data[i][j] = ulaw2Linear_SG[pBuff[j*cn + i]] ;
        }
      }
    }else if (tag == 1) {
      for (int i = 0 ; i < cn ; i ++) {
        for (int j = 0 ; j < len ; j ++) {
          pAud->data[i][j] = (pBuff[j*cn + i] - 128 ) * 256 ;
        }
      }
    } else {
      ZLOG_ERROR("Error Format in Open File %s",pWavPath);
      r2_aud_free(pAud);
      Z_SAFE_DEL_AR1(pBuff);
      return NULL ;
    }
  }else if(bits == 16){
    
    short* pBuff_Short = (short*) pBuff ;
    for (int i = 0 ; i < cn ; i ++) {
      for (int j = 0 ; j < len ; j ++) {
        pAud->data[i][j] = pBuff_Short[j*cn + i] ;
      }
    }
  }else if(bits == 32){
    if (tag == 1) {
      int* pBuff_Int = (int*) pBuff ;
      for (int i = 0 ; i < cn ; i ++) {
        for (int j = 0 ; j < len ; j ++) {
          pAud->data[i][j] = pBuff_Int[j*cn + i] / 32768  ;
        }
      }
    }else if(tag == 2){
      float* pBuff_Float = (float*) pBuff ;
      for (int i = 0 ; i < cn ; i ++) {
        for (int j = 0 ; j < len ; j ++) {
          pAud->data[i][j] = pBuff_Float[j*cn + i] * 32768 ;
        }
      }
    }else {
      ZLOG_ERROR("Error Format in Open File %s",pWavPath);
      r2_aud_free(pAud);
      Z_SAFE_DEL_AR1(pBuff);
      return NULL ;
    }
    
  }
  
  Z_SAFE_DEL_AR1(pBuff);
  return pAud ;
  
}


r2_aud* ReadPcm(const char * filepath){
  
  
  FILE * pFile = fopen(filepath,"rb");
  if (pFile == NULL) {
    return NULL ;
  }
  fseek(pFile,0,SEEK_END);
  int len = ftell(pFile) / sizeof(float);
  
  fseek(pFile,0,SEEK_SET);
  if (len > 0) {
    r2_aud* pAud = r2_aud_malloc(1,16000,len);
    fread(pAud->data[0], sizeof(float), len, pFile);
    fclose(pFile);
    return pAud ;
  }else{
    ZLOG_ERROR("Empty pcm file %s",filepath);
    fclose(pFile);
    return  NULL ;
  }
  
}




