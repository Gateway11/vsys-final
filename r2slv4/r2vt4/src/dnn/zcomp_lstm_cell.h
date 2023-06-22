//
//  zcomp_lstm_cell.h
//  r2vt4
//
//  Created by hadoop on 6/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zcomp_lstm_cell__
#define __r2vt4__zcomp_lstm_cell__

#include "../zmath.h"
#include "../mat/zmat.h"
#include "../mat/zvec.h"
#include "zcomp_activation.h"
#include "../io/zinput.h"

namespace __r2vt4__ {
  
  class Nnet_Memory_Lstm_Cell ;
  
  class Nnet_Lstm_Cell{
    
  public:
    Nnet_Lstm_Cell(int input_dim, int output_dim, bool bFw);
    virtual ~Nnet_Lstm_Cell();
    
    void ReadData(ZInput* pInput);
    
  public:
    void PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Memory_Lstm_Cell* pMem, ZMat *&pMat_Out);
    
  public:
    bool _bFw ;
    
    int   _input_dim;
    int   _output_dim;
    int   _num_units;
    bool  _ln ;
    bool  _peephole ;
    float _forget_bias ;
    std::string _acn ;
    
    ZMat_Neon *_gifo_x_neon;
    ZMat_Neon *_gifo_m_neon;
    
    Nnet_LayerNorm *_ln_cell_i ;
    Nnet_LayerNorm *_ln_cell_j ;
    Nnet_LayerNorm *_ln_cell_f ;
    Nnet_LayerNorm *_ln_cell_c ;
    Nnet_LayerNorm *_ln_cell_o ;
    
    ZVec *_bias ;
    
    ZVec *_peephole_f ;
    ZVec *_peephole_i ;
    ZVec *_peephole_o ;
    
    ZMat_Neon *_proj_neon ;
    
  };
  
  class Nnet_Memory_Lstm_Cell{
  public:
    Nnet_Memory_Lstm_Cell(Nnet_Lstm_Cell* pLstmNode, int iBatchSize);
    virtual ~Nnet_Memory_Lstm_Cell();
    
  public:
    int ConfigMem(const ZMat_Neon* pMat_In);
    
  public:
    
    Nnet_Lstm_Cell* _lstm_node ;
    int _batch_size ;
    
    ZMat_Neon* _buf_gifo_neon ;
    
    ZMat* _buf_gifo_mat ;
    ZVec *_buf_gifo_vec;// cell_dim * 4
    
    ZVec *_buf_j;
    ZVec *_buf_i;
    ZVec *_buf_f;
    ZVec *_buf_o;
    
    ZVec *_buf_c;
    ZVec *_buf_m_bk;
    ZVec *_buf_m;
    
    ZMat* _buf_out ;
    
    
  };
  
};


#endif /* __r2vt4__zcomp_lstm_cell__ */
