//
//  zvtapi.h
//  r2vt5
//
//  Created by hadoop on 3/6/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt5__zvtapi__
#define __r2vt5__zvtapi__

#ifdef __cplusplus
extern "C" {
#endif
  
  //Pre Det
#define R2_VT_WORD_PRE          0x0001
  //WORD_DET
#define R2_VT_WORD_DET          0x0002
  //Cancel Det
#define R2_VT_WORD_CANCEL       0x0004
  //Det With Cmd
#define R2_VT_WORD_DET_CMD      0x0008
  //Det Without Cmd
#define R2_VT_WORD_DET_NOCMD    0x0010
  
#define R2_VT_WORD_DET_BEST     0x0020
  
  /**
   *  激活词类型
   *
   *  @param WORD_AWAKE      激活词
   *  @param WORD_SLEEP      休眠词
   *  @param WORD_HOTWORD    热词
   *  @param WORD_OTHER      保留
   */
  enum WordType{
    WORD_AWAKE = 1 ,
    WORD_SLEEP ,
    WORD_HOTWORD ,
    WORD_OTHER 
  };
  
  /**
   *  激活词信息
   *
   *  @param iWordType              激活词类型
   *  @param pWordContent_UTF8      激活词内容，UTF-8编码
   *  @param pWordContent_PHONE     激活词内容，phone串
   *  @param fBlockAvgScore         所有phone声学平均得分门限
   *  @param fBlockMinScore         单个phone声学最小得分门限
   *  @param bLeftSilDet            是否左静音检测
   *  @param bRightSilDet           是否右静音检测
   *  @param bRemoteAsrCheckWithAec     AEC条件下是否需要远端二次确认
   *  @param bRemoteAsrCheckWithNoAec   非AEC条件下是否需要远端二次确认
   *  @param bLocalClassifyCheck    是否需要本地二次确认
   *  @param fClassifyShield        本地二次确认门限
   *  @param pLocalClassifyNnetPath 本地二次确认模型地址
   */

  struct WordInfo{
    WordType  iWordType ;
    char      pWordContent_UTF8[256] ;
    char      pWordContent_PHONE[25600] ;
    float     fBlockAvgScore ;
    float     fBlockMinScore ;
    bool      bLeftSilDet ;
    bool      bRightSilDet ;
    bool      bRemoteAsrCheckWithAec ;
    bool      bRemoteAsrCheckWithNoAec ;
    bool      bLocalClassifyCheck ;
    float     fClassifyShield ;
    char      pLocalClassifyNnetPath[256] ;
  };
  

  /**
   *  激活词检测结果信息
   *
   *  @param iWordPos_Start    激活词开始位置 point
   *  @param iWordPos_End      激活词结束位置 point
   *  @param fEnergy           激活词能量
   *  @param fWordSlInfo       激活词寻向信息
   *  @param fNoiseLevel       激活词噪声级别, 0: 安静  1:嘈杂
   */
  struct WordDetInfo{
    int       iWordPos_Start ;
    int       iWordPos_End ;
    union{
      float     fWordEnergy ;
      float     fEnergy ;
    };
    float     fWordSlInfo[6] ;
    int       iWordSlCandNum ;
    float     fSilShield ;
    int       iNoiseLevel ;
    float     fCfScore ;
  };
  
  
  /** task handle */
  typedef long long r2_vt_htask ;
  
  /**
   激活算法初始化
   */
  int r2_vt_sysinit(const char* pNnetPath, const char* pPhoneTablePath);
  
  /**
   激活算法退出
   */
  int r2_vt_sysexit();
  
  /**
   *  创建激活算法句柄
   *
   *  @return 激活算法句柄
   */
  r2_vt_htask r2_vt_create(int iCn);
  
  /**
   *  释放激活算法句柄
   *
   *  @param hTask 激活算法句柄
   *
   *  @return 0:成功；1：失败
   */
  int r2_vt_free(r2_vt_htask hTask);
  
  /**
   *  设置指定激活词的其他信息
   *
   *  @param hTask             激活算法句柄
   *  @param pWordContent_UTF8 激活词内容，如果为空，则对所有激活词生效，并修改对应参数的默认值
   *  @param iParam            参数编号
   *  @param pValue            参数值
   *
   *  @return 0:成功；1：失败
   */
  
#define R2_VT_PARAM_INT_DIRTY_SET_AEC           1   //AEC条件下DIRTY开关， 默认开
#define R2_VT_PARAM_INT_DIRTY_SET_NOAEC         2   //非AEC条件下DIRTY开关， 默认开
  
#define R2_VT_PARAM_INT_FRM_NUM_MIN             3   //每个字最小帧长，默认10帧
#define R2_VT_PARAM_INT_FRM_NUM_MAX             4   //每个字最大帧长，默认60帧
  
#define R2_VT_PARAM_INT_LEFT_SIL_SKIP           5   //前置静音边界SKIP帧数，默认10帧
#define R2_VT_PARAM_INT_LEFT_SIL_OFFSET         6   //前置静音检测长度，默认50帧
#define R2_VT_PARAM_FLOAT_LEFT_SIL_SHIELD       7   //前置静音检测门限，默认0.5f
  
#define R2_VT_PARAM_INT_RIGHT_SIL_SKIP          8   //后置静音边界SKIP帧数，默认10帧
#define R2_VT_PARAM_INT_RIGHT_SIL_OFFSET        9   //前置静音检测长度，默认35帧
#define R2_VT_PARAM_FLOAT_RIGHT_SIL_SHIELD      10  //前置静音检测门限，默认0.3f
  
  
//---|------------------cf------------------|----
//--------|-----ruo-----|------qi------|--------
#define R2_GVT_PARAM_INT_CF_START_OFFSET        11  //设置二次确认算法开始点位置偏移 默认10帧
#define R2_GVT_PARAM_INT_CF_END_OFFSET          12  //设置二次确认算法结束点位置偏移 默认10帧
  
  int r2_vt_setparam(r2_vt_htask hTask, const char* pWordContent_UTF8, const int iParam, const void* pValue) ;
  
  
  /**
   *  设置激活词（休眠词）
   *
   *  @param hTask    激活算法句柄
   *  @param pWordLst 激活词列表
   *  @param iWordNum 激活词个数
   *
   *  @return 0:成功；1：失败
   */
  int r2_vt_setwords(r2_vt_htask hTask, const WordInfo* pWordLst, int iWordNum);
  
  /**
   *  获取当前系统的激活词信息
   *
   *  @param hTask    激活算法句柄
   *  @param pWordLst 激活词列表
   *  @param iWordNum 激活词个数
   *
   *  @return 0:成功；1：失败
   */
  int r2_vt_getwords(r2_vt_htask hTask, const WordInfo** pWordLst, int* iWordNum);
  
  /**
   *  激活词处理函数
   *
   *  @param hTask    激活算法句柄
   *  @param pWavBuff 语音内容
   *  @param iWavLen  语音长度
   *  @param iVtFlag    AEC:0x0001  VAD_END:0x0002
   *
   *  @return 激活事件 0:没有激活词；非0:检测到激活词
   */
#define  R2_VT_FLAG_AEC       0x0001
#define  R2_VT_FLAG_VAD_END   0x0002
  
  int r2_vt_process(r2_vt_htask hTask, const float** pWavBuff, int iWavLen, int iVtFlag);
  
  /**
   *  获取激活词的检测信息
   *  只有在r2_vt_process返回非0时调用
   *
   *  @param hTask        麦克风阵列激活算法句柄
   *  @param pWordInfo    激活词的信息
   *  @param pWordDetInfo 激活词的检测信息
   *  @param iOffsetFrm   位置偏移
   *
   *  @return 0:成功；1:失败
   */
  int r2_vt_getdetwordinfo(r2_vt_htask hTask, const WordInfo** pWordInfo, const WordDetInfo** pWordDetInfo, int iOffsetFrm);
  
  /**
   *  激活算法重置
   *
   *  @param hTask 激活算法句柄
   *
   *  @return 0:成功；1:失败
   */
  int r2_vt_reset(r2_vt_htask hTask);
  
  //secsl_cb
  typedef int (*dosecsl_callback) (int nFrmStart_Sil, int nFrmEnd_Sil, int nFrmStart_Vt, int nFrmEnd_Vt, WordDetInfo* pWordDetInfo, void* param_cb);
  int r2_vt_set_dosecsl_cb(r2_vt_htask hTask, dosecsl_callback fun_cb, void* param_cb);
  
  //dirty_cb
  typedef bool (*checkdirty_callback)(void* param_cb, float fSlInfo[3]);
  int r2_vt_set_checkdirty_cb(r2_vt_htask hTask, checkdirty_callback fun_cb, void* param_cb);
  
  //data_cb
  typedef int (*getdata_callback)(int nFrmStart, int nFrmEnd, float* pData, void* param_cb);
  int r2_vt_set_getdata_cb(r2_vt_htask hTask, getdata_callback fun_cb, void* param_cb);
  
  //get best score
  float r2_vt_getbestscore_eer(r2_vt_htask hTask, const float** pWavBuff, int iWavLen);
  
  int r2vt_mem_print();
  
#ifdef __cplusplus
};
#endif


#endif /* __r2vt5__zvtapi__ */
