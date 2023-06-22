//
//  zg2papi.cpp
//  r2vt4
//
//  Created by hadoop on 5/26/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zg2papi.h"
#include "../zmath.h"

using namespace __r2vt4__ ;

r2_g2p_htask r2_g2p_create(const char* pModelPath){
  
  return  0 ;
}

int r2_g2p_free(r2_g2p_htask hTask){
  
  return 0 ;
}

/**
 *  Do G2P
 *
 *  @param hTask     G2P句柄
 *  @param pGrapheme Grapheme
 *
 *  @return Phoneme
 */
const char* r2_g2p_process(r2_g2p_htask hTask, const char* pGrapheme){
  
  std::string res  ;
  return res.c_str() ;
}




