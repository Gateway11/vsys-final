//
//  zslapi.h
//  r2sl3
//
//  Created by hadoop on 9/27/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

//  Sound source location module encapsulation

#ifndef __r2sl3__zslapi__
#define __r2sl3__zslapi__

#ifdef __cplusplus
extern "C" {
#endif
  
  
  /** task handle */
  typedef long long r2_sourcelocation_htask ;
  
  /************************************************************************/
  /** System Init	, Exit
   */
  int r2_sourcelocation_sysinit();
  int r2_sourcelocation_sysexit();
  
  /************************************************************************/
  /** Task Alloc , Free
   */
  r2_sourcelocation_htask r2_sourceloaction_create(float *pDelays, int iMicNum, float* pMicPos, int iFlagArrayFormation);
  int r2_sourcelocation_free(r2_sourcelocation_htask hTask);
  
  /************************************************************************/
  /** R2 SL
   *
   * pWaveBuff: iWavLen * iCn
   * iFlag: 0 End
   */
  float r2_sourcelocation_process_data(r2_sourcelocation_htask hTask, float** pfDataBuff, int iDataLen);
  float r2_sourcelocation_get_candidate(r2_sourcelocation_htask hTask, int iStartPos, int iEndPos, float *pfCandidates, int iCandiNum);
  
  int r2_sourcelocation_reset(r2_sourcelocation_htask hTask);
  
#ifdef __cplusplus
};
#endif





#endif /* __r2sl3__zslapi__ */
