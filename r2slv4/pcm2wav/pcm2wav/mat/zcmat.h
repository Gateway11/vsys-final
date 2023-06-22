//
//  zcmat.h
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zcmat__
#define __r2vt4__zcmat__

#include "../zmath.h"
#include "../io/zinput.h"
#include "../io/zoutput.h"
#include "zmat.h"

namespace __r2vt4__ {
  
  typedef float zcomplex[2] ;

  class ZCMat
  {
  public:
    ZCMat(int row, int col);
  public:
    ~ZCMat(void);
    
    int row ;
    int col ;
    int col_inc ;
    
    zcomplex ** data ;
    
    
  public:
    int Clean();
    int Print(const char* pLabel);
    
  public:
    //Calc Gcc Phat
    //pCMatA[pMatB->data_i[i][0]] .* pCMatA[pMatB->data_i[i][1]] -> pCMatC[i]
    static int CalcGcc(ZCMat* pCMatA, ZMat* pMatB, ZCMat* pCMatC) ;
    
    
  };

};


#endif /* __r2vt4__zcmat__ */
