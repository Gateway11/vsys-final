//
//  zcomp.h
//  r2vt4
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zcomp__
#define __r2vt4__zcomp__

#include "../zmath.h"
#include "../mat/zmat.h"
#include "../mat/zmat_neon.h"
#include "../io/zinput.h"
#include "../io/zoutput.h"

namespace __r2vt4__ {
  
#define   LayerName_Sigmoid           "<Sigmoid>"
#define   LayerName_Tanh              "<Tanh>"
#define   LayerName_Relu              "<Relu>"
#define   LayerName_SoftMax           "<Softmax>"
#define   LayerName_AffineTransform   "<AffineTransform>"
#define   LayerName_LinearTransform   "<LinearTransform>"
#define   LayerName_LSTM              "<LSTM>"
#define   LayerName_BLSTM             "<BLSTM>"
#define   LayerName_BLSTM_KALDI       "<BLstmProjectedStreams>"
  
  class Nnet_Component ;
  class Nnet_Component_Memory{
  public:
    Nnet_Component_Memory(Nnet_Component* pComponent, int iBatchSize);
    virtual ~Nnet_Component_Memory();
    
    virtual int ConfigMem(const ZMat_Neon* pMat_In);
    
  public:
    
    ZMat_Neon*  m_pMat_Out_Neon ;
    ZMat*   m_pMat_Out ;
    int m_iFrmNum_Total ;
    
    ZVec* m_pVec_Out ;
    Nnet_Component* m_pComponent ;
  };
  
  class Nnet_Component {
  public:
    Nnet_Component(int input_dim, int output_dim, std::string component_name) :
    _input_dim(input_dim),
    _output_dim(output_dim),
    _component_name(component_name)
    { }
    
    virtual ~Nnet_Component()
    { }
    
    /// Abstract interface for propagation/backpropagation
  public:
    /// Forward pass transformation (to be implemented by descending class...)
    virtual void PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out){};
    virtual void PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out){};
    
    /// Generate Forward Needed Memory
    virtual Nnet_Component_Memory* GeneratePropagateMemory(int iBatchSize){
      Nnet_Component_Memory* pMem = Z_SAFE_NEW(pMem, Nnet_Component_Memory, this, iBatchSize);
      return pMem ;
    } ;
    
    /// Virtual interface for initialization and I/O,
  public:
    /// Reads the component content
    virtual void ReadData(ZInput* pInput) {} ;
    
    /// Writes the component content
    virtual void WriteData(ZOutput* pOutput) const {} ;
    
  public:
    void ReadMatrixToNeon(ZInput* pInput, int row, int col, ZMat_Neon* pMatA, ZMat_Neon* pMatB);
    
    /// Data members,
  public:
    int _input_dim;  ///< Dimension of the input of the Component,
    int _output_dim; ///< Dimension of the output of the Component,
    std::string _component_name ;
  };
  
};

#endif /* __r2vt4__zcomp__ */
