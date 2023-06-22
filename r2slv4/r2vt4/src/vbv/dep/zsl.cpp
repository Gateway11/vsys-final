//
//  zsl.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zsl.h"

namespace __r2vt4__ {

  ZSl::ZSl(int iMicNum, float* pMicPosLst, float* pMicDelay, z_mic_info* pMicInfo_Sl){
    
    m_iMicNum = iMicNum ;
    
    //sl mic
    m_pMicInfo_Sl = z_copymicinfo(pMicInfo_Sl) ;
    
    //engine
    m_pMics_Sl = Z_SAFE_NEW_AR1(m_pMics_Sl,float,m_pMicInfo_Sl->iMicNum * 3);
    for (int i = 0 ; i < m_pMicInfo_Sl->iMicNum ; i ++)	{
      int iMicId = m_pMicInfo_Sl->pMicIdLst[i] ;
      m_pMics_Sl[i * 3 ] = pMicPosLst[iMicId*3] ;
      m_pMics_Sl[i * 3 +1] = pMicPosLst[iMicId*3+1] ;
      m_pMics_Sl[i * 3 +2] = pMicPosLst[iMicId*3+2];
    }
    
    m_pMicI2sDelay = Z_SAFE_NEW_AR1(m_pMicI2sDelay, float, m_pMicInfo_Sl->iMicNum) ;
    memcpy(m_pMicI2sDelay, pMicDelay, sizeof(float) * iMicNum);
    
    if (m_pMicInfo_Sl->iMicNum > 1) {
#ifdef USE_LZHU_SL
      m_hEngine_Sl =  r2_sl_create(m_pMicInfo_Sl->iMicNum, m_pMics_Sl, m_pMicI2sDelay);
#else
      m_hEngine_Sl =  r2_sourceloaction_create(m_pMicI2sDelay, m_pMicInfo_Sl->iMicNum, m_pMics_Sl);
#endif
    }else{
      m_hEngine_Sl = NULL ;
    }

    m_pData_Sl = Z_SAFE_NEW_AR1(m_pData_Sl, float*, m_pMicInfo_Sl->iMicNum) ;
    
  }
  
  ZSl::~ZSl(void){
    
    Z_SAFE_DEL_AR1(m_pData_Sl) ;
    
    Z_SAFE_DEL_AR1(m_pMicI2sDelay);
    
    if (m_pMicInfo_Sl->iMicNum > 1) {
#ifdef USE_LZHU_SL
      r2_sl_free(m_hEngine_Sl);
#else
      r2_sourcelocation_free(m_hEngine_Sl);
#endif
    }

    
    z_free_micinfo(m_pMicInfo_Sl);
    Z_SAFE_DEL_AR1(m_pMics_Sl);
    
  }
  
  int ZSl::PutData(float** pfDataBuff, int iDataLen){
    
    for (int i = 0 ; i < m_pMicInfo_Sl->iMicNum ; i ++)	{
      m_pData_Sl[i] = pfDataBuff[m_pMicInfo_Sl->pMicIdLst[i]] ;
    }
    if (m_pMicInfo_Sl->iMicNum > 1) {
#ifdef USE_LZHU_SL
      r2_sl_put_data(m_hEngine_Sl, m_pData_Sl, iDataLen);
#else
      r2_sourcelocation_process_data(m_hEngine_Sl, m_pData_Sl, iDataLen);
#endif
    }

    
    return 0 ;
  }
  
  int ZSl::GetSl(int iStartPos, int iEndPos, float pSlInfo[3] ){

    if (m_pMicInfo_Sl->iMicNum > 1) {
#ifdef USE_LZHU_SL
      r2_sl_get_candidate(m_hEngine_Sl, iStartPos, iEndPos, pSlInfo, 1);
#else
      r2_sourcelocation_get_candidate(m_hEngine_Sl, iStartPos, iEndPos, pSlInfo, 1);
#endif
    }
    
    return 0 ;
  }
  
  int ZSl::GetSl2(int iStartPos, int iEndPos, float* pSlInfo){
    
    if (m_pMicInfo_Sl->iMicNum > 1) {
#ifdef USE_LZHU_SL
    r2_sl_get_candidate(m_hEngine_Sl, iStartPos, iEndPos, pSlInfo, 2);
#else
    r2_sourcelocation_get_candidate(m_hEngine_Sl, iStartPos, iEndPos, pSlInfo, 2);
#endif
    }
    
    return 0 ;
    
  }
  
  int ZSl::Reset(){
    
    if (m_pMicInfo_Sl->iMicNum > 1) {
#ifdef USE_LZHU_SL
      r2_sl_free(m_hEngine_Sl);
      m_hEngine_Sl =  r2_sl_create(m_pMicInfo_Sl->iMicNum, m_pMics_Sl, m_pMicI2sDelay);
#else
      r2_sourcelocation_free(m_hEngine_Sl);
      m_hEngine_Sl =  r2_sourceloaction_create(m_pMicI2sDelay, m_pMicInfo_Sl->iMicNum, m_pMics_Sl);
#endif
    }
    
    return 0 ;
  }

};




