//
//  main.cpp
//  testphone
//
//  Created by hadoop on 3/31/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include <iostream>

#include "../../r2vt4/src/vt/dep/zphosto.h"
#include "../../r2vt4/src/dnn/znnet.h"

using namespace __r2vt4__ ;



int main(int argc, const char * argv[]) {
  
  //char* pNnetPath = "/Users/hadoop/Documents/XCode/test/workdir_en/en.final.svd.nnet" ;
  
  //Nnet* pNnet = Z_SAFE_NEW(pNnet, Nnet, pNnetPath, true) ;
  
  char* pMonPhoPath = "/Users/hadoop/Documents/XCode/test/workdir_en/monophone.en" ;
  char* pFixTriPhoPath = "/Users/hadoop/Documents/XCode/test/workdir_en/en.tri3a.hmms.fix" ;
  char* pTriPhoMapPath = "/Users/hadoop/Documents/XCode/test/workdir_en/en_tiedlist_htk_full" ;
  
  char* pNewTablePath = "/Users/hadoop/Documents/XCode/test/workdir_en/phonetable" ;
  
  //char* pWordTriPhoLst = "a1-r+e4|I1-l+IH1_E|sil-r_B+w|sil-l_B+w|#;r_B-w+o4|l_B-w+o4;w_B-o1+j|#;o2-q+i2|o2-q+i2_E|#;j_B-i2+sil|j_B-i2+A1|q_B-i2+E1|q_B-i2+N|q_B-i2+W|q_B-i2+b|q_B-i2+c|q_B-i2+ch|q_B-i2+d|j_B-i2+e1|j_B-i2+er3|q_B-i2+f|q_B-i2+g|q_B-i2+h|q_B-i2+i1|q_B-i2+j|q_B-i2+k|q_B-i2+l|q_B-i2+m|q_B-i2+p|q_B-i2+q|q_B-i2+r|q_B-i2+s|q_B-i2+sh|q_B-i2+t|j_B-i2+v|q_B-i2+x|j_B-i2+y|q_B-i2+z|q_B-i2+zh|q_B-i2_E+A1|q_B-i2_E+W|q_B-i2_E+b|q_B-i2_E+c|q_B-i2_E+ch|q_B-i2_E+d|j_B-i2+o3_E|q_B-i2_E+er3|q_B-i2_E+f|q_B-i2_E+g|q_B-i2_E+h|q_B-i2_E+i1|q_B-i2_E+j|q_B-i2_E+k|q_B-i2_E+l|q_B-i2_E+m|q_B-i2_E+p|q_B-i2_E+q|q_B-i2_E+r|q_B-i2_E+s|q_B-i2_E+sh|q_B-i2_E+t|q_B-i2_E+x|q_B-i2_E+z|q_B-i2_E+zh|#;" ;
  
  char* pWordTriPhoLst= "sil-m_B+E1|#;m_B-E2+sil;E1_B-Y_E+sh|E1_B-Y_E+s|#;Y-sh+I4|Y-s+E1|#;c_B-I1+u1|ch_B-IH4+l|s_B-I1+l|r_B-IH4+E1|#;I3-l+e3|I3-l+a1|#;l_B-e3+sil|l_B-a3_E+sil|#;" ;
  
  ZPhoTable* pTable = Z_SAFE_NEW(pTable, ZPhoTable) ;
  pTable->LoadOldTable(pMonPhoPath, pFixTriPhoPath, pTriPhoMapPath) ;
  
  pTable->SaveNewTable(pNewTablePath) ;
  
  //pTable->LoadNewTable(pNewTablePath) ;
  std::vector<ZPhoBatch*> BatchLst = pTable->ParseWordTriPhoLst(pWordTriPhoLst) ;
  Z_SAFE_DEL(pTable);
  
  return  0 ;
  
}
