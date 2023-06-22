//
//  zcomp_activation.cpp
//  r2vt4
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zcomp_activation.h"

namespace __r2vt4__ {
  
  //Nnet_Sigmoid--------------------------------------------------------------
  Nnet_Sigmoid::Nnet_Sigmoid(int input_dim)
  :Nnet_Component(input_dim, input_dim, LayerName_Sigmoid){
    
  }
  
  Nnet_Sigmoid::~Nnet_Sigmoid(){
    
  }
  
  void Nnet_Sigmoid::PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out){
    
    pMem->ConfigMem(pMat_In);
    pMat_Out = pMem->m_pMat_Out_Neon ;
    
    pMat_Out->Copy(pMat_In);
    pMat_Out->RowSigMoid();
    
  };
  
  void Nnet_Sigmoid::PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out){
    
    pVec_Out = pMem->m_pVec_Out ;
    pVec_Out->Copy(pVec_In) ;
    pVec_Out->RowSigMoid() ;
  };
  
  
  //Nnet_SoftMax--------------------------------------------------------------
  Nnet_SoftMax::Nnet_SoftMax(int input_dim)
  :Nnet_Component(input_dim, input_dim, LayerName_SoftMax){
    
  }
  
  Nnet_SoftMax::~Nnet_SoftMax(){
    
  }
  
  void Nnet_SoftMax::PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out){
    
    pMem->ConfigMem(pMat_In);
    
    pMat_Out = pMem->m_pMat_Out_Neon ;
    
    
    pMat_In->CopyTo(pMem->m_pMat_Out);
    pMem->m_pMat_Out->RowSoftMax() ;
    
    pMat_Out->CopyFrom(pMem->m_pMat_Out);
    
  };
  
  void Nnet_SoftMax::PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out){
    
    pVec_Out = pMem->m_pVec_Out ;
    pVec_Out->Copy(pVec_In) ;
    pVec_Out->RowSoftMax() ;
  };
  
  //Nnet_Tanh--------------------------------------------------------------
  Nnet_Tanh::Nnet_Tanh(int input_dim)
  :Nnet_Component(input_dim, input_dim,LayerName_Tanh){
    
  }
  
  Nnet_Tanh::~Nnet_Tanh(){
    
  }
  
  void Nnet_Tanh::PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out){
    
    pMem->ConfigMem(pMat_In);
    pMat_Out = pMem->m_pMat_Out_Neon ;
    
    pMat_Out->Copy(pMat_In);
    pMat_Out->RowTanh();
  };
  
  void Nnet_Tanh::PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out){
    
    pVec_Out = pMem->m_pVec_Out ;
    pVec_Out->Copy(pVec_In) ;
    pVec_Out->RowTanh();
    
  };
  
  //Nnet_Relu--------------------------------------------------------------
  Nnet_Relu::Nnet_Relu(int input_dim)
  :Nnet_Component(input_dim, input_dim,LayerName_Relu){
    
  }
  
  Nnet_Relu::~Nnet_Relu(){
    
  }
  
  void Nnet_Relu::PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out){
    
    pMem->ConfigMem(pMat_In);
    pMat_Out = pMem->m_pMat_Out_Neon ;
    
    pMat_Out->Copy(pMat_In);
    pMat_Out->RowRelu();
    
  };
  
  void Nnet_Relu::PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out){
    
    pVec_Out = pMem->m_pVec_Out ;
    pVec_Out->Copy(pVec_In) ;
    pVec_Out->RowRelu() ;
    
  };
  
  //Nnet_LayerNorm--------------------------------------------------------------
  Nnet_LayerNorm::Nnet_LayerNorm(int input_dim){
    
    _scale = Z_SAFE_NEW(_scale, ZVec, input_dim);
    _shift = Z_SAFE_NEW(_shift, ZVec, input_dim);
    
  }
  
  Nnet_LayerNorm::~Nnet_LayerNorm(){
    
    Z_SAFE_DEL(_scale);
    Z_SAFE_DEL(_shift);
  }
  
  
  void Nnet_LayerNorm::PropagateFnc_Mat(ZMat* pMat_In){
    
    for (int i = 0 ; i < pMat_In->row ; i ++) {
      float mean = 0 , var = 0 ;
      for (int j = 0 ; j < pMat_In->col ; j ++) {
        mean += pMat_In->data[i][j] ;
        var += pMat_In->data[i][j] * pMat_In->data[i][j] ;
      }
      mean = mean / pMat_In->col ;
      var = sqrt(var / pMat_In->col - mean * mean + 1e-5f) ;
      
      for (int j = 0 ; j < pMat_In->col ; j ++) {
        pMat_In->data[i][j] = (pMat_In->data[i][j] - mean) / var ;
      }
    }
    
    pMat_In->RowMul(_scale) ;
    pMat_In->RowAdd(_shift) ;
    
    return ;
    
  };
  
  void Nnet_LayerNorm::PropagateFnc_Vec(ZVec* pVec_In){
    
    float mean = 0 , var = 0 ;
    for (int i = 0 ; i < pVec_In->size ; i ++) {
      mean += pVec_In->data[i] ;
      var += pVec_In->data[i] * pVec_In->data[i]  ;
    }
    mean = mean / pVec_In->size ;
    var = sqrt(var / pVec_In->size - mean * mean + 1e-5f) ;
    
    for (int i = 0 ; i < pVec_In->size ; i ++) {
      pVec_In->data[i] = (pVec_In->data[i] - mean) / var ;
    }
    pVec_In->RowMul(_scale);
    pVec_In->RowAdd(_shift);
    
    return ;
    
    
  };
  
  /// Reads the component content
  void Nnet_LayerNorm::ReadData(ZInput* pInput) {
    
    _scale->Read(pInput);
    _shift->Read(pInput);
  }
  
  /// Writes the component content
  void Nnet_LayerNorm::WriteData(ZOutput* pOutput) const {
    
    _scale->Write(pOutput);
    _shift->Write(pOutput);
    
  } ;
  
};





