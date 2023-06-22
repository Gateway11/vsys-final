//
//  zaudapi.h
//  r2vt4
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zaudapi__
#define __r2vt4__zaudapi__

#ifdef __cplusplus
  extern "C" {
#endif
    
    struct r2_aud {
      int  cn ;
      int  sr ;
      int  len ;
      float ** data ;
    };
    
    
    /**
     *  创建r2_aud结构体
     *
     *  @param cn  语音通道个数
     *  @param sr  语音采样率
     *  @param len 语音长度
     *
     *  @return r2_aud结构体指针，NULL：失败；非NULL：成功
     */
    r2_aud* r2_aud_malloc(int cn, int sr, int len) ;
    
    /**
     *  释放r2_aud结构体
     *
     *  @param pAud r2_aud结构体指针
     *
     *  @return 0:成功 1:失败
     */
    int r2_aud_free(r2_aud* pAud) ;
    
    /**
     *  从文件读取语音数据
     *
     *  @param pAudPath 文件路径
     *
     *  @return r2_aud结构体指针，NULL：失败；非NULL：成功
     */
    r2_aud* r2_aud_in(const char* pAudPath, int iSr) ;
    
    /**
     数据格式
     */
    enum pcm_format{
      format_int16 ,
      format_int24 ,
      format_int32 ,
      format_float32
    };
    /**
     *  按照指定的格式读取语音文件
     *
     *  @param pPcmPath 文件路径
     *  @param iOffset  偏移
     *  @param iSr      采样率
     *  @param iCn      通道数
     *  @param iFormat  数据格式
     *
     *  @return <#return value description#>
     */
    r2_aud* r2_pcm_in(const char* pPcmPath, int iOffset, int iSr, int iCn, pcm_format iFormat) ;
    
    /**
     *  将r2_aud结构体存储到文件中
     *
     *  @param pAudPath 文件路径
     *  @param pAud     r2_aud结构体指针
     *
     *  @return 0:成功；1:失败
     */
    int r2_aud_out(const char* pAudPath, r2_aud* pAud) ;
    
    /**
     *  根据采样率转换，并生成新的r2_aud
     *
     *  @param pAud 旧r2_aud
     *  @param iSr  新采样率
     *
     *  @return 新的r2_aud
     */
    r2_aud* r2_aud_rs(const r2_aud* pAud, int iSr) ;
    
#ifdef __cplusplus
  };
#endif
  

#endif /* __r2vt4__zaudapi__ */
