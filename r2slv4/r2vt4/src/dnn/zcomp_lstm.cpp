//
//  zcomp_lstm.cpp
//  r2vt4
//
//  Created by hadoop on 6/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zcomp_lstm.h"

namespace __r2vt4__ {
  
  //Nnet_Memory_Lstm--------------------------------------------------------------
  Nnet_Memory_Lstm::Nnet_Memory_Lstm(Nnet_Component* pComponent, int iBatchSize):Nnet_Component_Memory(pComponent, iBatchSize){
    
    Nnet_Lstm* pLstm = (Nnet_Lstm*) pComponent ;
    _mem_fw = Z_SAFE_NEW(_mem_fw, Nnet_Memory_Lstm_Cell, pLstm->_cell_fw, iBatchSize) ;
    
  }
  
  Nnet_Memory_Lstm::~Nnet_Memory_Lstm(){
    
    Z_SAFE_DEL(_mem_fw) ;
  }

  int Nnet_Memory_Lstm::ConfigMem(const ZMat_Neon* pMat_In){
    
    
    Nnet_Component_Memory::ConfigMem(pMat_In);
    _mem_fw->ConfigMem(pMat_In) ;
    
    return 0 ;
  }

  //Nnet_Lstm--------------------------------------------------------------
  Nnet_Lstm::Nnet_Lstm(int input_dim, int output_dim):Nnet_Component(input_dim, output_dim,LayerName_LSTM){
    
    _cell_fw = Z_SAFE_NEW(_cell_fw, Nnet_Lstm_Cell, _input_dim, _output_dim, true) ;
    
  }

  Nnet_Lstm::~Nnet_Lstm(){
    
    Z_SAFE_DEL(_cell_fw) ;
  }

  
  /// Forward pass transformation
  void Nnet_Lstm::PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out){
    
    Nnet_Memory_Lstm* pMem_Lstm = (Nnet_Memory_Lstm*)pMem ;
    pMem_Lstm->ConfigMem(pMat_In);
    
    ZMat* pMat_Out_Fw = NULL ;
    _cell_fw->PropagateFnc_Mat(pMat_In, pMem_Lstm->_mem_fw, pMat_Out_Fw) ;
    
    pMem_Lstm->m_pMat_Out_Neon->CopyFrom(pMat_Out_Fw);
    
    pMat_Out = pMem_Lstm->m_pMat_Out_Neon ;
  }

  void Nnet_Lstm::PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out){
    
    assert(0) ;
  }

  
  Nnet_Component_Memory* Nnet_Lstm::GeneratePropagateMemory(int iBatchSize){
    
    Nnet_Memory_Lstm* pMem = Z_SAFE_NEW(pMem, Nnet_Memory_Lstm, this, iBatchSize);
    return pMem ;
  }

  
  /// Reads the component content
  void Nnet_Lstm::ReadData(ZInput* pInput){
    
    _cell_fw->ReadData(pInput);
    
  }
  
  //Nnet_Memory_BLstm--------------------------------------------------------------
  Nnet_Memory_BLstm::Nnet_Memory_BLstm(Nnet_Component* pComponent, int iBatchSize):Nnet_Component_Memory(pComponent, iBatchSize){
    
    Nnet_BLstm* pBLstm = (Nnet_BLstm*) pComponent ;
    
    _mem_fw = Z_SAFE_NEW(_mem_fw, Nnet_Memory_Lstm_Cell, pBLstm->_cell_fw, iBatchSize) ;
    _mem_bw = Z_SAFE_NEW(_mem_bw, Nnet_Memory_Lstm_Cell, pBLstm->_cell_bw, iBatchSize) ;
    
  }

  Nnet_Memory_BLstm::~Nnet_Memory_BLstm(){
    
    Z_SAFE_DEL(_mem_fw) ;
    Z_SAFE_DEL(_mem_bw) ;
  }
  
  int Nnet_Memory_BLstm::ConfigMem(const ZMat_Neon* pMat_In){
    
    Nnet_Component_Memory::ConfigMem(pMat_In);
    _mem_bw->ConfigMem(pMat_In) ;
    _mem_fw->ConfigMem(pMat_In) ;
    return 0 ;
  }
  
  //Nnet_BLstm--------------------------------------------------------------
  Nnet_BLstm::Nnet_BLstm(int input_dim, int output_dim):Nnet_Component(input_dim, output_dim,LayerName_BLSTM){
    
    _cell_fw = Z_SAFE_NEW(_cell_fw, Nnet_Lstm_Cell, _input_dim, _output_dim/2, true) ;
    _cell_bw = Z_SAFE_NEW(_cell_bw, Nnet_Lstm_Cell, _input_dim, _output_dim/2, false) ;
  }

  Nnet_BLstm::~Nnet_BLstm(){
    
    Z_SAFE_DEL(_cell_fw) ;
    Z_SAFE_DEL(_cell_bw) ;
  }

  /// Forward pass transformation
  void Nnet_BLstm::PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out){
    
    Nnet_Memory_BLstm* pMem_BLstm = (Nnet_Memory_BLstm*)pMem ;
    pMem_BLstm->ConfigMem(pMat_In);
    
    ZMat *pMat_Out_Fw = NULL, *pMat_Out_Bw = NULL ;
    _cell_fw->PropagateFnc_Mat(pMat_In, pMem_BLstm->_mem_fw, pMat_Out_Fw) ;
    _cell_bw->PropagateFnc_Mat(pMat_In, pMem_BLstm->_mem_bw, pMat_Out_Bw) ;
    
    for (int i = 0 ; i < pMat_Out_Fw->row ; i ++) {
      memcpy(pMem_BLstm->m_pMat_Out->data[i], pMat_Out_Fw->data[i], sizeof(float) * pMat_Out_Fw->col) ;
      memcpy(pMem_BLstm->m_pMat_Out->data[i] + pMat_Out_Fw->col, pMat_Out_Bw->data[i], sizeof(float) * pMat_Out_Bw->col) ;
    }
    
    pMem_BLstm->m_pMat_Out_Neon->CopyFrom(pMem_BLstm->m_pMat_Out);
    
    pMat_Out = pMem_BLstm->m_pMat_Out_Neon ;
    
  }

  void Nnet_BLstm::PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out){
    
    assert(0) ;
  }
  
  Nnet_Component_Memory* Nnet_BLstm::GeneratePropagateMemory(int iBatchSize){
    
    Nnet_Memory_BLstm* pMem = Z_SAFE_NEW(pMem, Nnet_Memory_BLstm, this, iBatchSize);
    return pMem ;
  }
  
  /// Reads the component content
  void Nnet_BLstm::ReadData(ZInput* pInput){
    
    _cell_fw->ReadData(pInput) ;
    _cell_bw->ReadData(pInput) ;
    
  }
  
};




