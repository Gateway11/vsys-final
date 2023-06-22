//
//  r2mem_vad3.h
//  r2ad_offline
//
//  Created by hadoop on 8/1/16.
//  Copyright © 2016 hadoop. All rights reserved.
//

#ifndef __r2ad_offline__r2mem_vad3__
#define __r2ad_offline__r2mem_vad3__

#include "NNVadIntf.h"


namespace __r2ad_offline__ {
  
  class r2mem_vad3
  {
  public:
    r2mem_vad3(int iMicNum, int iMicId_Vad);
  public:
    ~r2mem_vad3(void);
    
    int process(float** pData_In, int iLen_In, float**& pData_Out, int& iLen_Out);
    
    int reset();
   
  protected:
    int AddOutData(float** pData_Out, int iOffset, int iLen_Out);
    
  public:
    
    int m_iMicNum ;
    int m_iMicId_Vad ;
    int m_iFrmSize ;
    
    int m_iLen_Out ;
    int m_iLen_Out_Total ;
    float ** m_pData_Out ;
    
    int m_iVadState ;
    
    //vad
    VD_HANDLE m_hEngine_Vad ;
  };
};


#endif /* __r2ad_offline__r2mem_vad3__ */
