//
//  main.cpp
//  CalculateEER
//
//  Created by hadoop on 6/21/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>


std::vector<std::string> GetFileContentLst(const char* pFilePath){
  
  std::vector<std::string> FileContentList ;
  
  FILE * pFile = fopen(pFilePath,"rb");
  if (pFile == NULL) {
    printf("Failed to load file %s",pFilePath);
    return FileContentList ;
  }
  
  char *line = new char[5120];
  while(!feof(pFile)) {
    line[0] = '\0';
    fgets(line,5120,pFile);
    strtok(line,"\r\n");
    if(strlen(line) > 3) {
      while (line[strlen(line) - 1] == ' ' || line[strlen(line) - 1] == '\t') {
        line[strlen(line) - 1] = '\0' ;
      }
    }
    if(strlen(line) > 3) {
      FileContentList.push_back(line);
    }
  }
  fclose(pFile);
  
  delete line ;
  
  return FileContentList ;
  
}


int GetScore(const char* pFilePath, int iColStart, int iColEnd, float**& pScore, int &iNum){
  
  std::vector<std::string> FileContentList = GetFileContentLst(pFilePath) ;
  
  iNum = FileContentList.size() ;
  int iCol = iColEnd - iColStart ;
  
  if (iCol <= 0) {
    iCol = 1 ;
  }
  
  pScore = new float*[iNum] ;
  pScore[0] = new float[iNum * iCol] ;
  for (int i = 1 ; i < iNum ; i ++) {
    pScore[i] = pScore[i-1] + iCol ;
  }
  
  char line[256] ;
  for (int i = 0 ; i < iNum ; i ++) {
    strcpy(line, FileContentList[i].c_str()) ;
    char* pS = strtok(line, " \r\n");
    for (int j = 0 ; j < iColEnd ; j ++) {
      if (j >= iColStart) {
        pScore[i][j-iColStart] = atof(pS) ;
      }
      pS = strtok(NULL, " \r\n");
    }
  }
  
  return  0 ;
  
}

struct ScoreRate{
  int id ;
  float miss ;
  float fa ;
};

int mycompare(const void * a, const void * b){
  const ScoreRate* pA = (const ScoreRate* )a ;
  const ScoreRate* pB = (const ScoreRate* )b ;
  if (pA->miss > pB->miss) {
    return 1 ;
  }else if(pA->miss < pB->miss){
    return -1 ;
  }else{
    return 0 ;
  }
}


int CalculateEER(float** pTrueScoreLst, int iTruNum, float** pFalseScoreLst, int iFalseNum, int iCol){
  
  ScoreRate* pRate = new ScoreRate[iTruNum] ;
  
  float* pSsss = new float[iCol] ;
  
  
  for (int i = 0 ; i < iTruNum ; i ++) {
    int iTure = 0 , iFalse = 0 ;
    
    memcpy(pSsss, pTrueScoreLst[i], sizeof(float) * iCol);
//    if (iCol == 2) {
//      pSsss[0] = -0.3f ;
//      pSsss[0] = 4.2f ;
//    }
    
    
    for (int j = 0 ; j < iTruNum ; j ++) {
      bool bOk = true ;
      for (int k = 0  ; k < iCol ; k ++) {
        if(pTrueScoreLst[j][k] <= pSsss[k]){
          bOk = false ;
          break ;
        }
      }
      if (bOk) {
        iTure ++ ;
      }
    }
    for (int j = 0 ; j < iFalseNum ; j ++) {
      bool bOk = true ;
      for (int k = 0  ; k < iCol ; k ++) {
        if(pFalseScoreLst[j][k] <= pSsss[k]){
          bOk = false ;
          break ;
        }
      }
      if (bOk) {
        iFalse ++ ;
      }
    }
    pRate[i].id = i ;
    pRate[i].miss = (iTruNum - iTure) * 1.0f / iTruNum ;
    pRate[i].fa = iFalse * 1.0f / iFalseNum ;
    //printf("%f %f %f\n", pRate[i * 2 + 0] ,pRate[i * 2 + 1], pTrueScoreLst[i][0] ) ;
//    printf("\n");
  }
  
  std::vector<int> FinalPos ;
  for (int i = 0 ; i < iTruNum ; i ++) {
    bool bError = false ;
    for (int j = 0 ; j < iTruNum ; j ++) {
      if (pRate[i].miss > pRate[j].miss && pRate[i].fa > pRate[j].fa ) {
        bError = true ;
        break ;
      }
    }
    if (!bError) {
      FinalPos.push_back(i) ;
    }
    
  }
  
  ScoreRate* pRate2 = new ScoreRate[iTruNum] ;
  for (int i = 0 ; i < FinalPos.size() ; i ++) {
    pRate2[i].id = pRate[FinalPos[i]].id ;
    pRate2[i].miss = pRate[FinalPos[i]].miss ;
    pRate2[i].fa = pRate[FinalPos[i]].fa ;
  }
  qsort(pRate2, FinalPos.size(), sizeof(ScoreRate), mycompare);
  for (int i = 0 ; i < FinalPos.size() ; i ++) {
    printf("miss:%f fa:%f shield:", pRate2[i].miss, pRate2[i].fa) ;
    for (int j = 0 ; j < iCol ; j ++) {
      printf("%f ", pTrueScoreLst[pRate2[i].id][j]);
    }
    printf("\n") ;
  }
  

  int minid = 0 ;
  float minscore = 10000 ;
  for (int i = 0 ; i < FinalPos.size() ; i ++) {
    float ss = fabs(pRate[FinalPos[i]].miss - pRate[FinalPos[i]].fa) ;
    if (minscore > ss ) {
      minid = FinalPos[i] ;
      minscore = ss ;
    }
  }
  
  
  printf("EER:  miss%f fa%f ;\n", pRate[minid].miss, pRate[minid].fa);
  printf("Shield:");
  for (int i = 0 ; i < iCol ; i ++) {
    printf(" %f", pTrueScoreLst[minid][i]) ;
  }
  printf("\n");
  
  delete pRate ;
  delete pRate2 ;
  
  return  0 ;
  
}

int DoCalcEER(const char* pTrueFilePath, const char* pFalseFilePath, int iColStart, int iColEnd){
  
  float** pTrueScoreLst = NULL , ** pFalseScoreLst = NULL;
  int iTruNum = 0 , iFalseNum = 0 ;
  GetScore(pTrueFilePath, iColStart, iColEnd, pTrueScoreLst, iTruNum);
  GetScore(pFalseFilePath, iColStart, iColEnd, pFalseScoreLst, iFalseNum);
  
  
  CalculateEER(pTrueScoreLst, iTruNum, pFalseScoreLst, iFalseNum, iColEnd - iColStart);
  
  delete pTrueScoreLst[0];
  delete pTrueScoreLst ;
  delete pFalseScoreLst[0] ;
  delete pFalseScoreLst ;
  
  return  0 ;
}

int main(int argc, const char * argv[]) {
  
  //const char* pTrueFilePath = "/Users/hadoop/Documents/XCode/test/nnet-forward/true.txt" ;
  //const char* pFalseFilePath = "/Users/hadoop/Documents/XCode/test/nnet-forward/false.txt" ;
  
  const char* pFalseFilePath = "/Users/hadoop/False.txt" ;
  const char* pTrueFilePath = "/Users/hadoop/True.txt" ;
  
  int iColStart = 0 ;
  int iColEnd = 2 ;
  
  if (argc == 4) {
    DoCalcEER(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
  }else{
    DoCalcEER(pTrueFilePath, pFalseFilePath, iColStart, iColEnd);
  }
  
  return  0 ;
  
  
}
