//
//  zcomp_activation.h
//  r2vt4
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zcomp_activation__
#define __r2vt4__zcomp_activation__

#include "../zmath.h"
#include "../mat/zmat.h"
#include "../io/zinput.h"
#include "../io/zoutput.h"

#include "zcomp.h"

namespace __r2vt4__ {
  
  
  //Nnet_Sigmoid--------------------------------------------------------------
  class Nnet_Sigmoid: public Nnet_Component{
  public:
    Nnet_Sigmoid(int input_dim);
    virtual ~Nnet_Sigmoid();
    
  public:
    /// Forward pass transformation
    virtual void PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out);
    virtual void PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out);
  };
  
  //Nnet_SoftMax--------------------------------------------------------------
  class Nnet_SoftMax: public Nnet_Component{
  public:
    Nnet_SoftMax(int input_dim);
    ~Nnet_SoftMax();
    
  public:
    /// Forward pass transformation
    virtual void PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out);
    virtual void PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out);
  };
  
  //Nnet_Tanh--------------------------------------------------------------
  class Nnet_Tanh: public Nnet_Component{
  public:
    Nnet_Tanh(int input_dim);
    virtual ~Nnet_Tanh();
    
  public:
    /// Forward pass transformation
    virtual void PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out);
    virtual void PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out);
  };
  
  //Nnet_Relu--------------------------------------------------------------
  class Nnet_Relu: public Nnet_Component{
  public:
    Nnet_Relu(int input_dim);
    virtual ~Nnet_Relu();
    
  public:
    /// Forward pass transformation
    virtual void PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out);
    virtual void PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out);
  };
  
  //Nnet_LayerNorm--------------------------------------------------------------
  class Nnet_LayerNorm{
  public:
    Nnet_LayerNorm(int input_dim);
    virtual ~Nnet_LayerNorm();
    
  public:
    /// Forward pass transformation
    void PropagateFnc_Mat(ZMat* pMat_In);
    void PropagateFnc_Vec(ZVec* pVec_In);
    
    /// Reads the component content
    void ReadData(ZInput* pInput);
    
    /// Writes the component content
    void WriteData(ZOutput* pOutput) const;
    
  public:
    ZVec *_scale ;
    ZVec *_shift ;
    
  };
  
};


#endif /* __r2vt4__zcomp_activation__ */
