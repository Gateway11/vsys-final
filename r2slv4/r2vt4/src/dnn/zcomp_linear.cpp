//
//  zcomp_linear.cpp
//  r2vt4
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zcomp_linear.h"

namespace __r2vt4__ {

  //Nnet_LinearTransform--------------------------------------------------------------
  Nnet_Component_Memory_LinearTransfrom::Nnet_Component_Memory_LinearTransfrom(Nnet_Component* pComponent, int iBatchSize)
  :Nnet_Component_Memory(pComponent, iBatchSize){
    
    Nnet_LinearTransform* pComp = (Nnet_LinearTransform*) m_pComponent ;
    if (pComp->_svd) {
      m_pMat_Out_Svd_Neon = Z_SAFE_NEW(m_pMat_Out_Svd_Neon, ZMat_Neon, m_iFrmNum_Total, pComp->_svd_dim, false);
      m_pVec_Out_Svd = Z_SAFE_NEW(m_pVec_Out_Svd, ZVec, pComp->_svd_dim) ;
    }else {
      m_pMat_Out_Svd_Neon = NULL ;
      m_pVec_Out_Svd = NULL ;
    }
    
  }
  
  Nnet_Component_Memory_LinearTransfrom::~Nnet_Component_Memory_LinearTransfrom(){
    
    Z_SAFE_DEL(m_pVec_Out_Svd) ;
    Z_SAFE_DEL(m_pMat_Out_Svd_Neon) ;
  }
  
  int Nnet_Component_Memory_LinearTransfrom::ConfigMem(const ZMat_Neon* pMat_In){
    
    Nnet_LinearTransform* pComp = (Nnet_LinearTransform*) m_pComponent ;
    
    int rt = Nnet_Component_Memory::ConfigMem(pMat_In);
    if (pComp->_svd ) {
      if (rt > 0) {
        Z_SAFE_DEL(m_pMat_Out_Svd_Neon);
        m_pMat_Out_Svd_Neon = Z_SAFE_NEW(m_pMat_Out_Svd_Neon, ZMat_Neon, m_iFrmNum_Total, pComp->_svd_dim, false);
      }
      
      m_pMat_Out_Svd_Neon->ResetRow(pMat_In->m_iRow);
      
    }
    
    return rt ;
  }
  
  
  //Nnet_LinearTransform--------------------------------------------------------------
  Nnet_LinearTransform::Nnet_LinearTransform(int input_dim, int output_dim)
  :Nnet_Component(input_dim, output_dim, LayerName_LinearTransform){
    
    _ln = false ;
    _svd = false ;
    _acn = "" ;
    
    _svd_dim = 0 ;
    
    _weights_svd_neon = NULL ;
    _weights_svd_neon_t = NULL ;
    _biases_svd = NULL ;
    _biases_svd_neon = NULL ;
    
    _weights_neon = NULL ;
    _weights_neon_t = NULL ;
    _biases = NULL ;
    _biases_neon = NULL ;
    
    _ln_cell = NULL ;
    
  }
  
  Nnet_LinearTransform::~Nnet_LinearTransform(){
    
    Z_SAFE_DEL(_weights_neon);
    Z_SAFE_DEL(_weights_neon_t);
    Z_SAFE_DEL(_biases);
    Z_SAFE_DEL(_biases_neon);
    
    Z_SAFE_DEL(_weights_svd_neon);
    Z_SAFE_DEL(_weights_svd_neon_t);
    Z_SAFE_DEL(_biases_svd);
    Z_SAFE_DEL(_biases_svd_neon);
    
    Z_SAFE_DEL(_ln_cell) ;
    
  }
  
  /// Forward pass transformation
  void Nnet_LinearTransform::PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out){
    
    
    pMem->ConfigMem(pMat_In);
    pMat_Out = pMem->m_pMat_Out_Neon ;
    
    if (_svd) {
      Nnet_Component_Memory_LinearTransfrom* pMem_LT = (Nnet_Component_Memory_LinearTransfrom*)pMem ;
      
      pMem_LT->m_pMat_Out_Svd_Neon->RowCopy_N(_biases_svd_neon);
      pMem_LT->m_pMat_Out_Svd_Neon->Add_aAB_NEON(pMat_In, _weights_svd_neon);
      
      pMat_Out->RowCopy_N(_biases_neon);
      pMat_Out->Add_aAB_NEON(pMem_LT->m_pMat_Out_Svd_Neon, _weights_neon) ;
    }else{
      pMat_Out->RowCopy_N(_biases_neon);
      pMat_Out->Add_aAB_NEON(pMat_In, _weights_neon);
    }
    
    //Trans To Mat
    pMat_Out->CopyTo(pMem->m_pMat_Out) ;
    
    if (_ln) {
      _ln_cell->PropagateFnc_Mat(pMem->m_pMat_Out) ;
    }
    
    //acn
    if (_acn == "Sigmoid") {
      pMem->m_pMat_Out->RowSigMoid() ;
    }else if(_acn == "Relu"){
      pMem->m_pMat_Out->RowRelu();
    }else if(_acn == "Tanh"){
      pMem->m_pMat_Out->RowTanh();
    }else if(_acn == "Softmax"){
      pMem->m_pMat_Out->RowSoftMax() ;
    }
    pMat_Out->CopyFrom(pMem->m_pMat_Out);
    
  }
  
  void Nnet_LinearTransform::PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out){
    
    pVec_Out = pMem->m_pVec_Out ;
    
    if (_svd) {
      
      Nnet_Component_Memory_LinearTransfrom* pMem_LT = (Nnet_Component_Memory_LinearTransfrom*)pMem ;
      
      pMem_LT->m_pVec_Out_Svd->Copy(_biases_svd);
      pMem_LT->m_pVec_Out_Svd->Add_aAX_NENO(_weights_svd_neon_t, pVec_In);
      
      pVec_Out->Copy(_biases);
      pVec_Out->Add_aAX_NENO(_weights_neon_t, pMem_LT->m_pVec_Out_Svd);
    }else{
      
      pVec_Out->Copy(_biases);
      pVec_Out->Add_aAX_NENO(_weights_neon_t, pVec_In);
    }
    
    if (_ln) {
      _ln_cell->PropagateFnc_Vec(pVec_Out);
    }
    
    if (_acn == "Sigmoid") {
      pVec_Out->RowSigMoid() ;
    }else if(_acn == "Relu"){
      pVec_Out->RowRelu();
    }else if(_acn == "Tanh"){
      pVec_Out->RowTanh();
    }else if(_acn == "Softmax"){
      pVec_Out->RowSoftMax();
    }
    
    return ;
  }
  
  Nnet_Component_Memory* Nnet_LinearTransform::GeneratePropagateMemory(int iBatchSize){
    
    Nnet_Component_Memory_LinearTransfrom* pMem = Z_SAFE_NEW(pMem, Nnet_Component_Memory_LinearTransfrom, this, iBatchSize);
    return pMem ;
  }
  
  /// Reads the component content
  void Nnet_LinearTransform::ReadData(ZInput* pInput){
    
    pInput->ReadBasicType(&_ln);
    pInput->ReadBasicType(&_svd);
    _acn = pInput->ReadToken() ;
    
    if (_svd) {
      pInput->ReadBasicType(&_svd_dim);
      
      _weights_svd_neon = Z_SAFE_NEW(_weights_svd_neon, ZMat_Neon, _input_dim, _svd_dim, false) ;
      _weights_svd_neon_t = Z_SAFE_NEW(_weights_svd_neon_t, ZMat_Neon, _input_dim, _svd_dim, true) ;
      ReadMatrixToNeon(pInput, _input_dim, _svd_dim, _weights_svd_neon, _weights_svd_neon_t) ;
      
      _biases_svd = Z_SAFE_NEW(_biases_svd, ZVec, _svd_dim);
      _biases_svd->Read(pInput);
      
      _biases_svd_neon = Z_SAFE_NEW(_biases_svd_neon, ZMat_Neon, 4, _svd_dim, false);
      _biases_svd_neon->RowCopy_4(_biases_svd);
      
      _weights_neon = Z_SAFE_NEW(_weights_neon, ZMat_Neon, _svd_dim, _output_dim, false) ;
      _weights_neon_t = Z_SAFE_NEW(_weights_neon_t, ZMat_Neon, _svd_dim, _output_dim, true) ;
      ReadMatrixToNeon(pInput, _svd_dim, _output_dim, _weights_neon, _weights_neon_t) ;
      
      
#ifdef __USE_MAT_NEON__
      _weights_svd_neon->Change2F16() ;
      _weights_neon->Change2F16() ;
#endif
    }else{
      _weights_neon = Z_SAFE_NEW(_weights_neon, ZMat_Neon, _input_dim, _output_dim, false) ;
      _weights_neon_t = Z_SAFE_NEW(_weights_neon_t, ZMat_Neon, _input_dim, _output_dim, true) ;
      ReadMatrixToNeon(pInput, _input_dim, _output_dim, _weights_neon, _weights_neon_t) ;
      
#ifdef __USE_MAT_NEON__
      _weights_neon->Change2F16() ;
#endif
    }
    
    if (_ln) {
      _ln_cell = Z_SAFE_NEW(_ln_cell, Nnet_LayerNorm, _output_dim);
      _ln_cell->ReadData(pInput);
      _biases = Z_SAFE_NEW(_biases, ZVec, _output_dim);
      _biases->Clean() ;
      
      _biases_neon = Z_SAFE_NEW(_biases_neon, ZMat_Neon, 4, _output_dim, false);
      _biases_neon->RowCopy_4(_biases) ;
      
    }else{
      _ln_cell = NULL ;
      _biases = Z_SAFE_NEW(_biases, ZVec, _output_dim);
      _biases->Read(pInput);
      
      _biases_neon = Z_SAFE_NEW(_biases_neon, ZMat_Neon, 4, _output_dim, false);
      _biases_neon->RowCopy_4(_biases) ;
    }
    

    
  }
  
  void Nnet_LinearTransform::ReadData_Old(ZInput* pInput, bool ln, bool svd, int svd_dim, std::string acn){
    
    _ln = ln ;
    _svd = svd ;
    _svd_dim = svd_dim ;
    _acn = acn ;
    
    if (_svd) {
      _weights_svd_neon_t = Z_SAFE_NEW(_weights_svd_neon_t, ZMat_Neon, _svd_dim, _input_dim, false) ;
      _weights_svd_neon = Z_SAFE_NEW(_weights_svd_neon, ZMat_Neon, _svd_dim, _input_dim, true) ;
      ReadMatrixToNeon(pInput, _svd_dim, _input_dim, _weights_svd_neon_t, _weights_svd_neon) ;
      
      _biases_svd = Z_SAFE_NEW(_biases_svd, ZVec, _svd_dim);
      _biases_svd->Read(pInput);
      
      _biases_svd_neon = Z_SAFE_NEW(_biases_svd_neon, ZMat_Neon, 4, _svd_dim, false);
      _biases_svd_neon->RowCopy_4(_biases_svd);
      
      _weights_neon_t = Z_SAFE_NEW(_weights_neon_t, ZMat_Neon, _output_dim, _svd_dim, false) ;
      _weights_neon = Z_SAFE_NEW(_weights_neon, ZMat_Neon, _output_dim, _svd_dim, true) ;
      ReadMatrixToNeon(pInput, _output_dim, _svd_dim, _weights_neon_t, _weights_neon) ;
    }else{
      
      _weights_neon_t = Z_SAFE_NEW(_weights_neon_t, ZMat_Neon, _output_dim, _input_dim, false) ;
      _weights_neon = Z_SAFE_NEW(_weights_neon, ZMat_Neon, _output_dim, _input_dim, true) ;
      ReadMatrixToNeon(pInput, _output_dim, _input_dim, _weights_neon_t, _weights_neon) ;
    }
    
    
    _ln_cell = NULL ;
    _biases = Z_SAFE_NEW(_biases, ZVec, _output_dim);
    _biases->Read(pInput);
    
    _biases_neon = Z_SAFE_NEW(_biases_neon, ZMat_Neon, 4, _output_dim, false);
    _biases_neon->RowCopy_4(_biases) ;
    
    
//    if (_svd) {
//      _weights_svd_neon->Print("_weights_svd_neon");
//      _biases_svd->Print("_biases_svd");
//    }
//    
//    _weights_neon->Print("_weights_neon");
//    _biases->Print("_biases") ;
    
    return ;
    
  }

  
};




