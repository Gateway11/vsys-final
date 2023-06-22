//
//  zcfapi.h
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zcfapi__
#define __r2vt4__zcfapi__

#ifdef __cplusplus
extern "C" {
#endif
  
  /** task handle */
  typedef long long r2_cf_htask ;
  
  /************************************************************************/
  /** System Init	, Exit
   */
  int r2_cf_sysinit();
  int r2_cf_sysexit();
  
  /************************************************************************/
  /** Task Alloc , Free
   */
  r2_cf_htask r2_cf_create(const char* pNnetPath, int iCn);
  int r2_cf_free(r2_cf_htask hTask);
  
  /************************************************************************/
  /** classify
   *
   * pWaveBuff: iWavLen * iCn
   * iFlag: 0 End
   */
  float r2_cf_check_buff(r2_cf_htask hTask, const float** pWavBuff, int iWavLen, int iCn, int iFlag);
  float r2_cf_check_file(r2_cf_htask hTask, const char* pFilePath, int iFlag);
  
  int r2_cf_get_score(r2_cf_htask hTask, const float* pWavData,int iCn, int iWavLen, float* pScore, int iScoreLen);
  
#ifdef __cplusplus
};
#endif


#endif /* __r2vt4__zcfapi__ */
