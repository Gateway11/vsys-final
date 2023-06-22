//
//  zfilter.cpp
//  r2vt4
//
//  Created by hadoop on 3/9/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#include "zfilter.h"

namespace  __r2vt4__ {
  
  //--------------------------------------------------------------
  ZFilter::ZFilter(int iLeftPos, int iRightPos, int iInDim, int iOutDim){
    
    InitFilter(iLeftPos,iRightPos,iInDim,iOutDim);
  }
  
  ZFilter::~ZFilter(void)
  {
    Z_SAFE_DEL(m_pInFea);
    Z_SAFE_DEL(m_pOutFea);
    
    Z_SAFE_DEL_AR1(m_pSingleData);
  }
  
  
  int ZFilter::InitFilter(int iLeftPos, int iRightPos, int iInDim, int iOutDim){
    
    m_iInDim = iInDim ;
    m_iOutDim = iOutDim ;
    
    int iSS = zmin(iLeftPos,0);
    int iEE = zmax(iRightPos,0);
    m_iInBuffLen = iEE - iSS + 1 ;
    m_iLeftPos = iLeftPos - iSS ;
    m_iRightPos = iRightPos - iSS ;
    m_iCurPos = 0 - iSS ;
    
    m_iInFeaPos = 0 ;
    m_pInFea = Z_SAFE_NEW(m_pInFea,ZMat,m_iInBuffLen - 1,m_iInDim);
    
    m_iOutBuffLen = 16 ;
    m_pOutFea = Z_SAFE_NEW(m_pOutFea,ZMat,m_iOutBuffLen,m_iOutDim);
    
    m_pSingleData = Z_SAFE_NEW_AR1(m_pSingleData,float*, m_iInBuffLen);
    
    m_bFirstInFrame = true ;
    m_bFirstOutFrame = true ;
    return 0 ;
  }
  
  int ZFilter::FilterFea(ZMat* pInFea, int iInFeaNum, ZMat* &pOutFea, int &iOutFeaNum){
    
    if (iInFeaNum + m_iInFeaPos > m_iOutBuffLen){
      m_iOutBuffLen = iInFeaNum + m_iInFeaPos + 16 ;
      Z_SAFE_DEL(m_pOutFea);
      m_pOutFea = Z_SAFE_NEW(m_pOutFea,ZMat,m_iOutBuffLen,m_iOutDim);
    }
    
    if (iInFeaNum > 0 && m_bFirstInFrame){
      OnFirstInFrame(pInFea->data[0]);
      m_bFirstInFrame = false ;
    }
    
    m_iOutFeaPos = 0 ;
    if (iInFeaNum + m_iInFeaPos > m_iInBuffLen - 1){
      for (int m = m_iInBuffLen - 1 - m_iInFeaPos ; m < iInFeaNum ; m ++ ){
        if (m_iRightPos - m_iCurPos == 7) {
          m_iRightPos = m_iRightPos ;
        }
        
        for (int n = 0 ; n < m_iInBuffLen ; n ++){
          if (m < n){
            m_pSingleData[m_iInBuffLen - 1 - n] = m_pInFea->data[m_iInFeaPos+m-n];
          }else{
            m_pSingleData[m_iInBuffLen - 1 - n] = pInFea->data[m-n];
          }
        }
        DoSingleFilter(m_pSingleData,m_bFirstOutFrame);
        
        //shft for last frame
        if (m == iInFeaNum - 1){
          for (int n = 0 ; n < m_iInBuffLen - 1 ; n ++){
            memcpy(m_pInFea->data[n],m_pSingleData[n+1],sizeof(float)*m_iInDim);
          }
        }
        if (m_bFirstOutFrame){
          m_bFirstOutFrame = false ;
        }
        m_iOutFeaPos ++ ;
      }
      m_iInFeaPos = m_iInBuffLen - 1 ;
    }else if(iInFeaNum > 0){
      
      for (int i = 0 ; i <  iInFeaNum; i ++) {
        memcpy(m_pInFea->data[m_iInFeaPos + i], pInFea->data[i], sizeof(float) * m_iInDim);
      }
      m_iInFeaPos += iInFeaNum ;
    }
    
    pOutFea = m_pOutFea ;
    iOutFeaNum = m_iOutFeaPos ;
    
    return 0 ;
    
  }
  
  int ZFilter::ResetFilter(){
    
    m_iOutFeaPos = 0 ;
    m_iInFeaPos = 0 ;
    
    m_bFirstInFrame = true ;
    m_bFirstOutFrame = true ;
    
    return 0 ;
    
  }
  
  //ZFilter_Delt----------------------------------------------------------------------------------------------------------
  ZFilter_Delt::ZFilter_Delt(int iLeftPos, int iRightPos, int iInDim, int iOutDim)
  :ZFilter(iLeftPos,iRightPos,iInDim,iOutDim){
  }
  
  ZFilter_Delt::~ZFilter_Delt(void)
  {
  }
  
  int ZFilter_Delt::DoSingleFilter(float** pFeaIn, bool bFirstOutFrm){
    
    float * pFeaOut = m_pOutFea->data[m_iOutFeaPos];
    memcpy(pFeaOut,pFeaIn[m_iCurPos],sizeof(float)*m_iInDim);
    int ll = m_iOutDim - m_iInDim ;
    int ss = m_iInDim - ll ;
    for (int i = 0 ; i < ll ; i ++){
      pFeaOut[m_iInDim + i] = (2.0f*(pFeaIn[m_iRightPos][ss + i] - pFeaIn[m_iLeftPos][ss + i])
                               + (pFeaIn[m_iRightPos-1][ss + i] - pFeaIn[m_iLeftPos+1][ss + i])) / 10.0f;
    }
    return 0 ;
  }
  
  int ZFilter_Delt::OnFirstInFrame(float* pFeaIn){
  
    return 0 ;
    
  }
  
#ifdef Z_TF_NNET
  
  static float g_mean[87] = {15.9990282f,16.6069851f,16.6384563f,16.9749088f,17.1593552f,17.1078205f,16.9658051f,16.8324413f,16.7711105f,16.7110691f,16.7340546f,16.7752056f,16.8678837f,16.9334126f,16.9097233f,16.9178028f,17.0687866f,17.2755737f,17.3886471f,17.3881798f,17.4612122f,17.5892487f,16.7016773f,16.5713387f,16.3818359f,16.2471924f,16.0877571f,15.8371525f,15.4049778f,0.0011494f,0.0010173f,0.0008637f,0.0008497f,0.0007389f,0.0006822f,0.0007140f,0.0007231f,0.0006424f,0.0005600f,0.0005213f,0.0004211f,0.0002511f,0.0000813f,-0.0000423f,-0.0000430f,0.0000433f,0.0001192f,0.0001339f,0.0000899f,0.0000218f,0.0000014f,0.0000515f,0.0000670f,0.0000526f,0.0000756f,0.0001079f,0.0000728f,0.0000151f,-0.0152900f,-0.0158032f,-0.0159841f,-0.0162367f,-0.0163882f,-0.0165462f,-0.0167098f,-0.0168513f,-0.0170111f,-0.0171102f,-0.0172736f,-0.0174784f,-0.0176892f,-0.0178534f,-0.0179263f,-0.0179864f,-0.0181543f,-0.0182760f,-0.0182311f,-0.0181999f,-0.0183390f,-0.0185275f,-0.0173732f,-0.0174342f,-0.0174987f,-0.0174639f,-0.0173187f,-0.0170296f,-0.0164782f} ;
  static float g_var[87] = {3.6834488f,3.7444546f,3.7366984f,3.8071036f,3.8643873f,3.8429060f,3.7959638f,3.7250116f,3.6527643f,3.6066942f,3.5723710f,3.5332839f,3.5043569f,3.4680669f,3.4044909f,3.3499923f,3.3384132f,3.3518388f,3.3654323f,3.3439443f,3.3363743f,3.3672435f,5.3875322f,5.3231144f,5.2373800f,5.1989098f,5.1716738f,5.1255817f,5.0392685f,0.6847376f,0.7140540f,0.7247341f,0.7453557f,0.7599887f,0.7595286f,0.7498437f,0.7371762f,0.7261569f,0.7191843f,0.7174401f,0.7178849f,0.7180113f,0.7161793f,0.7089777f,0.7024556f,0.7017723f,0.7045895f,0.7076374f,0.7077351f,0.7110642f,0.7169318f,0.6949537f,0.6927273f,0.6930136f,0.6941982f,0.6923096f,0.6844098f,0.6697469f,0.2327710f,0.2431262f,0.2475486f,0.2549773f,0.2596903f,0.2569836f,0.2497762f,0.2426901f,0.2369795f,0.2338764f,0.2330832f,0.2336972f,0.2337191f,0.2329607f,0.2301853f,0.2274966f,0.2263993f,0.2268861f,0.2283831f,0.2285734f,0.2296042f,0.2313710f,0.2242439f,0.2243714f,0.2250304f,0.2255793f,0.2254038f,0.2237812f,0.2202581f};
  
#else
  //ZFilter_Norm----------------------------------------------------------------------------------------------------------
  float ZFilter_Norm::avg_data[174] = {
    16.2404f,16.6128f,16.596f,16.6401f,16.6479f,16.345f,16.1058f,15.9906f,15.717f,15.4157f,15.0315f,15.0621f,15.4669f,15.8933f,15.7233f,15.1394f,15.4232f,15.7599f,15.1383f,14.4325f,14.6703f,15.2585f,15.1616f,14.9356f,14.8994f,14.9524f,14.4738f,13.9604f,13.569f,
    -0.000119989f,6.04623e-05f,1.72714e-05f,0.000259089f,0.0004497f,0.000623553f,0.000622171f,0.000517424f,0.000598736f,0.000702944f,0.000954144f,0.00081793f,0.000647984f,0.000632524f,0.000529432f,0.00011709f,5.10208e-05f,0.000134383f,8.05519e-05f,4.58274e-05f,-1.99467e-06f,0.000221098f,0.00030144f,0.000502931f,6.78106e-05f,-2.25175e-05f,-0.000151579f,-0.000140346f,-0.000481125f,
    7.56482e-05f,6.25931e-05f,-7.51853e-05f,-6.93187e-05f,-3.156e-05f,8.76753e-06f,-4.55435e-05f,-7.62564e-05f,-9.68782e-05f,-7.20891e-05f,-3.60915e-05f,-4.55111e-05f,-6.57399e-05f,-9.77991e-05f,-7.50835e-05f,-3.40042e-05f,-4.8123e-05f,-7.45169e-05f,1.46673e-05f,0.000123412f,6.54317e-05f,9.54372e-06f,3.49851e-05f,4.30365e-05f,6.06784e-05f,8.75432e-05f,0.000130353f,0.000196138f,0.000326151f,
    266.582f,279.219f,279.197f,281.015f,282.193f,273.124f,265.165f,260.602f,252.293f,243.42f,232.63f,234.0f,245.694f,258.449f,252.563f,233.888f,242.625f,253.604f,235.232f,214.958f,221.661f,239.538f,235.336f,228.718f,226.508f,227.733f,212.934f,198.197f,187.156f,
    0.127423f,0.12421f,0.125651f,0.127897f,0.139724f,0.147135f,0.14341f,0.131072f,0.126664f,0.126852f,0.130669f,0.140453f,0.128747f,0.12122f,0.112891f,0.100734f,0.111503f,0.11987f,0.128837f,0.130588f,0.117024f,0.119949f,0.0944461f,0.0974626f,0.0811213f,0.0847066f,0.071645f,0.0677473f,0.0655029f,
    0.0242975f,0.0232687f,0.0233267f,0.0234409f,0.0252594f,0.0254787f,0.0242605f,0.022068f,0.0207452f,0.0203909f,0.0201786f,0.021325f,0.0191649f,0.0181111f,0.0170093f,0.015285f,0.0174437f,0.0187107f,0.0196249f,0.0192994f,0.0172593f,0.0177097f,0.0136227f,0.0141941f,0.0119386f,0.0126343f,0.0105357f,0.00972856f,0.0094232f
  };
  
#endif
  
  ZFilter_Norm::ZFilter_Norm(int iLeftPos, int iRightPos, int iInDim, int iOutDim)
  :ZFilter(iLeftPos,iRightPos,iInDim,iOutDim){
    
    m_iNormLen = m_iRightPos - m_iLeftPos + 1 ;
    
    m_pNormMat = Z_SAFE_NEW(m_pNormMat,ZMat,2,m_iInDim);
    
    m_pfMean = Z_SAFE_NEW_AR1(m_pfMean,float, m_iInDim);
    m_pfVar = Z_SAFE_NEW_AR1(m_pfVar,float, m_iInDim);
    
#ifdef Z_TF_NNET
    for (int i = 0 ; i < iInDim;  i ++) {
      m_pfMean[i] = g_mean [i] ;
      m_pfVar[i] = g_var[i] ;
    }
#else
    for (int i = 0 ; i < iInDim;  i ++) {
      m_pfMean[i] = avg_data [i] ;
      m_pfVar[i] = sqrtf(avg_data[i + m_iInDim] - m_pfMean[i] * m_pfMean[i]) ;
    }
#endif
    
  }
  
  ZFilter_Norm::~ZFilter_Norm(void)
  {
    Z_SAFE_DEL(m_pNormMat);
    
    Z_SAFE_DEL_AR1(m_pfMean);
    Z_SAFE_DEL_AR1(m_pfVar);
  }
  
  int ZFilter_Norm::DoSingleFilter(float** pFeaIn, bool bFirstOutFrm){
    
    if (bFirstOutFrm && m_iLeftPos < m_iCurPos) {
      for (int i = 0 ; i < m_iInDim ; i ++){
        for (int j = m_iLeftPos; j < m_iCurPos ; j ++) {
          m_pOutFea->data[m_iOutFeaPos + j - m_iLeftPos][i] = (pFeaIn[j][i] - m_pfMean[i]) / m_pfVar[i]  ;
        }
      }
      m_iOutFeaPos += m_iCurPos - m_iLeftPos ;
    }
    
    float * pFeaOut = m_pOutFea->data[m_iOutFeaPos];
    
    for (int i = 0 ; i < m_iInDim ; i ++){
        pFeaOut[i] = (pFeaIn[m_iCurPos][i] - m_pfMean[i]) / m_pfVar[i]  ;
      
    }
    
    return 0 ;
  }
  
  int ZFilter_Norm::OnFirstInFrame(float* pFeaIn){
    
    return 0 ;
    
  }
  
  //ZFilter_Cmb----------------------------------------------------------------------------------------------------------
  ZFilter_Cmb::ZFilter_Cmb(int iLeftPos, int iRightPos, int iInDim, int iOutDim)
  :ZFilter(iLeftPos,iRightPos,iInDim,iOutDim){
    
  }
  
  ZFilter_Cmb::~ZFilter_Cmb(void){

  }
  
  int ZFilter_Cmb::DoSingleFilter(float** pFeaIn, bool bFirstOutFrm){
    
    float * pFeaOut = m_pOutFea->data[m_iOutFeaPos];
    for (int i = 0 ; i < m_iInBuffLen ; i ++){
      memcpy(pFeaOut + i*m_iInDim , pFeaIn[i],sizeof(float)*m_iInDim);
    }
    return 0 ;
  }
  
  int ZFilter_Cmb::OnFirstInFrame(float* pFeaIn){
    
    if (m_iLeftPos < m_iCurPos){
      for (int i = m_iLeftPos ; i < m_iCurPos ; i ++){
        memcpy(m_pInFea->data[i],pFeaIn,sizeof(float)*m_iInDim);
      }
      m_iInFeaPos = m_iCurPos ;
    }
    return 0 ;
    
  }
  
  
}



