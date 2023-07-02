#ifndef R2_MEM_COD_H
#define R2_MEM_COD_H

#include "vsys_types.h"

class r2mem_cod
{
public:
  r2mem_cod(uint32_t format);
public:
  ~r2mem_cod(void);
  
  int reset();
  int pause();
  int resume();
  int process(float* pData_In, int iLen_In);
  int processdata();
  
  int getdatalen();
  int getdata(char* pData, int iLen);
  int getdata2(char* &pData, int &iLen);
  
  bool istoolong();
  bool isneedresume();
  
  int m_iShield_TooLong ;
  int m_iShield_Resume ;
  
  
public:
  
  uint32_t scaling;
  
  int m_iLen_In ;
  int m_iLen_In_Total ;
  float * m_pData_In;
  
  int m_iLen_Pause ;
  
  int m_iLen_Cod ;
  int m_iLen_Frm_Cod ;
  unsigned char* m_pData_Cod ;
  
  bool m_bPaused ;
  
  int m_iLen_Out ;
  int m_iLen_Out_Cur ;
  int m_iLen_Out_Total ;
  char* m_pData_Out ;

  //for amplitude norm
  int m_iLen_Am ;
  float m_fShield_Am ;
  
};

#endif
