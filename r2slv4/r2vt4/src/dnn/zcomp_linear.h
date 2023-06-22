//
//  zcomp_linear.h
//  r2vt4
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zcomp_linear__
#define __r2vt4__zcomp_linear__

#include "../zmath.h"
#include "../mat/zmat.h"
#include "../io/zinput.h"
#include "../io/zoutput.h"

#include "zcomp.h"
#include "zcomp_activation.h"

namespace __r2vt4__ {
  
  class Nnet_LinearTransform ;
  class Nnet_Component_Memory_LinearTransfrom:public Nnet_Component_Memory{
    
  public:
    Nnet_Component_Memory_LinearTransfrom(Nnet_Component* pComponent, int iBatchSize);
    virtual ~Nnet_Component_Memory_LinearTransfrom();
    
    virtual int ConfigMem(const ZMat_Neon* pMat_In);
    
  public:
    ZVec* m_pVec_Out_Svd ;
    ZMat_Neon* m_pMat_Out_Svd_Neon ;
  };
  
  
  class Nnet_LinearTransform: public Nnet_Component{
    
  public:
    Nnet_LinearTransform(int input_dim, int output_dim);
    virtual ~Nnet_LinearTransform();
    
  public:
    /// Forward pass transformation
    virtual void PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out);
    virtual void PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out);
    
    virtual Nnet_Component_Memory* GeneratePropagateMemory(int iBatchSize);
    /// Reads the component content
    virtual void ReadData(ZInput* pInput);
    void ReadData_Old(ZInput* pInput, bool ln, bool svd, int svd_dim, std::string acn);
        
  public:
    
    bool _ln ;
    Nnet_LayerNorm* _ln_cell ;
    
    bool _svd ;
    int _svd_dim ;
    
    //batch
    ZMat_Neon *_weights_svd_neon ;
    ZMat_Neon * _biases_svd_neon ;
    
    ZMat_Neon *_weights_neon ;
    ZMat_Neon * _biases_neon ;
    
    //single
    ZMat_Neon *_weights_svd_neon_t ;
    ZVec *_biases_svd ;
    
    ZMat_Neon *_weights_neon_t ;
    ZVec *_biases ;
    
    std::string _acn ;
    
  };
  
  
};


#endif /* __r2vt4__zcomp_linear__ */
