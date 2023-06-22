//
//  zinput.h
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zinput__
#define __r2vt4__zinput__

#include "../zmath.h"

namespace __r2vt4__ {

  enum ZInputType {
    ZIN_KALDI_BINERY,
    ZIN_KALDI_TXT,
    ZIN_RAW_FORMAT
  };
  
  class ZInput
  {
  public:
    ZInput(ZInputType iInputType);
  public:
    virtual ~ZInput(void);
    
    template<class T> void ReadBasicType(T *t){
      
      int len_c_in = GetChar();
      //int len_c_expected = (std::numeric_limits<T>::is_signed ? 1 :  -1) * sizeof(T);
      int len_c_expected = sizeof(T);
      if (len_c_in !=  len_c_expected) {
        assert(0);
        //ZLOG_ERROR("ReadBasicType Error");
      }
      ReadRawData((char*)t, sizeof(T));
    }
    
    template<class T> void ReadArray(T *t, int num){
      
      ReadRawData((char*)t, sizeof(T) * num);
    }
    
    std::string ReadToken() ;
    bool ExpectToken(const char* pExpectToken) ;
    virtual bool IsEof() = 0 ;
    
  protected:
    char GetChar();
    virtual int ReadRawData(char* pDataBuff, size_t iDataLen) = 0 ;
    
    
  public:
    ZInputType m_iInputType ;
    
  };
  
  
  class ZBuffInput: public ZInput
  {
  public:
    ZBuffInput(char* pDataBuff, int iDataLen, ZInputType iInputType);
  public:
    virtual ~ZBuffInput(void);
    
  protected:
    virtual int ReadRawData(char* pDataBuff, size_t iDataLen);
    virtual bool IsEof(){
      return m_iDataLen == m_iReadPos ;
    }
    
  private:
    char* m_pDataBuff ;
    int m_iDataLen ;
    int m_iReadPos ;
  };
  
  
  class ZFileInput: public ZInput
  {
  public:
    ZFileInput(const char* pFilePath, ZInputType iInputType);
  public:
    virtual ~ZFileInput(void);
    
  public:
    bool isValid();
    
  protected:
    virtual int ReadRawData(char* pDataBuff, size_t iDataLen);
    virtual bool IsEof(){
      return !(feof(m_pFile) == 0) ;
    }
    
  private:
    FILE* m_pFile ;
  };
  
};


#endif /* __r2vt4__zinput__ */
