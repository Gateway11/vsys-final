//
//  zcomp.cpp
//  r2vt4
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zcomp.h"

namespace __r2vt4__ {
  
  Nnet_Component_Memory::Nnet_Component_Memory(Nnet_Component* pComponent, int iBatchSize){
    
    m_pComponent = pComponent ;
    m_iFrmNum_Total = iBatchSize ;
    
    m_pVec_Out = Z_SAFE_NEW(m_pVec_Out, ZVec, m_pComponent->_output_dim);
    m_pMat_Out_Neon = Z_SAFE_NEW(m_pMat_Out_Neon, ZMat_Neon, m_iFrmNum_Total, m_pComponent->_output_dim, false);
    m_pMat_Out = Z_SAFE_NEW(m_pMat_Out, ZMat, m_iFrmNum_Total, m_pComponent->_output_dim);
  }
  
  Nnet_Component_Memory::~Nnet_Component_Memory(){
    
    Z_SAFE_DEL(m_pVec_Out) ;
    Z_SAFE_DEL(m_pMat_Out) ;
    Z_SAFE_DEL(m_pMat_Out_Neon) ;
    m_pComponent = NULL ;
  }
  
  int Nnet_Component_Memory::ConfigMem(const ZMat_Neon* pMat_In){
    
    assert(!pMat_In->m_bTrans);
    
    int rt = 0 ;
    
    if (pMat_In->m_iRow > m_iFrmNum_Total) {
      m_iFrmNum_Total = pMat_In->m_iRow * 2 ;
      Z_SAFE_DEL(m_pMat_Out);
      
      m_pMat_Out_Neon = Z_SAFE_NEW(m_pMat_Out_Neon, ZMat_Neon, m_iFrmNum_Total, m_pComponent->_output_dim, false);
      m_pMat_Out = Z_SAFE_NEW(m_pMat_Out, ZMat, m_iFrmNum_Total, m_pComponent->_output_dim);
      
      rt = 1 ;
    }
    
    m_pMat_Out_Neon->ResetRow(pMat_In->m_iRow);
    m_pMat_Out->ResetRow(pMat_In->m_iRow);
    
    return rt ;
    
  }
  
  
  void Nnet_Component::ReadMatrixToNeon(ZInput* pInput, int row, int col, ZMat_Neon* pMatA, ZMat_Neon* pMatB){
    
    ZMat* pMat = Z_SAFE_NEW(pMat, ZMat, row, col);
    pMat->Read(pInput);
    if (pMatA != NULL) {
      pMatA->CopyFrom(pMat) ;
    }
    if (pMatB != NULL){
      pMatB->CopyFrom(pMat) ;
    }
    Z_SAFE_DEL(pMat);
    
  }
  
  
};




