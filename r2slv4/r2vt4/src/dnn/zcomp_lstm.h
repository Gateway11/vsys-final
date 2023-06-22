//
//  zcomp_lstm.h
//  r2vt4
//
//  Created by hadoop on 6/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zcomp_lstm__
#define __r2vt4__zcomp_lstm__

#include "zcomp_lstm_cell.h"
#include "zcomp.h"

namespace __r2vt4__ {

  //Nnet_Lstm--------------------------------------------------------------
  class Nnet_Memory_Lstm : public Nnet_Component_Memory{
    
  public:
    Nnet_Memory_Lstm(Nnet_Component* pComponent, int iBatchSize);
    virtual ~Nnet_Memory_Lstm();
    
  public:
    virtual int ConfigMem(const ZMat_Neon* pMat_In);
    
    Nnet_Memory_Lstm_Cell* _mem_fw ;
    
  };
  
  class Nnet_Lstm : public Nnet_Component{
  public:
    Nnet_Lstm(int input_dim, int output_dim);
    virtual ~Nnet_Lstm();
    
  public:
    /// Forward pass transformation
    virtual void PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out);
    virtual void PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out);
    
    virtual Nnet_Component_Memory* GeneratePropagateMemory(int iBatchSize);
    
    /// Reads the component content
    virtual void ReadData(ZInput* pInput);
    
  public:
    
    Nnet_Lstm_Cell* _cell_fw ;
  };
  
  //Nnet_BLstm--------------------------------------------------------------
  class Nnet_Memory_BLstm : public Nnet_Component_Memory{
    
  public:
    Nnet_Memory_BLstm(Nnet_Component* pComponent, int iBatchSize);
    virtual ~Nnet_Memory_BLstm();
    
  public:
    virtual int ConfigMem(const ZMat_Neon* pMat_In);
    
    Nnet_Memory_Lstm_Cell* _mem_fw ;
    Nnet_Memory_Lstm_Cell* _mem_bw ;
    
  };
  
  class Nnet_BLstm : public Nnet_Component{
  public:
    Nnet_BLstm(int input_dim, int output_dim);
    virtual ~Nnet_BLstm();
    
  public:
    /// Forward pass transformation
    virtual void PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out);
    virtual void PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out);
    
    virtual Nnet_Component_Memory* GeneratePropagateMemory(int iBatchSize);
    
    /// Reads the component content
    virtual void ReadData(ZInput* pInput);
    
  public:
    
    Nnet_Lstm_Cell* _cell_fw ;
    Nnet_Lstm_Cell* _cell_bw ;
    
  };

};


#endif /* __r2vt4__zcomp_lstm__ */
