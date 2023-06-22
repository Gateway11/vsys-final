//
//  zcomp_affine.cpp
//  r2vt4
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zcomp_affine.h"

namespace __r2vt4__ {
  
  //Nnet_AffineTransform--------------------------------------------------------------
  Nnet_AffineTransform::Nnet_AffineTransform(int input_dim, int output_dim)
  :Nnet_Component(input_dim, output_dim, LayerName_AffineTransform){
    
    _weights_neon = NULL ;
    _weights_neon_t = NULL ;
    _biases = NULL ;
    _biases_neon = NULL ;
    

    
  }
  
  Nnet_AffineTransform::~Nnet_AffineTransform(){
    
    Z_SAFE_DEL(_weights_neon);
    Z_SAFE_DEL(_weights_neon_t);
    Z_SAFE_DEL(_biases);
    Z_SAFE_DEL(_biases_neon);
  }
  
  /// Forward pass transformation
  void Nnet_AffineTransform::PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Component_Memory* pMem, ZMat_Neon *&pMat_Out){
    
    //pMat_In->Print("pMat_In") ;
    
    pMem->ConfigMem(pMat_In);
    pMat_Out = pMem->m_pMat_Out_Neon ;
    
    pMat_Out->RowCopy_N(_biases_neon);
    
    //pMat_Out->Print("pMat_Out") ;
    
    pMat_Out->Add_aAB_NEON(pMat_In, _weights_neon);
    
    
  }
  
  void Nnet_AffineTransform::PropagateFnc_Vec(const ZVec* pVec_In, Nnet_Component_Memory* pMem, ZVec *&pVec_Out){
    
    pVec_Out= pMem->m_pVec_Out ;
    pVec_Out->Copy(_biases);
    pVec_Out->Add_aAX_NENO(_weights_neon_t, pVec_In);
  }
  
  /// Reads the component content
  void Nnet_AffineTransform::ReadData(ZInput* pInput){
    
    float learn_rate_coef_ = 0.0f, bias_learn_rate_coef_ = 0.0f, max_norm_ = 0.0f ;
    
    pInput->ExpectToken("<LearnRateCoef>");
    pInput->ReadBasicType(&learn_rate_coef_);
    
    pInput->ExpectToken("<BiasLearnRateCoef>");
    pInput->ReadBasicType(&bias_learn_rate_coef_);
    
    pInput->ExpectToken("<MaxNorm>");
    pInput->ReadBasicType(&max_norm_);
    
    _weights_neon_t = Z_SAFE_NEW(_weights_neon_t, ZMat_Neon, _output_dim, _input_dim, false) ;
    _weights_neon = Z_SAFE_NEW(_weights_neon, ZMat_Neon, _output_dim, _input_dim, true) ;
    
    ReadMatrixToNeon(pInput, _output_dim, _input_dim, _weights_neon_t, _weights_neon) ;
    
    _biases = Z_SAFE_NEW(_biases, ZVec, _output_dim) ;
    _biases_neon = Z_SAFE_NEW(_biases_neon, ZMat_Neon, 4, _output_dim, false) ;
    
    _biases->Read(pInput);
    _biases_neon->RowCopy_4(_biases) ;
    
  }
  
  void Nnet_AffineTransform::ReadData_Old(ZInput* pInput){
    
    _weights_neon_t = Z_SAFE_NEW(_weights_neon_t, ZMat_Neon, _output_dim, _input_dim, false) ;
    _weights_neon = Z_SAFE_NEW(_weights_neon, ZMat_Neon, _output_dim, _input_dim, true) ;
    
    ReadMatrixToNeon(pInput, _output_dim, _input_dim, _weights_neon_t, _weights_neon) ;
    
    _biases = Z_SAFE_NEW(_biases, ZVec, _output_dim) ;
    _biases_neon = Z_SAFE_NEW(_biases_neon, ZMat_Neon, 4, _output_dim, false) ;
    
    _biases->Read(pInput);
    _biases_neon->RowCopy_4(_biases) ;
    
    _weights_neon->Print("_weights_neon");
    _biases->Print("_biases") ;
    
  }
  
  /// Writes the component content
  void Nnet_AffineTransform::WriteData(ZOutput* pOutput) const{
    
    float learn_rate_coef_ = 0.0f, bias_learn_rate_coef_ = 0.0f, max_norm_ = 0.0f ;
    
    pOutput->WriteToken("<LearnRateCoef>");
    pOutput->WriteBasicType(&learn_rate_coef_);
    
    pOutput->WriteToken("<BiasLearnRateCoef>");
    pOutput->WriteBasicType(&bias_learn_rate_coef_);
    
    pOutput->WriteToken("<MaxNorm>");
    pOutput->WriteBasicType(&max_norm_);
    
    _weights_neon_t->Write(pOutput);
    
    _biases->Write(pOutput);
  }
  
    
};




