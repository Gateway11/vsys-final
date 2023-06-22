//
//  zvtsto.h
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zvtsto__
#define __r2vt4__zvtsto__

#include "../zmath.h"
#include "../dnn/znnet.h"
#include "../fea/zfea2.h"
#include "../fea/zfilter.h"
#include "../buff/zbuff.h"

#include "zvtapi.h"
#include "dep/zphosto.h"
#include "dep/zvtdet.h"


namespace __r2vt4__ {

#ifndef __ARM_ARCH_ARM__
  //#define _VT_AUDIO_
#else
  //#define _VT_AUDIO_
#endif
  
  //ZVtSto--------------------------------------------------------------
  class ZVtSto
  {
  public:
    ZVtSto(const char* pNnetPath, const char* pPhoneTablePath);
    
  public:
    ~ZVtSto(void);
    
  public:
    std::vector<ZVtWord*> ParseWordTriPhoLst(const WordInfo* pWordLst, int iWordNum);
    int PrintWordInfo(ZVtWord* pVtWord, const char* pWordContent_Phone);
  public:
    
    int m_iSr ;
    int m_iBat ;
    int m_iFrmSize ;
    int m_iFrmOffset ;
    
    Nnet*    m_pNnet ;
    std::string m_PhoTablePath ;
    
    
  };



};


#endif /* __r2vt4__zvtsto__ */
