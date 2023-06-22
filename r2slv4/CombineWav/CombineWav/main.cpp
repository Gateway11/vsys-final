//
//  main.cpp
//  CombineWav
//
//  Created by hadoop on 7/12/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include <iostream>

#include "../../r2vt4/src/aud/zaudapi.h"

int Combine(){
  
  const char * pSrcPath_1 = "/Users/hadoop/Documents/data/PC录音1KHz反向-01.wav" ;
  const char * pSrcPath_2 = "/Users/hadoop/Documents/data/PC录音1KHz反向-02.wav" ;
  const char * pSrcPath_3 = "/Users/hadoop/Documents/data/PC录音1KHz反向-03.wav" ;
  const char * pSrcPath_4 = "/Users/hadoop/Documents/data/PC录音1KHz反向-04.wav" ;
  
  const char * pDesPath = "/Users/hadoop/Documents/data/PC录音1KHz反向.wav" ;
  
  r2_aud* pAud_In_1 = r2_aud_in(pSrcPath_1, 0) ;
  r2_aud* pAud_In_2 = r2_aud_in(pSrcPath_2, 0) ;
  r2_aud* pAud_In_3 = r2_aud_in(pSrcPath_3, 0) ;
  r2_aud* pAud_In_4 = r2_aud_in(pSrcPath_4, 0) ;
  
  
  
  r2_aud* pAud_Out = r2_aud_malloc(4, pAud_In_1->sr, pAud_In_1->len);
  for (int i = 0 ; i < pAud_In_1->len ; i ++) {
    pAud_Out->data[0][i] = pAud_In_1->data[0][i] * 100 ;
    pAud_Out->data[1][i] = pAud_In_2->data[0][i] * 100 ;
    pAud_Out->data[2][i] = pAud_In_3->data[0][i] * 100 ;
    pAud_Out->data[3][i] = pAud_In_4->data[0][i] * 100 ;
  }
  
  //  memcpy(pAud_Out->data[0], pAud_In_1->data[0], sizeof(float) * pAud_In_1->len);
  //  memcpy(pAud_Out->data[1], pAud_In_2->data[0], sizeof(float) * pAud_In_1->len);
  //  memcpy(pAud_Out->data[2], pAud_In_3->data[0], sizeof(float) * pAud_In_1->len);
  //  memcpy(pAud_Out->data[3], pAud_In_4->data[0], sizeof(float) * pAud_In_1->len);
  
  r2_aud_out(pDesPath, pAud_Out);
  
  r2_aud_free(pAud_In_1);
  r2_aud_free(pAud_In_2);
  r2_aud_free(pAud_In_3);
  r2_aud_free(pAud_In_4);
  r2_aud_free(pAud_Out);
  
  return  0 ;
}

int removewav(){
  
  const char * pSrcPath = "/Users/hadoop/Documents/filelst/111/音轨-2.wav" ;
  const char * pDesPath = "/Users/hadoop/Documents/filelst/111/音轨-2-left.wav" ;
  
  r2_aud* pAud_In = r2_aud_in(pSrcPath, 0) ;
  r2_aud* pAud_Out = r2_aud_malloc(pAud_In->cn, pAud_In->sr, pAud_In->len);
  
  int iFrmSize1 = pAud_In->sr / 10 ;
  int iFrmSize2 = iFrmSize1 - 200 ;
  int iFrmNum = pAud_In->len / iFrmSize1 ;
  
  for (int i = 0 ; i < pAud_In->cn ; i ++) {
    for (int j = 0 ; j < iFrmNum; j ++) {
      memcpy(pAud_Out->data[i] + j * iFrmSize2, pAud_In->data[i] + j * iFrmSize1, iFrmSize2 * sizeof(float));
    }
  }
  
  r2_aud_out(pDesPath, pAud_Out);
  
  r2_aud_free(pAud_In);
  r2_aud_free(pAud_Out);
  
  return  0 ;
  
}


int Pcm2Wav(){
  
  const char * pSrcPath = "/Users/hadoop/2017-08-09/haier2_5.pcm" ;
  const char * pDesPath = "/Users/hadoop/2017-08-09/ttq.wav" ;
  
  r2_aud* pAud_In = r2_pcm_in(pSrcPath, 0, 48000, 6, format_int32) ;
  //r2_aud* pAud_In = r2_aud_in(pSrcPath, 16000);
  //pAud_In->cn = 6 ;
  r2_aud* pAud_Out = r2_aud_rs(pAud_In, 16000) ;
  r2_aud_out(pDesPath, pAud_Out) ;
  //r2_aud_out(pDesPath, pAud_In) ;

  
  r2_aud_free(pAud_In);
  //r2_aud_free(pAud_Out) ;
  
  return  0 ;
  
  
}

int main(int argc, const char * argv[]) {
  

  Pcm2Wav();
  
  return 0 ;
  
  
}
