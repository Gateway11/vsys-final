//
//  zg2papi.h
//  r2vt4
//
//  Created by hadoop on 5/26/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zg2papi__
#define __r2vt4__zg2papi__

#ifdef __cplusplus
extern "C" {
#endif
  
  //Grapheme-to-Phoneme
  
  /** task handle */
  typedef long long r2_g2p_htask ;
  
  /**
   *  创建G2P句柄
   *
   *  @param pModelPath 模型位置
   *
   *  @return G2P句柄
   */
  r2_g2p_htask r2_g2p_create(const char* pModelPath);
  
  /**
   *  释放G2P句柄
   *
   *  @param hTask G2P句柄
   *
   *  @return  0:成功；1：失败
   */
  int r2_g2p_free(r2_g2p_htask hTask);
  
  /**
   *  Do G2P
   *
   *  @param hTask     G2P句柄
   *  @param pGrapheme Grapheme
   *
   *  @return Phoneme
   */
  const char* r2_g2p_process(r2_g2p_htask hTask, const char* pGrapheme);
  
  
#ifdef __cplusplus
};
#endif



#endif /* __r2vt4__zg2papi__ */
