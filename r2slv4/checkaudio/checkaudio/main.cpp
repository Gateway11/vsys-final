//
//  main.cpp
//  checkaudio
//
//  Created by hadoop on 8/18/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include <iostream>

int main(int argc, const char * argv[]) {
  // insert code here...
  
  const char * pUsage = "Error Param\n Usage:\tcheckaudio wavpath offset totlachannel checkchannel\n" ;
  if (argc != 5) {
    printf(pUsage);
    return 0 ;
  }
  
  //const char* pWavPath = argv[1] ;
  //int iOffset = atoi(argv[2]);
  //int iTotalCn = atoi(argv[3]) ;
  //int iCheckCn = atoi(argv[4]) ;
  
  const char* pWavPath = "/Users/hadoop/raw_data.pcm" ;
  int iOffset = 0;
  int iTotalCn = 6 ;
  int iCheckCn = 2 ;
  
  FILE* pFile = fopen(pWavPath, "rb") ;
  if (pFile == NULL) {
    printf("Failed to Read File %s\n", pWavPath);
    return 0 ;
  }
  
  fseek(pFile, 0, SEEK_END) ;
  int len = (ftell(pFile) - iOffset) / sizeof(int) ;
  unsigned int * pData_Int32 = new unsigned int[len] ;
  fseek(pFile, iOffset, SEEK_SET) ;
  fread(pData_Int32, sizeof(int), len, pFile);
  fclose(pFile);
  
  printf("Read File Ok %s\n", pWavPath);
  
  int iLen = len / iTotalCn ;
  
  for (int i = 0; i < iTotalCn ; i ++) {
    int iii_cur = -1 , iii_last = -1 ;
    if (i == iCheckCn) {
      for (int j = 0 ; j < iLen ; j ++) {
        iii_cur = (pData_Int32[j * iTotalCn + i])  % 256 ;
        if (iii_cur > 0 && iii_last > 0) {
          if (iii_cur != iii_last + 1) {
            printf("%d: %d %d--------\n", j, iii_last, iii_cur);
          }
        }
        iii_last = iii_cur ;
        //printf("%d: %d\n", j, iii_cur );
      }
    }
  }
  
  delete [] pData_Int32 ;
  
  std::cout << "Hello, World!\n";
  return 0;
}
