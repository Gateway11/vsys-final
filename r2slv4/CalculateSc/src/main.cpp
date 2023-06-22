//
//  main.cpp
//  CalculateSc
//
//  Created by hadoop on 4/7/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include <iostream>
#include "../../r2vt4/src/zmath.h"
#include "../../r2vt4/src/mat/zmat.h"
#include "../../r2vt4/src/mat/zvec.h"
#include "../../r2vt4/src/io/zinput.h"

using namespace __r2vt4__ ;


int GeneratePrior(const char* pPdfPath, const char* pPriorPath){
  
  int iMaxStateNum = 100000 ;
  double* pNum = Z_SAFE_NEW_AR1(pNum, double, iMaxStateNum);
  
  int iMaxLineNum = 102400 ;
  char* line = Z_SAFE_NEW_AR1(line, char, iMaxLineNum);

  int iStateNum = 0 ;
  
  FILE* pFile_In = fopen(pPdfPath, "rb") ;
  
  int iLineNum = 0 ;
  while ( !feof(pFile_In)) {
    fgets(line, iMaxLineNum, pFile_In) ;
    strtok(line, "\r\n") ;
    if (strlen(line) < 5) {
      continue ;
    }
    std::vector<std::string> IdLst = z_str_split(line, " ");
    if (IdLst.size() > 11) {
      for (int i = 10 ; i < IdLst.size() - 9 ; i ++) {
        int iD = atoi(IdLst[i].c_str()) ;
        pNum[iD] = pNum[iD] + 1 ;
        if (iD > iStateNum) {
          iStateNum = iD ;
        }
      }
    }
    iLineNum ++ ;
    if (iLineNum %100000 == 0) {
      printf("%d\n",iLineNum) ;
    }
  }
  
  fclose(pFile_In) ;
  
  printf("Read Pdf Completely\n") ;
  
  iStateNum = iStateNum + 1 ;
  
  double total = 0.0 ;
  for (int i = 0 ; i < iStateNum ; i ++) {
    total += pNum[i] ;
  }
  
  FILE* pFile_Out = fopen(pPriorPath, "wb");
  printf("StateNum: %d\n", iStateNum) ;
  
  for (int i = 0 ; i < iStateNum ; i ++) {
    pNum[i] = log(pNum[i] / total) ;
    fprintf(pFile_Out, "%0.7ff,", (float)(pNum[i]));
    printf("%0.7ff,", (float)(pNum[i]));
  }
  fprintf(pFile_Out, "\n");
  printf("\n");
  
  fclose(pFile_Out) ;
  
  Z_SAFE_DEL_AR1(pNum);
  Z_SAFE_DEL_AR1(line);
  
  
  return  0 ;
}


int main(int argc, const char * argv[]) {
  
  const char* pPdfPath = "/Users/hadoop/Documents/1.txt" ;
  const char* pPriorPath = "/Users/hadoop/Documents/XCode/test/1.mat" ;
  
  if (argc == 3) {
    GeneratePrior(argv[1],argv[2]);
  }else{
    GeneratePrior(pPdfPath, pPriorPath);
  }
  
  return 0 ;
}
