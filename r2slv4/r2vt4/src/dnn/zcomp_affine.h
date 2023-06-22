//
//  zcomp_affine.h
//  r2vt4
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zcomp_affine__
#define __r2vt4__zcomp_affine__

#include "../zmath.h"
#include "../mat/zmat.h"
#include "../io/zinput.h"
#include "../io/zoutput.h"

#include "zcomp.h"

namespace __r2vt4__ {
  
  class Nnet_AffineTransform: public Nnet_Component{
    
  public:
    Nnet_AffineTransform(int input_dim, int output_dim);
    virtual ~Nnet_AffineTransform();
    
  public:
    /// Forward pass transformation
    virtual void PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out);
    virtual void PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out);
    
    /// Reads the component content
    virtual void ReadData(ZInput* pInput);
    void ReadData_Old(ZInput* pInput);
    
    
    /// Writes the component content
    virtual void WriteData(ZOutput* pOutput) const;
    
  public:
    ZVec *_biases;
    ZMat_Neon *_weights_neon_t;
    
    ZMat_Neon *_weights_neon;
    ZMat_Neon* _biases_neon ;
  };
  

  
  
  
  
};


#endif /* __r2vt4__zcomp_affine__ */
