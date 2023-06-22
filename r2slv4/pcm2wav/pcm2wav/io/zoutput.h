//
//  zoutput.h
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zoutput__
#define __r2vt4__zoutput__

#include "../zmath.h"

namespace __r2vt4__ {

  enum ZOutputType {
    ZOUT_KALDI_BINERY,
    ZOUT_KALDI_TXT,
    ZOUT_RAW_FORMAT
  };
  
  class ZOutput
  {
  public:
    ZOutput(ZOutputType iOutputType);
  public:
    virtual ~ZOutput(void);
    
    template<class T> void WriteBasicType(const T *t) {
      
      char len_c = (std::numeric_limits<T>::is_signed ? 1 :  -1) * static_cast<char>(sizeof(T));
      PutChar(len_c);
      WriteRawData((const char*)t, sizeof(T));
    }
    
    template<class T> void WriteArray(const T *t, int num){
      
      WriteRawData((const char*)t, sizeof(T) * num);
    }
    
    void WriteToken(const char* pToken) ;
    
  protected:
    int PutChar(char c);
    virtual int WriteRawData(const char* pDataBuff, size_t iDataLen) = 0 ;
    
  public:
    ZOutputType m_iOutputType ;
  };
  
  class ZBuffOutput: public ZOutput
  {
  public:
    ZBuffOutput(char* &pDataBuff, int &iDataLen, ZOutputType iOutputType);
  public:
    virtual ~ZBuffOutput(void);
    
  protected:
    virtual int WriteRawData(const char* pDataBuff, size_t iDataLen);
    
  private:
    char*& m_pDataBuff ;
    int& m_iWritePos ;
    int m_iTotalLen ;
    
  };
  
  
  class ZFileOutput: public ZOutput
  {
  public:
    ZFileOutput(const char* pFilePath, ZOutputType iOutputType);
  public:
    virtual ~ZFileOutput(void);
    
  public:
    bool isValid();
    
  protected:
    virtual int WriteRawData(const char* pDataBuff, size_t iDataLen);
    
  private:
    FILE* m_pFile ;
  };
  
};


#endif /* __r2vt4__zoutput__ */
