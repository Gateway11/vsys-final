//
//  main.cpp
//  testsl
//
//  Created by hadoop on 7/14/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include <iostream>
#include "../../r2vt4/src/aud/zaudapi.h"
#include "../../r2vt4/src/sl/zslapi.h"


int main(int argc, const char * argv[]) {
  
  int iMicNum = 6 ;
  const char* pAudPath = "/Users/hadoop/record.wav" ;
  r2_aud* pAud_In =  r2_aud_in(pAudPath, 16000) ;
  for (int i = 0 ; i < pAud_In->cn ; i ++) {
    float total = 0.0f ;
    for (int j = 0 ; j < pAud_In->len; j ++) {
      total += pAud_In->data[i][j] ;
    }
    total = total / pAud_In->len ;
    for (int j = 0 ; j < pAud_In->len; j ++) {
      pAud_In->data[i][j] = pAud_In->data[i][j] - total  ;
    }
  }
  //  r2_aud_out("/Users/hadoop/record_16k.wav", pAud_In);
  
  
  //  const char* pAudPath = "/Users/hadoop/record_16k.wav" ;
  //  r2_aud* pAud_In =  r2_aud_in(pAudPath, 16000) ;
  
  
  //  for (int i = 0 ; i < iMicNum ; i ++) {
  //    memcpy(pAud_In->data[i], pAud_In->data[i+2], sizeof(float) * pAud_In->len) ;
  //  }
  
  //  float pMicDelay[]={0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  //  float pMicPos[]={
  //    +0.0425000000f,+0.0000000000f,0.0f,
  //    +0.0300520382f,+0.0300520382f,0.0f,
  //    +0.0000000000f,+0.0425000000f,0.0f,
  //    -0.0300520382f,+0.0300520382f,0.0f,
  //    -0.0425000000f,+0.0000000000f,0.0f,
  //    -0.0300520382f,-0.0300520382f,0.0f,
  //    +0.0000000000f,-0.0425000000f,0.0f,
  //    +0.0300520382f,-0.0300520382f,0.0f };
  
  float pMicDelay[]={0.0f, 1.0f/96000.0f, 0.0f, 1.0f/96000.0f, 0.0f, 1.0f/96000.0f, 0.0f, 1.0f/96000.0f};
  float pMicPos[]={
    0.00000001,0.03750000,0.00000000,
    0.00000000,-0.03750000,0.00000000,
    0.03247596,0.01875000,0.00000000,
    -0.03247595,-0.01875000,0.00000000,
    -0.03247596,0.01875000,0.00000000,
    0.03247595,-0.01875000,0.00000000 };
  
  r2_sl_sysinit();
  r2_sl_htask hTask =  r2_sl_create(iMicNum, pMicPos, pMicDelay);
  
  r2_sl_put_data(hTask, pAud_In->data, pAud_In->len);
  
  r2_sl_free(hTask) ;
  r2_sl_sysexit();
  
  r2_aud_free(pAud_In);

  
  return 0;
}
