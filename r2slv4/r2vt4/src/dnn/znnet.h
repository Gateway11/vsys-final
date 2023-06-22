//
//  znnet.h
//  r2vt4
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__znnet__
#define __r2vt4__znnet__

#include "../zmath.h"
#include "../mat/zmat.h"
#include "../mat/zmat_neon.h"
#include "../io/zinput.h"
#include "../io/zoutput.h"

#include "zcomp.h"
#include "zcomp_linear.h"
#include "zcomp_activation.h"
#include "zcomp_affine.h"
//#include "zcomp_blstm.h"
#include "zcomp_lstm.h"

namespace __r2vt4__ {
  
  class Nnet ;
  
  //Nnet_Memory_Batch--------------------------------------------------------------
  class Nnet_Memory_Batch{
  public:
    Nnet_Memory_Batch(Nnet* pNnet, int iBatchNum, int iSkipNum);
    virtual ~Nnet_Memory_Batch();
    
  public:
    int GetOutPut(ZMat* pMat_In, int iLen_In, ZMat* &pMat_Out, int &iLen_Out);
    int Reset();
    
  protected:
    int ProcessBatch();
    int ProcessSingle();
    
  public:
    //Dnn
    Nnet* m_pNnet ;
    std::vector<Nnet_Component_Memory*> memorys_ ;
    
    ZMat* m_pMat_Out ;
    
    int m_iTotalNum ;
    int m_iBatchNum;
    int m_iSkipNum;
    
    //Last
    int m_iLastNum ;
    ZVec* m_pLastScore ;
    
    //Batch
    ZMat* m_pMat_In_Batch ;
    ZMat_Neon* m_pMat_In_Batch_Neon ;
    ZMat* m_pMat_Out_Batch ;
    
    //Single
    ZVec* m_pVec_In_Single ;
    ZVec* m_pVec_Out_Single ;
    
  };
  
  
  //Nnet_Memory_Total--------------------------------------------------------------
  class Nnet_Memory_Total{
  public:
    Nnet_Memory_Total(Nnet* pNnet);
    virtual ~Nnet_Memory_Total();
    
  public:
    int GetOutPut(ZMat* pMat_In, int iLen_In, ZMat* &pMat_Out, int &iLen_Out);
    int Reset();
    
  public:
    
    int m_iLen_Total ;
    
    //In
    ZMat*  m_pMat_In ;
    ZMat_Neon* m_pMat_In_Neon ;
    
    //Out
    ZMat* m_pMat_Out ;
    
    Nnet* m_pNnet ;
    std::vector<Nnet_Component_Memory*> memorys_ ;
    
  };
  
  
  //Nnet--------------------------------------------------------------
  class Nnet {
  public:
    Nnet(const char* pNnetPath, bool bOldVersion);
    ~Nnet();
    
  public:
    
    /// Read the MLP from stream (can add layers to exisiting instance of Nnet)
    void Read(ZInput* pInput);
    
    
    void LoadOldVersion(const char* pNnetPath);
    void LoadNewVersion(const char* pNnetPath);
    
    void SetPrior(const float* prior);
    
    
    void ProcessPrior_Mat(ZMat* pMat) ;
    void ProcessPrior_Vec(ZVec* pVec) ;
    
  public:
    /// Vector which contains all the components composing the neural network,
    /// the components are for example: AffineTransform, Sigmoid, Softmax
    std::vector<Nnet_Component*> components_;
    int m_iInputDim ;
    int m_iOutputDim ;
    
    bool m_bPrior ;
    ZVec* m_pVec_Prior ;
    
  public:
    void LoadSn(const char* pSnPath);
    bool m_bSn ;
    ZVec* m_pVec_Sn_Mean ;
    ZVec* m_pVec_Sn_Var ;
  };
  
  
};



#endif /* __r2vt4__znnet__ */
