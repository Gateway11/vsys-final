//
//  zns.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zns__
#define __r2vt4__zns__

#include "../../zmath.h"
#include "r2ssp.h"

namespace __r2vt4__ {

  class ZNs
  {
  public:
    ZNs(int iMicNum, z_mic_info* pMicInfo_Ns, int iNsMode);
    virtual ~ZNs(void);
    
  public:
    int Process(const float** pData_In, int iLen_In, float** &pData_Out, int &iLen_Out);
    
  protected:
    
    int   m_iMicNum ;
    z_mic_info* m_pMicInfo_Ns ;
    
    
    int   m_iFrmSize ;
    
    r2ssp_handle*  m_pHandle ;
    
    int m_iLen_Out_Total ;
    float** m_pData_Out ;
    
    
  };
  
};


#endif /* __r2vt4__zns__ */
