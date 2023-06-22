//
//  znnet.cpp
//  r2vt4
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "znnet.h"

namespace __r2vt4__ {
  
  //Nnet_Memory_Batch--------------------------------------------------------------
  Nnet_Memory_Batch::Nnet_Memory_Batch(Nnet* pNnet, int iBatchNum, int iSkipNum){
    
    m_pNnet = pNnet ;
    m_iBatchNum = iBatchNum ;
    m_iSkipNum = iSkipNum ;
    m_iTotalNum = m_iBatchNum * m_iSkipNum ;
    
    m_pMat_Out = Z_SAFE_NEW(m_pMat_Out, ZMat, m_iTotalNum * 10 , m_pNnet->m_iOutputDim) ;
    
    //Last
    m_iLastNum = 0 ;
    m_pLastScore = Z_SAFE_NEW(m_pLastScore, ZVec, m_pNnet->m_iOutputDim) ;
    
    
    //Batch
    m_pMat_In_Batch = Z_SAFE_NEW(m_pMat_In_Batch, ZMat, m_iBatchNum, m_pNnet->m_iInputDim) ;
    m_pMat_In_Batch_Neon = Z_SAFE_NEW(m_pMat_In_Batch_Neon, ZMat_Neon, m_iBatchNum, m_pNnet->m_iInputDim, false) ;
    m_pMat_Out_Batch = Z_SAFE_NEW(m_pMat_Out_Batch, ZMat, m_iBatchNum, m_pNnet->m_iOutputDim) ;
    
    //Single
    m_pVec_In_Single = Z_SAFE_NEW(m_pVec_In_Single, ZVec, m_pNnet->m_iInputDim) ;
    m_pVec_Out_Single = Z_SAFE_NEW(m_pVec_Out_Single, ZVec, m_pNnet->m_iOutputDim) ;
    
    for (int i = 0 ; i < pNnet->components_.size() ; i ++) {
      Nnet_Component_Memory* pMem =  m_pNnet->components_[i]->GeneratePropagateMemory(m_iBatchNum);
      memorys_.push_back(pMem);
    }
    
  }
  
  Nnet_Memory_Batch::~Nnet_Memory_Batch(){
    
    for (int i = 0 ; i < memorys_.size() ; i ++) {
      Z_SAFE_DEL(memorys_[i]);
    }
    memorys_.clear() ;
    
    Z_SAFE_DEL(m_pMat_Out) ;
    
    Z_SAFE_DEL(m_pMat_In_Batch) ;
    Z_SAFE_DEL(m_pMat_In_Batch_Neon) ;
    Z_SAFE_DEL(m_pMat_Out_Batch) ;
    
    Z_SAFE_DEL(m_pVec_In_Single) ;
    Z_SAFE_DEL(m_pVec_Out_Single) ;
    
    Z_SAFE_DEL(m_pLastScore);
    
  }
  
  int Nnet_Memory_Batch::GetOutPut(ZMat* pMat_In, int iLen_In, ZMat* &pMat_Out, int &iLen_Out){
    
    assert(pMat_In->col == m_pNnet->m_iInputDim);
    assert(m_iLastNum < m_iSkipNum);
    
    if (m_pMat_Out->row < iLen_In) {
      Z_SAFE_DEL(m_pMat_Out);
      m_pMat_Out = Z_SAFE_NEW(m_pMat_Out, ZMat,iLen_In * 2, m_pNnet->m_iOutputDim);
    }
    
    //Process LastPos
    int iCur = 0 ;
    if (m_iLastNum != 0) {
      while (m_iLastNum < m_iSkipNum && iCur < iLen_In) {
        memcpy(m_pMat_Out->data[iCur], m_pLastScore->data, sizeof(float) * m_pNnet->m_iOutputDim) ;
        m_iLastNum ++ ;
        iCur ++ ;
      }
      
      if (m_iLastNum == m_iSkipNum) {
        m_iLastNum = 0 ;
      }
    }
    
    //Batch Num
    while (iCur + m_iTotalNum <= iLen_In) {
      
      for (int n = 0 ; n < m_iBatchNum; n ++) {
        memcpy(m_pMat_In_Batch->data[n], pMat_In->data[iCur+n*m_iSkipNum], sizeof(float) * m_pNnet->m_iInputDim);
      }
      
      ProcessBatch() ;
      
      for (int n = 0 ; n < m_iBatchNum ; n ++) {
        for (int j = 0 ; j < m_iSkipNum ; j ++) {
          memcpy(m_pMat_Out->data[iCur + n * m_iSkipNum + j], m_pMat_Out_Batch->data[n],
                 sizeof(float) * m_pNnet->m_iOutputDim);
        }
      }
      
      iCur += m_iTotalNum ;
    }
    
    //Single Num
    while (iCur + m_iSkipNum <= iLen_In) {
      
      memcpy(m_pVec_In_Single->data, pMat_In->data[iCur], sizeof(float) * m_pNnet->m_iInputDim);
      
      ProcessSingle() ;
      
      for (int j = 0 ; j < m_iSkipNum ; j ++) {
        memcpy(m_pMat_Out->data[iCur + j], m_pVec_Out_Single->data,
               sizeof(float) * m_pNnet->m_iOutputDim);
      }
      iCur += m_iSkipNum ;
    }
    
    //Last
    if (iCur < iLen_In) {
      
      m_iLastNum = iLen_In - iCur ;
      
      memcpy(m_pVec_In_Single->data, pMat_In->data[iCur], sizeof(float) * m_pNnet->m_iInputDim);
      
      ProcessSingle() ;
      
      for (int j = 0 ; j < m_iLastNum ; j ++) {
        memcpy(m_pMat_Out->data[iCur + j], m_pVec_Out_Single->data,
               sizeof(float) * m_pNnet->m_iOutputDim);
      }
      iCur += m_iLastNum ;
      
      m_pLastScore->Copy(m_pVec_Out_Single);
    }
    
    m_iLastNum = 0 ;
    
    assert(iCur == iLen_In) ;
    
    pMat_Out = m_pMat_Out ;
    iLen_Out = iLen_In ;
    
    return  0 ;
    
    
  }
  
  int Nnet_Memory_Batch::Reset(){
    
    m_iLastNum = 0 ;
    
    return 0 ;
    
  }
  
  int Nnet_Memory_Batch::ProcessBatch(){
    
    m_pMat_In_Batch_Neon->CopyFrom(m_pMat_In_Batch);
    
    ZMat_Neon* pMat_Out_Neon = m_pMat_In_Batch_Neon ;
    
    for (int i = 0 ; i < memorys_.size() ; i ++) {
      m_pNnet->components_[i]->PropagateFnc_Mat(pMat_Out_Neon, memorys_[i], pMat_Out_Neon) ;
    }
    
    pMat_Out_Neon->CopyTo(m_pMat_Out_Batch) ;
    
    m_pNnet->ProcessPrior_Mat(m_pMat_Out_Batch) ;
    
    return  0 ;
  }
  
  int Nnet_Memory_Batch::ProcessSingle(){
    
    ZVec* pVec_Out = m_pVec_In_Single ;
    
    for (int i = 0 ; i < memorys_.size() ; i ++) {
      m_pNnet->components_[i]->PropagateFnc_Vec(pVec_Out, memorys_[i], pVec_Out) ;
    }
    
    m_pVec_Out_Single->Copy(pVec_Out) ;
    
    m_pNnet->ProcessPrior_Vec(m_pVec_Out_Single) ;
    
    return  0 ;
  }
  
  //Nnet_Memory_Total--------------------------------------------------------------
  Nnet_Memory_Total::Nnet_Memory_Total(Nnet* pNnet){
    
    m_iLen_Total = 100 ;
    
    //Nnet
    m_pNnet = pNnet ;
    for (int i = 0 ; i < pNnet->components_.size() ; i ++) {
      Nnet_Component_Memory* pMem =  m_pNnet->components_[i]->GeneratePropagateMemory(m_iLen_Total);
      memorys_.push_back(pMem);
    }
    
    m_pMat_In = Z_SAFE_NEW(m_pMat_In, ZMat, m_iLen_Total, m_pNnet->m_iInputDim ) ;
    m_pMat_In_Neon = Z_SAFE_NEW(m_pMat_In_Neon, ZMat_Neon, m_iLen_Total, m_pNnet->m_iInputDim, false ) ;
    
    m_pMat_Out = Z_SAFE_NEW(m_pMat_Out, ZMat, m_iLen_Total, m_pNnet->m_iOutputDim) ;
    
  }
  
  Nnet_Memory_Total::~Nnet_Memory_Total(){
    
    for (int i = 0 ; i < memorys_.size() ; i ++) {
      Z_SAFE_DEL(memorys_[i]);
    }
    memorys_.clear() ;
    
    Z_SAFE_DEL(m_pMat_In) ;
    Z_SAFE_DEL(m_pMat_In_Neon) ;
    Z_SAFE_DEL(m_pMat_Out) ;
  }
  
  int Nnet_Memory_Total::GetOutPut(ZMat* pMat_In, int iLen_In, ZMat* &pMat_Out, int &iLen_Out){
    
    //Prepare buff
    if (iLen_In > m_iLen_Total) {
      Z_SAFE_DEL(m_pMat_In) ;
      Z_SAFE_DEL(m_pMat_In_Neon) ;
      Z_SAFE_DEL(m_pMat_Out) ;
      
      m_iLen_Total = iLen_In ;
      
      m_pMat_In = Z_SAFE_NEW(m_pMat_In, ZMat, m_iLen_Total, m_pNnet->m_iInputDim ) ;
      m_pMat_In_Neon = Z_SAFE_NEW(m_pMat_In_Neon, ZMat_Neon, m_iLen_Total, m_pNnet->m_iInputDim, false ) ;
      
      m_pMat_Out = Z_SAFE_NEW(m_pMat_Out, ZMat, m_iLen_Total, m_pNnet->m_iOutputDim) ;
    }
    
    //Prepare row
    m_pMat_In->ResetRow(iLen_In);
    m_pMat_In_Neon->ResetRow(iLen_In);
    m_pMat_Out->ResetRow(iLen_In);
    
    //Prepare Data
    for (int i = 0 ; i < iLen_In ; i ++) {
      memcpy(m_pMat_In->data[i], pMat_In->data[i], sizeof(float) * m_pNnet->m_iInputDim) ;
    }
    
    m_pMat_In_Neon->CopyFrom(m_pMat_In) ;
    
    //Propagate
    ZMat_Neon* pMat_Out_Neon = m_pMat_In_Neon ;
    
    for (int i = 0 ; i < memorys_.size() ; i ++) {
      m_pNnet->components_[i]->PropagateFnc_Mat(pMat_Out_Neon, memorys_[i], pMat_Out_Neon) ;
    }
    
    
    pMat_Out_Neon->CopyTo(m_pMat_Out) ;
    
    m_pMat_Out->RowSoftMax() ;
    
    m_pNnet->ProcessPrior_Mat(m_pMat_Out) ;
    
    pMat_Out = m_pMat_Out ;
    iLen_Out = iLen_In ;
    
    return  0 ;
  }
  
  int Nnet_Memory_Total::Reset(){
    
    return 0 ;
  }
  
  
  //Nnet--------------------------------------------------------------
  Nnet::Nnet(const char* pNnetPath, bool bOldVersion){
    
    m_iInputDim = 0 ;
    m_iOutputDim = 0 ;
    
    m_bPrior = false ;
    m_pVec_Prior = NULL ;
    
    m_bSn = false ;
    m_pVec_Sn_Mean = NULL ;
    m_pVec_Sn_Var = NULL ;
    
    if (bOldVersion) {
      LoadOldVersion(pNnetPath);
    }else{
      LoadNewVersion(pNnetPath);
    }
  }
  
  Nnet::~Nnet(){
    
    for (int i = 0 ; i < components_.size() ; i ++) {
      Z_SAFE_DEL(components_[i]);
    }
    components_.clear();
    
    Z_SAFE_DEL(m_pVec_Prior);
    Z_SAFE_DEL(m_pVec_Sn_Mean);
    Z_SAFE_DEL(m_pVec_Sn_Var);
    
  }
  
  /// Read the MLP from stream (can add layers to exisiting instance of Nnet)
  void Nnet::Read(ZInput* pInput){
    
    int dim_out, dim_in;
    pInput->ExpectToken("<Nnet>");
    
    std::string token = pInput->ReadToken() ;
    
    while (token != "</Nnet>") {
      
      pInput->ReadBasicType(&dim_out);
      pInput->ReadBasicType(&dim_in);
      
      Nnet_Component* pComponent = NULL ;
      if (token == LayerName_SoftMax) {
        pComponent = Z_SAFE_NEW(pComponent, Nnet_SoftMax, dim_in);
      }else if(token == LayerName_Sigmoid){
        pComponent = Z_SAFE_NEW(pComponent, Nnet_Sigmoid, dim_in);
      }else if(token == LayerName_Tanh){
        pComponent = Z_SAFE_NEW(pComponent, Nnet_Tanh, dim_in);
      }else if(token == LayerName_Relu){
        pComponent = Z_SAFE_NEW(pComponent, Nnet_Relu, dim_in);
      }else if(token == LayerName_LinearTransform){
        pComponent = Z_SAFE_NEW(pComponent, Nnet_LinearTransform, dim_in, dim_out);
      }else if(token == LayerName_AffineTransform){
        pComponent = Z_SAFE_NEW(pComponent, Nnet_AffineTransform, dim_in, dim_out);
      }else if(token == LayerName_BLSTM ){
        pComponent = Z_SAFE_NEW(pComponent, Nnet_BLstm, dim_in, dim_out);
      }else if(token == "<None>" ){
        pComponent = Z_SAFE_NEW(pComponent, Nnet_BLstm, dim_in, dim_out);
      }else if(token == LayerName_LSTM ){
        pComponent = Z_SAFE_NEW(pComponent, Nnet_Lstm, dim_in, dim_out);
      }else{
        assert(0);
      }
      pComponent->ReadData(pInput);
      
      token = pInput->ReadToken() ;
      if (token == "<!EndOfComponent>") {
        token = pInput->ReadToken() ;
      }
      
      components_.push_back(pComponent);
      
      if (token == "<CenterLoss>") {
        break ;
      }
    }
  }
  
  void Nnet::LoadOldVersion(const char* pNnetPath){
    
    ZFileInput* pInput = Z_SAFE_NEW(pInput, ZFileInput, pNnetPath, ZIN_RAW_FORMAT);
    
    //layNum
    int iLayerDimNum  = 0 ;
    pInput->ReadArray(&iLayerDimNum, 1);
    
    int* pLayerDimLst = Z_SAFE_NEW_AR1(pLayerDimLst, int, iLayerDimNum);
    pInput->ReadArray(pLayerDimLst, iLayerDimNum);
    
    int iLayerNum = iLayerDimNum - 1 ;
//    for (int i = 0 ; i < iLayerNum ; i ++) {
//      
//      Nnet_AffineTransform* pLayer = Z_SAFE_NEW(pLayer, Nnet_AffineTransform, pLayerDimLst[i], pLayerDimLst[i+1]);
//      pLayer->ReadData_Old(pInput) ;
//      components_.push_back(pLayer) ;
//      
//      if (i == iLayerNum - 1) {
//        Nnet_SoftMax* pAct = Z_SAFE_NEW(pAct, Nnet_SoftMax,pLayerDimLst[i+1] );
//        components_.push_back(pAct) ;
//      }else if(i < iLayerNum - 2){
//        Nnet_Sigmoid* pAct = Z_SAFE_NEW(pAct, Nnet_Sigmoid,pLayerDimLst[i+1] );
//        components_.push_back(pAct) ;
//      }
//    }
    for (int i = 0 ; i < iLayerNum - 1 ; i ++) {
      
      if (i < iLayerNum - 2) {
        Nnet_LinearTransform* pLayer = Z_SAFE_NEW(pLayer, Nnet_LinearTransform, pLayerDimLst[i], pLayerDimLst[i+1]);
        pLayer->ReadData_Old(pInput, false, false, 0, "Sigmoid");
        components_.push_back(pLayer) ;
        
      }else{
        Nnet_LinearTransform* pLayer = Z_SAFE_NEW(pLayer, Nnet_LinearTransform, pLayerDimLst[i], pLayerDimLst[i+2]);
        pLayer->ReadData_Old(pInput, false, true, pLayerDimLst[i+1], "Softmax");
        components_.push_back(pLayer) ;
      }
      
    }
    
    Z_SAFE_DEL_AR1(pLayerDimLst) ;
    
    m_iInputDim = components_[0]->_input_dim ;
    m_iOutputDim = components_[components_.size() - 1]->_output_dim ;
    
    
    m_bPrior = true ;
    m_pVec_Prior = Z_SAFE_NEW(m_pVec_Prior, ZVec, m_iOutputDim);
    m_pVec_Prior->Read(pInput);
    m_pVec_Prior->RowMul(-1.0f);
    
    
    Z_SAFE_DEL(pInput);
    
  }
  
  
  void Nnet::LoadNewVersion(const char* pNnetPath){
    
    ZFileInput* pInput = Z_SAFE_NEW(pInput, ZFileInput, pNnetPath, ZIN_KALDI_BINERY);
    
    Read(pInput);
    
    Z_SAFE_DEL(pInput);
    
    m_iInputDim = components_[0]->_input_dim ;
    m_iOutputDim = components_[components_.size() - 1]->_output_dim ;
    
    m_bPrior = false ;
    m_pVec_Prior = Z_SAFE_NEW(m_pVec_Prior, ZVec, m_iOutputDim);
    
  }
  
  void Nnet::SetPrior(const float* prior){
    
    m_bPrior = true ;
    memcpy(m_pVec_Prior->data, prior, sizeof(float) * m_iOutputDim) ;
    m_pVec_Prior->RowMul(-1.0f);
  }
  
  
  void Nnet::ProcessPrior_Mat(ZMat* pMat){
    
    if(m_bSn){
      //pMat->RowAdd(m_pVec_Sn_Mean);
      //pMat->RowMul(m_pVec_Sn_Var) ;
      
      
      for (int i = 0 ; i < pMat->row ; i ++) {
        double mean = 0 , var = 0 ;
        for (int j = 0 ; j < pMat->col ; j ++) {
          mean += pMat->data[i][j] ;
          var += pMat->data[i][j] * pMat->data[i][j] ;
        }
        mean = mean / pMat->col ;
        var = sqrt(var / pMat->col - mean * mean + 1e-5f) ;
        
        for (int j = 0 ; j < pMat->col ; j ++) {
          pMat->data[i][j] = (pMat->data[i][j] - mean) / var ;
        }
      }
      pMat->RowAdd(m_pVec_Sn_Mean);
      pMat->RowMul(m_pVec_Sn_Var) ;
      
    }
    
    if (m_bPrior) {
      pMat->RowAdd(m_pVec_Prior);
    }
    
    
  }
  
  void Nnet::ProcessPrior_Vec(ZVec* pVec){
    
    if(m_bSn){
      //pVec->RowAdd(m_pVec_Sn_Mean);
      //pVec->RowMul(m_pVec_Sn_Var) ;
      
      double mean = 0 , var = 0 ;
      for (int i = 0 ; i < pVec->size ; i ++) {
        mean += pVec->data[i] ;
        var += pVec->data[i] * pVec->data[i]  ;
      }
      mean = mean / pVec->size ;
      var = sqrt(var / pVec->size - mean * mean + 1e-5f) ;
      
      for (int i = 0 ; i < pVec->size ; i ++) {
        pVec->data[i] = (pVec->data[i] - mean) / var ;
      }
      pVec->RowAdd(m_pVec_Sn_Mean);
      pVec->RowMul(m_pVec_Sn_Var) ;
    }
    if (m_bPrior) {
      pVec->RowAdd(m_pVec_Prior) ;
//      pVec->RowSoftMax();
//      for (int i = 0 ; i < pVec->size ; i ++) {
//        pVec->data[i] = (pVec->data[i] + m_pVec_Prior->data[i]) ;
//      }
    }
    
  }
  
  
  void Nnet::LoadSn(const char* pSnPath){
    
    m_bSn = true ;
    
    ZInput* pInput = Z_SAFE_NEW(pInput, ZFileInput, pSnPath,  ZIN_RAW_FORMAT) ;
    
    int size = 0 ;
    
    m_pVec_Sn_Mean = Z_SAFE_NEW(m_pVec_Sn_Mean, ZVec, m_iOutputDim);
    m_pVec_Sn_Var = Z_SAFE_NEW(m_pVec_Sn_Var, ZVec, m_iOutputDim) ;
    
    pInput->ExpectToken("FV");
    pInput->ReadBasicType(&size);
    assert(size == m_iOutputDim);
    m_pVec_Sn_Mean->Read(pInput) ;
    
    
    pInput->ExpectToken("FV");
    pInput->ReadBasicType(&size);
    assert(size == m_iOutputDim);
    m_pVec_Sn_Var->Read(pInput) ;
    
    for (int i = 0 ; i < m_iOutputDim ; i ++) {
      m_pVec_Sn_Mean->data[i] = - m_pVec_Sn_Mean->data[i] ;
      m_pVec_Sn_Var->data[i] = 1.0f / m_pVec_Sn_Var->data[i] ;
      //m_pVec_Sn_Var->data[i] = 1.0f ;
    }
    
    Z_SAFE_DEL(pInput) ;
    
  }
};



