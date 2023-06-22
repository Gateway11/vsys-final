//
//  main.cpp
//  testmat
//
//  Created by hadoop on 3/30/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include <iostream>
#include "../../r2vt4/src/mat/zmat.h"
#include "../../r2vt4/src/dnn/znnet.h"
#include <sys/time.h>
#include <time.h>

using  namespace __r2vt4__ ;

// 返回自系统开机以来的毫秒数（tick）
unsigned long GetMyTickCount()
{
  
  struct timeval tv ;
  gettimeofday(&tv, NULL) ;
  
  return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}



//int TestVec_Ax(){
//
//  ZVec* pVecA = Z_SAFE_NEW(pVecA,ZVec,5);
//  ZVec* pVecB = Z_SAFE_NEW(pVecB,ZVec,6);
//
//  ZMat* pMatA = Z_SAFE_NEW(pMatA,ZMat,6,5);
//  ZMat* pMatAT = Z_SAFE_NEW(pMatAT,ZMat,5,6);
//
//  for (int i = 0 ; i < pVecA->size ; i ++) {
//    pVecA->data[i] = i ;
//  }
//
//  for (int i = 0 ; i < pMatA->row ; i ++) {
//    for (int j = 0 ; j < pMatA->col ; j ++) {
//      pMatA->data[i][j] = i  + j ;
//      pMatAT->data[j][i] = pMatA->data[i][j];
//    }
//  }
//
//  pVecB->Clean() ;
//  pVecB->Add_aAX(pMatA, pVecA);
//  pVecB->Print("AX");
//
//  pVecB->Clean() ;
//  pVecB->Add_aATX(pMatAT, pVecA);
//  pVecB->Print("ATX");
//
//  Z_SAFE_DEL(pVecA);
//  Z_SAFE_DEL(pVecB);
//  Z_SAFE_DEL(pMatA);
//  Z_SAFE_DEL(pMatAT);
//
//  return  0 ;
//}
//
//int TestMat_AB(){
//
//  int a = 128 ;
//  int b = 96 ;
//  int c = 64 ;
//
//  ZMat* pMatA = Z_SAFE_NEW(pMatA,ZMat,a,b);
//  ZMat* pMatAT = Z_SAFE_NEW(pMatAT,ZMat,b,a);
//  ZMat* pMatB = Z_SAFE_NEW(pMatB,ZMat,b,c);
//  ZMat* pMatBT = Z_SAFE_NEW(pMatBT,ZMat,c,b);
//  ZMat* pMatC = Z_SAFE_NEW(pMatC,ZMat,a,c);
//
//  for (int i = 0 ; i < a ; i ++) {
//    for (int j = 0 ; j < b ; j ++ ) {
//      pMatA->data[i][j] = (i * 3 + j)/10.0f ;
//      pMatAT->data[j][i] = pMatA->data[i][j];
//    }
//  }
//
//  for (int i = 0 ; i < b ; i ++) {
//    for (int j = 0 ; j < c ; j ++ ) {
//      pMatB->data[i][j] = (i * 4 + j)/10.0f ;
//      pMatBT->data[j][i] = pMatB->data[i][j];
//    }
//  }
//
////  timeval tv1,tv2 ;
////  gettimeofday(&tv1, NULL);
////  for (int i = 0 ; i < 10; i ++) {
////    pMatC->Clean() ;
////    pMatC->Add_aAB(pMatA, pMatB);
////    printf("%d\n",i);
////  }
////  gettimeofday(&tv2, NULL);
////  printf("%d.%06d  %d.%06d\n",tv1.tv_sec,tv1.tv_usec,tv2.tv_sec,tv2.tv_usec);
//
//  pMatC->Clean() ;
//  pMatC->Add_aAB(pMatA, pMatB);
//  pMatC->Print("AB");
//
//  pMatC->Clean() ;
//  pMatC->Add_aATB(pMatAT, pMatB);
//  pMatC->Print("ATB");
//
//  pMatC->Clean() ;
//  pMatC->Add_aABT(pMatA, pMatBT);
//  pMatC->Print("ABT");
//
//  //    pMatC->Clean() ;
//  //    pMatC->Add_aAB(pMatA, pMatB);
//  //    pMatC->Print("AB");
//
//  //    pMatC->Clean() ;
//  //    pMatC->Add_aATB(pMatAT, pMatB);
//  //    pMatC->Print("ATB");
//  //
//  //    pMatC->Clean() ;
//  //    pMatC->Add_aABT(pMatA, pMatBT);
//  //    pMatC->Print("ABT");
//
//  Z_SAFE_DEL(pMatA);
//  Z_SAFE_DEL(pMatAT);
//  Z_SAFE_DEL(pMatB);
//  Z_SAFE_DEL(pMatBT);
//  Z_SAFE_DEL(pMatC);
//
//  return  0 ;
//
//}
//
//
//int TestCMat_AB(){
//
//  int a = 5 ;
//  int b = 6 ;
//  int c = 7 ;
//
//  ZCMat* pMatA = Z_SAFE_NEW(pMatA,ZCMat,a,b);
//  ZCMat* pMatAT = Z_SAFE_NEW(pMatAT,ZCMat,b,a);
//  ZCMat* pMatB = Z_SAFE_NEW(pMatB,ZCMat,b,c);
//  ZCMat* pMatBT = Z_SAFE_NEW(pMatBT,ZCMat,c,b);
//  ZCMat* pMatC = Z_SAFE_NEW(pMatC,ZCMat,a,c);
//
//  for (int i = 0 ; i < a ; i ++) {
//    for (int j = 0 ; j < b ; j ++ ) {
//      pMatA->data[i][j] = i * 3 + j + 3.0iF ;
//      pMatAT->data[j][i] = pMatA->data[i][j];
//    }
//  }
//
//  for (int i = 0 ; i < b ; i ++) {
//    for (int j = 0 ; j < c ; j ++ ) {
//      pMatB->data[i][j] = i * 4 + j - 6.0iF ;
//      pMatBT->data[j][i] = pMatB->data[i][j];
//    }
//  }
//
//  pMatC->Clean() ;
//  pMatC->Add_aAB(pMatA, pMatB);
//  pMatC->Print("AB");
//
//  pMatC->Clean() ;
//  pMatC->Add_aATB(pMatAT, pMatB);
//  pMatC->Print("ATB");
//
//  pMatC->Clean() ;
//  pMatC->Add_aABT(pMatA, pMatBT);
//  pMatC->Print("ABT");
//
//  Z_SAFE_DEL(pMatA);
//  Z_SAFE_DEL(pMatAT);
//  Z_SAFE_DEL(pMatB);
//  Z_SAFE_DEL(pMatBT);
//  Z_SAFE_DEL(pMatC);
//
//  return  0 ;
//
//}
//
//int TestNeonMat_AB(){
//
//  int a = 4 ;
//  int b = 4 ;
//  int c = 4 ;
//
//  ZMat* pMatA = Z_SAFE_NEW(pMatA,ZMat,a,b);
//  ZMat* pMatB = Z_SAFE_NEW(pMatB,ZMat,b,c);
//  ZMat* pMatC = Z_SAFE_NEW(pMatC,ZMat,a,c);
//  ZMat* pMatC_Tmp = Z_SAFE_NEW(pMatC_Tmp,ZMat,a,c);
//
//  for (int i = 0 ; i < a ; i ++) {
//    for (int j = 0 ; j < b ; j ++ ) {
//      pMatA->data[i][j] = (i * 3 + j)/10.0f ;
//    }
//  }
//
//  for (int i = 0 ; i < b ; i ++) {
//    for (int j = 0 ; j < c ; j ++ ) {
//      pMatB->data[i][j] = (i * 4 + j)/10.0f ;
//    }
//  }
//
//  pMatC->Clean() ;
//  unsigned long NEON_START = GetMyTickCount() ;
//  for(int i = 0 ; i < 100000 ; i ++){
//    NEON_Matrix4Mul(pMatA->data[0],pMatB->data[0],pMatC_Tmp->data[0]);
//    //NEON_Matrix4Mul2(pMatA->data[0],pMatA->data[1],pMatA->data[2],pMatA->data[3],
//    //                 pMatB->data[0],pMatB->data[1],pMatB->data[2],pMatB->data[3],
//    //                 pMatC_Tmp->data[0],pMatC_Tmp->data[1],pMatC_Tmp->data[2],pMatC_Tmp->data[3]);
//    NEON_Matrix4Add(pMatC_Tmp->data[0], pMatC->data[0], pMatC->data[0]);
//  }
//  unsigned long NEON_END = GetMyTickCount() ;
//  pMatC->Print("NEON_AB");
//
//  pMatC->Clean() ;
//  unsigned long BLIS_START = GetMyTickCount() ;
//  for(int i = 0 ; i < 100000 ; i ++){
//    pMatC->Add_aAB(pMatA, pMatB);
//  }
//  unsigned long BLIS_END = GetMyTickCount() ;
//  pMatC->Print("NEON_AB");
//
//  printf("NEON TIME: %ld\n", NEON_END - NEON_START);
//  printf("BLIS TIME: %ld\n", BLIS_END - BLIS_START);
//
//
//  Z_SAFE_DEL(pMatA);
//  Z_SAFE_DEL(pMatB);
//  Z_SAFE_DEL(pMatC);
//
//  return  0 ;
//
//
//}


int TestMat_Dnn(const char* pDnnPath){
  
  int iBatchSize = 4 ;
  int iSkipSize = 3 ;
  int iInNum = iBatchSize * iSkipSize ;
  int iFeaNum = 1000 ;
  int iRepeatNum = 1000 ;
  
  ZMat* pOutFea = NULL ;
  int iOutNum = 0 ;
  
  Nnet* pNnet = Z_SAFE_NEW(pNnet, Nnet, pDnnPath, false) ;
  Nnet_Memory_Batch* pNnetMem = Z_SAFE_NEW(pNnetMem, Nnet_Memory_Batch, pNnet, iBatchSize, iSkipSize) ;
  
  ZMat** pFeaLst = Z_SAFE_NEW_AR1(pFeaLst, ZMat*, iFeaNum) ;
  for (int i = 0 ; i < iFeaNum ; i ++) {
    pFeaLst[i] = Z_SAFE_NEW(pFeaLst[i], ZMat, iInNum, pNnet->m_iInputDim);
    pFeaLst[i]->RandInit() ;
  }
  
  printf("Init Ok\n");
  unsigned long Dnn_START = GetMyTickCount() ;
  for (int i = 0 ; i < iFeaNum ; i ++) {
    for (int j = 0 ; j < iRepeatNum ; j ++) {
      pNnetMem->GetOutPut(pFeaLst[i], iInNum, pOutFea, iOutNum) ;
    }
    
  }
  unsigned long Dnn_END = GetMyTickCount() ;
  
  printf("Total Cost: %ldms\n", Dnn_END - Dnn_START);
  printf("For Each batch Dnn TIME: %fms\n", (Dnn_END - Dnn_START ) * 1.0f / (iFeaNum * iInNum * iRepeatNum ));
  
  Z_SAFE_DEL(pNnet);
  Z_SAFE_DEL(pNnetMem) ;
  
  
  return  0 ;
  
}


int TestMat_Dnn_Single(const char* pDnnPath){
  
  int iBatchSize = 4 ;
  int iSkipSize = 1 ;
  int iInNum = iBatchSize * iSkipSize ;
  
  ZMat* pOutFea = NULL ;
  int iOutNum = 0 ;
  
  Nnet* pNnet = Z_SAFE_NEW(pNnet, Nnet, pDnnPath, false) ;
  Nnet_Memory_Batch* pNnetMem = Z_SAFE_NEW(pNnetMem, Nnet_Memory_Batch, pNnet, iBatchSize, 1) ;
  
  ZMat* pFea = Z_SAFE_NEW(pFea, ZMat, iInNum, pNnet->m_iInputDim);
  pFea->FixInit() ;
  //pFea->Print("xx") ;
  
  pNnetMem->GetOutPut(pFea, iInNum, pOutFea, iOutNum) ;
  
  pOutFea->row = iOutNum ;
  
  //pOutFea->Print("xx");
  
  
  Z_SAFE_DEL(pNnet);
  Z_SAFE_DEL(pNnetMem) ;
  
  return  0 ;
  
}

int TestMat_Dnn_Total(const char* pDnnPath){
  
  int iInNum = 50 ;
  
  ZMat* pOutFea = NULL ;
  int iOutNum = 0 ;
  int feanum = 1000 ;
  
  Nnet* pNnet = Z_SAFE_NEW(pNnet, Nnet, pDnnPath, false) ;
  Nnet_Memory_Total* pNnetMem = Z_SAFE_NEW(pNnetMem, Nnet_Memory_Total, pNnet) ;
  
  ZMat** pFeaLst = Z_SAFE_NEW_AR1(pFeaLst, ZMat*, feanum) ;
  for (int i = 0 ; i < feanum ; i ++) {
    pFeaLst[i] = Z_SAFE_NEW(pFeaLst[i], ZMat, iInNum, pNnet->m_iInputDim);
    pFeaLst[i]->RandInit() ;
  }
  printf("Init Ok\n");
  unsigned long Dnn_START = GetMyTickCount() ;
  for (int i = 0 ; i < feanum ; i ++) {
    pNnetMem->GetOutPut(pFeaLst[i], iInNum, pOutFea, iOutNum) ;
  }
  unsigned long Dnn_END = GetMyTickCount() ;
  
  printf("Total Cost: %ldms\n", Dnn_END - Dnn_START);
  printf("For Each batch Dnn TIME: %fms\n", (Dnn_END - Dnn_START ) * 1.0f / (feanum * iInNum));
  
  
  Z_SAFE_DEL(pNnet);
  Z_SAFE_DEL(pNnetMem) ;
  
  return  0 ;
  
}

int TestCnn(){
  
  int _input_dim_freq = 40 ;
  int _input_dim_time = 1000 ;
  int _input_channels = 64 ;
  
  int _filter_size_freq = 3 ;
  int _filter_size_time = 3 ;
  
  int _output_stride_freq = 1 ;
  int _output_stride_time = 1 ;
  int _output_dim_freq = (_input_dim_freq - _filter_size_freq) / _output_stride_freq + 1 ;
  int _output_dim_time = (_input_dim_time - _filter_size_time) / _output_stride_time + 1 ;
  int _output_channels = 64 ;
  
  int _iter = 1 ;
  

  ZMat*** _filter_data = Z_SAFE_NEW_AR2(_filter_data, ZMat*, _output_channels, _input_channels);
  for (int i = 0 ; i < _output_channels ; i ++) {
    for (int j = 0 ; j < _input_channels; j ++) {
      _filter_data[i][j] = Z_SAFE_NEW(_filter_data[i][j], ZMat, _filter_size_freq, _filter_size_time);
      _filter_data[i][j]->RandInit() ;
    }
  }
  
  ZMat** _input_data = Z_SAFE_NEW_AR1(_input_data, ZMat*, _input_channels);
  for (int i = 0 ; i < _input_channels ; i ++) {
    _input_data[i] = Z_SAFE_NEW(_input_data[i], ZMat, _input_dim_freq, _input_dim_time);
    _input_data[i]->RandInit() ;
  }
  
  ZMat** _output_data = Z_SAFE_NEW_AR1(_output_data, ZMat*, _output_channels) ;
  for (int i = 0 ; i < _output_channels ; i ++) {
    _output_data[i] = Z_SAFE_NEW(_output_data[i], ZMat, _output_dim_freq, _output_dim_time);
  }
  
  printf("Init Ok\n");
  
  unsigned long RAW_START = GetMyTickCount() ;
  //rawtest
  for (int iter = 0 ; iter < _iter ; iter ++) {
    for (int i = 0 ; i < _output_channels ; i ++) {
      _output_data[i]->Clean() ;
      
      for (int m = 0; m < _output_dim_freq;  m ++) {
        for (int n = 0 ; n < _output_dim_time ; n ++) {
          
          for ( int a = 0 ; a < _filter_size_freq ; a ++) {
            for (int b = 0 ; b < _filter_size_time ; b ++) {
              for (int c = 0 ; c < _input_channels ; c ++) {
                _output_data[i]->data[m][n] += _filter_data[i][c]->data[a][b] * _input_data[c]->data[m*_output_stride_freq + a][n * _output_stride_time + b] ;
              }
            }
          }
        }
      }
    }
  }
  
  unsigned long RAW_END = GetMyTickCount() ;
  printf("For Each Cnn TIME: %fms\n", (RAW_END - RAW_START ) * 1.0f / _iter);
  
  
  ZMat* _filter_mat = Z_SAFE_NEW(_filter_mat, ZMat, _output_channels, _filter_size_freq * _filter_size_time * _input_channels) ;
  ZMat_Neon* _filter_mat_neon = Z_SAFE_NEW(_filter_mat_neon, ZMat_Neon, _output_channels, _filter_size_freq * _filter_size_time * _input_channels, false);
  for (int i = 0; i < _output_channels; i ++) {
    for (int j = 0 ; j < _input_channels ; j ++) {
      for (int k = 0 ; k < _filter_size_freq ; k ++) {
        memcpy(_filter_mat->data[i] + (j * _filter_size_freq + k) * _filter_size_time, _filter_data[i][j]->data[k], sizeof(float) * _filter_size_time) ;
      }
    }
  }
  _filter_mat_neon->CopyFrom(_filter_mat) ;
  
  
  ZMat* _input_mat = Z_SAFE_NEW(_input_mat, ZMat, _output_dim_freq * _output_dim_time , _filter_size_freq * _filter_size_time * _input_channels) ;
  ZMat_Neon* _input_mat_neon = Z_SAFE_NEW(_input_mat_neon, ZMat_Neon, _output_dim_time * _output_dim_freq , _filter_size_freq * _filter_size_time * _input_channels, true);
  
  ZMat* _output_mat = Z_SAFE_NEW(_output_mat, ZMat, _output_channels, _output_dim_freq * _output_dim_time);
  ZMat_Neon* _output_mat_neon = Z_SAFE_NEW(_output_mat_neon, ZMat_Neon, _output_channels, _output_dim_freq * _output_dim_time, false) ;
  
  unsigned long QUICK_START = GetMyTickCount() ;
  
  for (int iter = 0 ; iter < _iter ; iter ++) {
    
    for (int m = 0 ; m < _output_dim_freq; m ++) {
      for (int n = 0 ; n < _output_dim_time ; n ++) {
        float* pData = _input_mat->data[m * _output_dim_time + n] ;
        for (int i = 0 ; i < _input_channels ; i ++) {
          for (int j = 0 ; j < _filter_size_freq ; j ++) {
            memcpy(pData + (i * _filter_size_freq + j * _filter_size_time), _input_data[i]->data[m * _output_stride_freq + j] + n * _output_stride_time , sizeof(float) * _filter_size_time) ;
          }
        }
      }
    }
    _input_mat_neon->CopyFrom(_input_mat) ;
    
    _output_mat_neon->Clean() ;
    
    _output_mat_neon->Add_aAB_NEON(_filter_mat_neon, _input_mat_neon) ;
    _output_mat_neon->CopyTo(_output_mat) ;
    
  }
  
  
  unsigned long QUICK_END = GetMyTickCount() ;
  printf("For Each Cnn TIME: %fms\n", (QUICK_END - QUICK_START ) * 1.0f / _iter);
  

  
  
  return 0 ;
  
}



int main(int argc, const char * argv[]) {
  
  //ZMat::InitSgemmMultiThread() ;
  
  //TestMat_AB();
  //TestCMat_AB() ;
  //TestGemmLowP();
  
  //TestNeonMat_AB();
  
  
  //ZMat::ExitSgemmMultiThread() ;
  
  ZMat::InitQuickExp() ;
  
  //printf("%s\n", argv[1]) ;
  TestMat_Dnn(argv[1]) ;
  //TestMat_Dnn_Single(argv[1]) ;
  //TestMat_Dnn_Total(argv[1]) ;
  //TestCnn();
  
  //TestMat_Dnn("/Users/hadoop/Documents/XCode/test/workdir_cn/iter000_lr0.002000_trn0.3970_2.4676_cv0.3747_2.5522.mod") ;
  //TestMat_Dnn_Single("/Users/hadoop/Documents/XCode/test/workdir_cn/iter000_lr0.002000_trn0.3970_2.4676_cv0.3747_2.5522.mod") ;
  //TestMat_Dnn_Total("/Users/hadoop/Documents/XCode/test/workdir_cn/relu_all/iter010_lr0.000977_trn0.9522_0.1306_cv0.9504_0.1317.mod") ;
  
  ZMat::ExitQuickExp() ;
  // insert code here...
  std::cout << "Hello, World!\n";
  return 0;
}
