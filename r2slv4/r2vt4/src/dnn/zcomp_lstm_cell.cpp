//
//  zcomp_lstm_cell.cpp
//  r2vt4
//
//  Created by hadoop on 6/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zcomp_lstm_cell.h"

namespace __r2vt4__ {
  
  
  Nnet_Lstm_Cell::Nnet_Lstm_Cell(int input_dim, int output_dim, bool bFw){
    
    _bFw = bFw ;
    
    _input_dim = input_dim ;
    _output_dim = output_dim ;
    
    _num_units = 0 ;
    _ln = false ;
    _peephole = false ;
    _forget_bias = 0.0f ;
    _acn = "" ;
    
    _gifo_m_neon = NULL ;
    _gifo_x_neon = NULL ;
    
    _ln_cell_i = NULL ;
    _ln_cell_j = NULL ;
    _ln_cell_f = NULL ;
    _ln_cell_c = NULL ;
    _ln_cell_o = NULL ;
    
    _bias = NULL ;
    
    _peephole_f = NULL ;
    _peephole_i = NULL ;
    _peephole_o = NULL ;
    
    _proj_neon = NULL ;
    
    
  }
  
  
  Nnet_Lstm_Cell::~Nnet_Lstm_Cell(){
    
    Z_SAFE_DEL(_gifo_m_neon);
    Z_SAFE_DEL(_gifo_x_neon);
    
    Z_SAFE_DEL(_ln_cell_i);
    Z_SAFE_DEL(_ln_cell_j);
    Z_SAFE_DEL(_ln_cell_f);
    Z_SAFE_DEL(_ln_cell_c);
    Z_SAFE_DEL(_ln_cell_o);
    
    Z_SAFE_DEL(_bias);
    
    Z_SAFE_DEL(_peephole_f);
    Z_SAFE_DEL(_peephole_i);
    Z_SAFE_DEL(_peephole_o);
    
    Z_SAFE_DEL(_proj_neon);
    
  }
  
  void Nnet_Lstm_Cell::ReadData(ZInput* pInput){
    
    pInput->ReadBasicType(&_input_dim) ;
    pInput->ReadBasicType(&_output_dim) ;
    pInput->ReadBasicType(&_num_units) ;
    pInput->ReadBasicType(&_ln) ;
    pInput->ReadBasicType(&_peephole) ;
    pInput->ReadBasicType(&_forget_bias) ;
    
    _acn =  pInput->ReadToken() ;
    
    _gifo_x_neon = Z_SAFE_NEW(_gifo_x_neon, ZMat_Neon, _input_dim, _num_units * 4, false) ;
    _gifo_x_neon->Read(pInput) ;
    _gifo_m_neon = Z_SAFE_NEW(_gifo_m_neon, ZMat_Neon, _output_dim, _num_units * 4, true) ;
    _gifo_m_neon->Read(pInput) ;
    
    
    if (_ln) {
      _ln_cell_i = Z_SAFE_NEW(_ln_cell_i, Nnet_LayerNorm, _num_units) ;
      _ln_cell_i->ReadData(pInput);
      _ln_cell_j = Z_SAFE_NEW(_ln_cell_j, Nnet_LayerNorm, _num_units) ;
      _ln_cell_j->ReadData(pInput);
      _ln_cell_f = Z_SAFE_NEW(_ln_cell_f, Nnet_LayerNorm, _num_units) ;
      _ln_cell_f->ReadData(pInput);
      _ln_cell_c = Z_SAFE_NEW(_ln_cell_c, Nnet_LayerNorm, _num_units) ;
      _ln_cell_c->ReadData(pInput);
      _ln_cell_o = Z_SAFE_NEW(_ln_cell_o, Nnet_LayerNorm, _num_units) ;
      _ln_cell_o->ReadData(pInput);
    }else{
      _bias = Z_SAFE_NEW(_bias, ZVec, _num_units * 4) ;
      _bias->Read(pInput);
    }
    
    if (_peephole) {
      _peephole_f = Z_SAFE_NEW(_peephole_f, ZVec, _num_units) ;
      _peephole_f->Read(pInput);
      _peephole_i = Z_SAFE_NEW(_peephole_i, ZVec, _num_units) ;
      _peephole_i->Read(pInput);
      _peephole_o = Z_SAFE_NEW(_peephole_o, ZVec, _num_units) ;
      _peephole_o->Read(pInput);
    }
    
    if (_num_units != _output_dim) {
      _proj_neon = Z_SAFE_NEW(_proj_neon, ZMat_Neon, _num_units, _output_dim, true) ;
      _proj_neon->Read(pInput);
    }
  }
  
  void Nnet_Lstm_Cell::PropagateFnc_Mat(ZMat_Neon* pMat_In, Nnet_Memory_Lstm_Cell* pMem, ZMat *&pMat_Out){
    
    pMem->ConfigMem(pMat_In);
    
    pMem->_buf_gifo_neon->Clean() ;
    pMem->_buf_c->Clean() ;
    pMem->_buf_m->Clean() ;
    
    
    pMem->_buf_gifo_neon->Add_aAB_NEON(pMat_In, _gifo_x_neon) ;
    pMem->_buf_gifo_neon->CopyTo(pMem->_buf_gifo_mat) ;
    
    for (int i = 0 ; i < pMat_In->m_iRow ; i ++) {
      
      if (_bFw) {
        memcpy(pMem->_buf_gifo_vec->data, pMem->_buf_gifo_mat->data[i], sizeof(float) * _num_units * 4);
      }else{
        memcpy(pMem->_buf_gifo_vec->data, pMem->_buf_gifo_mat->data[pMat_In->m_iRow - 1 - i], sizeof(float) * _num_units * 4);
      }
      
      if (!_ln) {
        pMem->_buf_gifo_vec->RowAdd(_bias) ;
      }
      
      pMem->_buf_gifo_vec->Add_aAX_NENO(_gifo_m_neon, pMem->_buf_m) ;
      
      int cell_pos = 0 , cell_len = _num_units * sizeof(float) ;
      memcpy(pMem->_buf_j->data, pMem->_buf_gifo_vec->data, cell_len);
      cell_pos += _num_units ;
      memcpy(pMem->_buf_i->data, pMem->_buf_gifo_vec->data + cell_pos, cell_len);
      cell_pos += _num_units ;
      memcpy(pMem->_buf_f->data, pMem->_buf_gifo_vec->data + cell_pos, cell_len);
      cell_pos += _num_units ;
      memcpy(pMem->_buf_o->data, pMem->_buf_gifo_vec->data + cell_pos, cell_len);
      
      
      if (_ln) {
        if (_peephole) {
          pMem->_buf_f->Add_aXY(_peephole_f, pMem->_buf_c);
          pMem->_buf_i->Add_aXY(_peephole_i, pMem->_buf_c);
          
          _ln_cell_i->PropagateFnc_Vec(pMem->_buf_i);
          _ln_cell_j->PropagateFnc_Vec(pMem->_buf_j);
          _ln_cell_f->PropagateFnc_Vec(pMem->_buf_f);
          
          pMem->_buf_f->RowSigMoid() ;
          pMem->_buf_c->RowMul(pMem->_buf_f) ;
          
          pMem->_buf_i->RowSigMoid() ;
          pMem->_buf_j->RowActive(_acn);
          
          pMem->_buf_c->Add_aXY(pMem->_buf_i, pMem->_buf_j);
          pMem->_buf_o->Add_aXY(_peephole_o, pMem->_buf_c);
          
          pMem->_buf_m_bk->Copy(pMem->_buf_c);
          _ln_cell_c->PropagateFnc_Vec(pMem->_buf_m_bk);
          pMem->_buf_m_bk->RowActive(_acn);
          
          _ln_cell_o->PropagateFnc_Vec(pMem->_buf_o);
          pMem->_buf_o->RowSigMoid() ;
          
          pMem->_buf_m_bk->RowMul(pMem->_buf_o);
          
          if (_num_units == _output_dim) {
            pMem->_buf_m->Copy(pMem->_buf_m_bk) ;
          }else{
            pMem->_buf_m->Clean() ;
            pMem->_buf_m->Add_aAX_NENO(_proj_neon, pMem->_buf_m_bk) ;
          }
          
        }else{
          
          _ln_cell_i->PropagateFnc_Vec(pMem->_buf_i);
          _ln_cell_j->PropagateFnc_Vec(pMem->_buf_j);
          _ln_cell_f->PropagateFnc_Vec(pMem->_buf_f);
          
          pMem->_buf_f->RowSigMoid() ;
          pMem->_buf_c->RowMul(pMem->_buf_f) ;
          
          pMem->_buf_i->RowSigMoid() ;
          pMem->_buf_j->RowActive(_acn);
          pMem->_buf_c->Add_aXY(pMem->_buf_i, pMem->_buf_j);
          
          pMem->_buf_m_bk->Copy(pMem->_buf_c);
          _ln_cell_c->PropagateFnc_Vec(pMem->_buf_m_bk);
          pMem->_buf_m_bk->RowActive(_acn);
          
          _ln_cell_o->PropagateFnc_Vec(pMem->_buf_o);
          pMem->_buf_o->RowSigMoid() ;
          pMem->_buf_m_bk->RowMul(pMem->_buf_o);
          
          if (_num_units == _output_dim) {
            pMem->_buf_m->Copy(pMem->_buf_m_bk) ;
          }else{
            pMem->_buf_m->Clean() ;
            pMem->_buf_m->Add_aAX_NENO(_proj_neon, pMem->_buf_m_bk) ;
          }
          
        }
      }else{
        if (_peephole) {
          pMem->_buf_f->Add(_forget_bias);
          pMem->_buf_f->Add_aXY(_peephole_f, pMem->_buf_c);
          pMem->_buf_f->RowSigMoid();
          
          pMem->_buf_i->Add_aXY(_peephole_i, pMem->_buf_c);
          pMem->_buf_i->RowSigMoid();
          
          pMem->_buf_j->RowActive(_acn);
          
          pMem->_buf_c->RowMul(pMem->_buf_f);
          pMem->_buf_c->Add_aXY(pMem->_buf_i, pMem->_buf_j);
          
          pMem->_buf_o->Add_aXY(_peephole_o, pMem->_buf_c);
          pMem->_buf_o->RowSigMoid();
          
          pMem->_buf_m_bk->Copy(pMem->_buf_c);
          pMem->_buf_m_bk->RowActive(_acn);
          pMem->_buf_m_bk->RowMul(pMem->_buf_o);
          
          if (_num_units == _output_dim) {
            pMem->_buf_m->Copy(pMem->_buf_m_bk) ;
          }else{
            pMem->_buf_m->Clean() ;
            pMem->_buf_m->Add_aAX_NENO(_proj_neon, pMem->_buf_m_bk) ;
          }
          
        }else{
          
          pMem->_buf_f->Add(_forget_bias);
          pMem->_buf_f->RowSigMoid();
          
          pMem->_buf_i->RowSigMoid();
          pMem->_buf_j->RowActive(_acn);
          
          pMem->_buf_c->RowMul(pMem->_buf_f);
          pMem->_buf_c->Add_aXY(pMem->_buf_i, pMem->_buf_j);
          
          pMem->_buf_o->RowSigMoid();
          
          pMem->_buf_m_bk->Copy(pMem->_buf_c);
          pMem->_buf_m_bk->RowActive(_acn);
          pMem->_buf_m_bk->RowMul(pMem->_buf_o);
          
          if (_num_units == _output_dim) {
            pMem->_buf_m->Copy(pMem->_buf_m_bk) ;
          }else{
            pMem->_buf_m->Clean() ;
            pMem->_buf_m->Add_aAX_NENO(_proj_neon, pMem->_buf_m_bk) ;
          }
          
        }
      }
      
      if (_bFw) {
        memcpy(pMem->_buf_out->data[i], pMem->_buf_m->data, sizeof(float) * _output_dim);
      }else{
        memcpy(pMem->_buf_out->data[pMat_In->m_iRow - 1 - i], pMem->_buf_m->data, sizeof(float) * _output_dim);
      }
    }
    
    pMat_Out = pMem->_buf_out ;
    
    return ;
    
  }
  
  
  Nnet_Memory_Lstm_Cell::Nnet_Memory_Lstm_Cell(Nnet_Lstm_Cell* pLstmNode, int iBatchSize){
    
    _lstm_node = pLstmNode ;
    _batch_size = iBatchSize ;
    
    _buf_gifo_neon = Z_SAFE_NEW(_buf_gifo_neon, ZMat_Neon, _batch_size, _lstm_node->_num_units * 4, false) ;
    
    _buf_gifo_mat = Z_SAFE_NEW(_buf_gifo_mat, ZMat, _batch_size, _lstm_node->_num_units * 4);
    _buf_gifo_vec = Z_SAFE_NEW(_buf_gifo_vec, ZVec, _lstm_node->_num_units * 4);
    
    _buf_j = Z_SAFE_NEW(_buf_j, ZVec, _lstm_node->_num_units) ;
    _buf_i = Z_SAFE_NEW(_buf_i, ZVec, _lstm_node->_num_units) ;
    _buf_f = Z_SAFE_NEW(_buf_f, ZVec, _lstm_node->_num_units) ;
    _buf_o = Z_SAFE_NEW(_buf_o, ZVec, _lstm_node->_num_units) ;
    
    _buf_c = Z_SAFE_NEW(_buf_c, ZVec, _lstm_node->_num_units) ;
    _buf_m_bk = Z_SAFE_NEW(_buf_m_bk, ZVec, _lstm_node->_num_units) ;
    _buf_m = Z_SAFE_NEW(_buf_m, ZVec, _lstm_node->_output_dim) ;
    
    _buf_out = Z_SAFE_NEW(_buf_out, ZMat, _batch_size, _lstm_node->_output_dim) ;
    
  }
  
  Nnet_Memory_Lstm_Cell::~Nnet_Memory_Lstm_Cell(){
    
    Z_SAFE_DEL(_buf_gifo_neon);
    
    Z_SAFE_DEL(_buf_gifo_mat);
    Z_SAFE_DEL(_buf_gifo_vec);
    
    Z_SAFE_DEL(_buf_j);
    Z_SAFE_DEL(_buf_i);
    Z_SAFE_DEL(_buf_f);
    Z_SAFE_DEL(_buf_o);
    
    Z_SAFE_DEL(_buf_c);
    Z_SAFE_DEL(_buf_m);
    Z_SAFE_DEL(_buf_m_bk);
    
    Z_SAFE_DEL(_buf_out);
    
  }
  
  int Nnet_Memory_Lstm_Cell::ConfigMem(const ZMat_Neon* pMat_In){
    
    if (_batch_size < pMat_In->m_iRow) {
      
      Z_SAFE_DEL(_buf_gifo_neon);
      Z_SAFE_DEL(_buf_gifo_mat);
      Z_SAFE_DEL(_buf_out);
      
      _batch_size = pMat_In->m_iRow * 2 ;
      
      _buf_gifo_neon = Z_SAFE_NEW(_buf_gifo_neon, ZMat_Neon, _batch_size, _lstm_node->_num_units * 4, false) ;
      
      _buf_gifo_mat = Z_SAFE_NEW(_buf_gifo_mat, ZMat, _batch_size, _lstm_node->_num_units * 4);
      _buf_out = Z_SAFE_NEW(_buf_out, ZMat, _batch_size, _lstm_node->_output_dim) ;
      
    }

    _buf_gifo_neon->ResetRow(pMat_In->m_iRow) ;
    _buf_gifo_mat->ResetRow(pMat_In->m_iRow) ;
    _buf_out->ResetRow(pMat_In->m_iRow) ;
    
    _buf_gifo_neon->Clean() ;
    _buf_c->Clean() ;
    _buf_m->Clean() ;
    
    return  1 ;
    
  }
  
  
};




