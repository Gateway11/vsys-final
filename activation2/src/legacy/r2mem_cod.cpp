
#include <math.h>
#include <string.h>
#include <assert.h>

#include "r2mem_cod.h"

#define  R2_AUDIO_SAMPLE_RATE  16000
#define  R2_AUDIO_FRAME_MS  10
#define r2_min(a,b)    (((a) < (b)) ? (a) : (b))

#define  R2_SAFE_NEW(p,type,...) new type(__VA_ARGS__)
#define  R2_SAFE_NEW_AR1(p,type,dim1) (type*)r2_new_ar1(sizeof(type),dim1)
#define  R2_SAFE_DEL(p)  do {if(p) { delete p ;p = NULL; } } while (0);
#define  R2_SAFE_DEL_AR1(p)    do {if(p) { delete [] p; p = NULL; } } while (0);

char* r2_new_ar1(int size,int dim1){
    if (dim1 == 0) {
        return nullptr ;
    }
    char * pData = new char[dim1*size];
    if (pData == nullptr) {
        return nullptr ;
    }
    memset(pData,0,size*dim1);
    return pData ;
}

r2mem_cod::r2mem_cod(uint32_t format){
  //Raw
  m_iLen_In = 0 ;
  m_iLen_In_Total = R2_AUDIO_SAMPLE_RATE * 20 ;
  m_pData_In = R2_SAFE_NEW_AR1(m_pData_In, float, m_iLen_In_Total);
  
  //Cod
  m_iLen_Cod = 0 ;
  m_iLen_Frm_Cod = R2_AUDIO_SAMPLE_RATE / 1000 * R2_AUDIO_FRAME_MS * 2 ;
  m_pData_Cod = R2_SAFE_NEW_AR1(m_pData_Cod, unsigned char, m_iLen_Frm_Cod * sizeof(float));
  
  m_bPaused = false ;
  
  //Out
  m_iLen_Out = 0 ;
  m_iLen_Out_Cur = 0 ;
  m_iLen_Out_Total = R2_AUDIO_SAMPLE_RATE * 20 ;
  m_pData_Out = R2_SAFE_NEW_AR1(m_pData_Out, char, m_iLen_Out_Total);
  
  reset() ;
  
  m_iLen_Pause = 0 ;
  
  m_iShield_TooLong = R2_AUDIO_SAMPLE_RATE * 6 ;
  m_iShield_Resume = R2_AUDIO_SAMPLE_RATE * 0.5f ;

  m_fShield_Am = 1.0f ;
  m_iLen_Am = m_iLen_Frm_Cod * 3 ;
  
  scaling = (format == AUDIO_FORMAT_ENCODING_PCM_16BIT || format == AUDIO_FORMAT_ENCODING_PCM_24BIT) ? 32768 : 16384;
}


r2mem_cod::~r2mem_cod(void){
  
  R2_SAFE_DEL_AR1(m_pData_In);
  R2_SAFE_DEL_AR1(m_pData_Cod);
  R2_SAFE_DEL_AR1(m_pData_Out);
}

int r2mem_cod::reset(){
  
  m_iLen_In = 0 ;
  m_iLen_Cod = 0 ;
  m_iLen_Out = 0 ;
  m_iLen_Out_Cur = 0 ;
  
  m_bPaused = false ;
  
  return  0 ;
  
}

int r2mem_cod::pause(){
  
  m_iLen_Out_Cur = 0 ;
  m_bPaused = true ;
  m_iLen_Pause = m_iLen_In ;
  
  return  0 ;
}

int r2mem_cod::resume(){
  
  assert(m_bPaused) ;
  m_bPaused = false ;
  
  return  0 ;
}

int r2mem_cod::process(float* pData_In, int iLen_In){
  
  if (iLen_In + m_iLen_In > m_iLen_In_Total) {
    m_iLen_In_Total = (iLen_In + m_iLen_In) * 2 ;
    float* pTmp = R2_SAFE_NEW_AR1(pTmp, float, m_iLen_In_Total);
    memcpy(pTmp, m_pData_In, sizeof(float) * m_iLen_In) ;
    R2_SAFE_DEL_AR1(m_pData_In) ;
    m_pData_In = pTmp ;
  }
  
  for (int i = 0 ; i < iLen_In ; i ++ , m_iLen_In ++ ) {
    m_pData_In[m_iLen_In] = pData_In[i] / 32768.0f ;
  }
  
  return  0 ;
  
}

int r2mem_cod::processdata(){
  
  assert(!m_bPaused) ;
  
  if (m_iLen_Cod == 0 && m_iLen_Frm_Cod < m_iLen_In ) {
    if (m_iLen_Am > m_iLen_In) {
      float total = 0.0f ;
      for (int i = 0; i < m_iLen_In ; i ++) {
        total += fabsf(m_pData_In[i]) ;
      }
      total = total / m_iLen_In ;
      if (total > 0.3f) {
        m_fShield_Am = 0.3f / total ;
      }else{
        m_fShield_Am = 1.0f ;
      }
    }else{
      
      float total = 0.0f, total_max = 0.0f ; ;
      for (int i = 0 ; i < m_iLen_In ; i ++) {
        if (i < m_iLen_Am) {
          total += fabsf(m_pData_In[i]) ;
        }else{
          total += fabsf(m_pData_In[i]) - fabsf(m_pData_In[i-m_iLen_Am]);
          if (total > total_max) {
            total_max = total ;
          }
        }
      }
      total_max = total_max / m_iLen_Am ;
      if (total_max > 0.3f) {
        m_fShield_Am = 0.3f / total_max ;
      }else{
        m_fShield_Am = 1.0f ;
      }
    }
  }
  
  while (m_iLen_Cod + m_iLen_Frm_Cod < m_iLen_In) {
    
    char * pData_Cod = NULL ;
    int iLen_Cod = 0 ;
    
    if (m_fShield_Am < 1.0f) {
      for (int i = 0 ; i < m_iLen_Frm_Cod ; i ++) {
        m_pData_In[m_iLen_Cod + i] = m_pData_In[m_iLen_Cod + i] * m_fShield_Am ;
      }
    }
    
      pData_Cod = (char*)(m_pData_In + m_iLen_Cod);
      iLen_Cod = sizeof(float) * m_iLen_Frm_Cod ;
    
    //store
    if (m_iLen_Out + iLen_Cod > m_iLen_Out_Total)	{
      m_iLen_Out_Total = (m_iLen_Out + iLen_Cod) * 2 ;
      char* pTmp = R2_SAFE_NEW_AR1(pTmp, char, m_iLen_Out_Total);
      memcpy(pTmp, m_pData_Out, m_iLen_Out * sizeof(unsigned char));
      R2_SAFE_DEL_AR1(m_pData_Out);
      m_pData_Out = pTmp ;
      
    }
      float *temp = (float *)pData_Cod;
      short *buff = (short *)(m_pData_Out + m_iLen_Out);
      for(int i = 0; i < m_iLen_Frm_Cod; i++){
          buff[i] = (short)(temp[i] * scaling);
      }
      iLen_Cod /= 2;
//    memcpy(m_pData_Out + m_iLen_Out, pData_Cod, sizeof(char) * iLen_Cod) ;
    m_iLen_Out += iLen_Cod ;
    
    
    m_iLen_Cod += m_iLen_Frm_Cod ;
  }
  
  return 0 ;
}

int r2mem_cod::getdatalen(){
  
  processdata() ;
  return m_iLen_Out - m_iLen_Out_Cur ;
}

int r2mem_cod::getdata(char* pData, int iLen){
  
  processdata() ;
  
    int ll = r2_min(iLen, m_iLen_Out - m_iLen_Out_Cur);
  memcpy(pData, m_pData_Out + m_iLen_Out_Cur, ll) ;
  m_iLen_Out_Cur += ll ;
  
  return  ll ;
}

int r2mem_cod::getdata2(char* &pData, int &iLen){
  
  processdata() ;
  
  iLen = m_iLen_Out - m_iLen_Out_Cur ;
  pData = m_pData_Out + m_iLen_Out_Cur ;
  m_iLen_Out_Cur += iLen ;
  
  return  0 ;
  
}

bool r2mem_cod::istoolong(){
  
  if (m_iLen_In > m_iShield_TooLong) {
    return true ;
  }else{
    return false ;
  }
}


bool r2mem_cod::isneedresume(){
  
  if (m_bPaused && m_iLen_In < m_iShield_TooLong && (m_iLen_In - m_iLen_Pause) > m_iShield_Resume) {
    return  true ;
  }else{
    return  false ;
  }

}
