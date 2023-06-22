#ifndef _FE_INTF_H_
#define _FE_INTF_H_

typedef void* HANDLE;

//#define _FINAL_RELEASE
typedef struct FE_HDR {
	int		nFsKHz;			//	����Ƶ�ʣ�ǧ���ȣ�
	int		nFrmMs;			//	֡�������룩
	int		nSftMs;			//	֡�ƣ����룩
	int		nMaxFrm;		//	���֡��
	int		nFtrDim;		//	����ά��
	char	szFtrInfo[256];	//	������Ϣ		
	char	szVersion[256];	//	�汾��Ϣ		
} FE_HDR;

//	������ȡģ��������
typedef enum FE_CODE {
	FE_OK = 0,			//	ִ�гɹ�
	FE_MEM_ERR  = 1,	//	�ڴ�������
	FE_MATH_ERR = 2,	//	��ѧ�������
	FE_OVERFLOW = 3,	//	�ڴ�Խ�����
	FE_FILE_ERR = 4		//	�ļ���������
} FE_CODE;

#ifdef __cplusplus
extern "C" {
#endif
HANDLE OpenFE(char* szWorkDir, FE_HDR& feInfo);
FE_CODE RunFE( HANDLE hFE,short* pWave, int nLen, float*& pFeature,
			  int*& pSNR, int& nFrm, bool bComp);
//FE_CODE Run1FE( HANDLE hFE, short* pWave, float*& pFeature );
FE_CODE CloseFE( HANDLE hFE );
FE_CODE WarpFE( HANDLE hFE,float Alpha );
FE_CODE ResetFE( HANDLE hFE );
FE_CODE ContinueFE(HANDLE hFE, float* pWave, int nLen);
FE_CODE NormDiffFE(HANDLE hFE, float*& pFeature, int*& pSNR, int& nFrm, bool bEndflag);
#ifdef __cplusplus
}
#endif
////	������ȡģ���ʼ������������ռ䣬���� FE_OK ��ʾ�ɹ�
//typedef HANDLE (*lpOpenFE)(	char*	szWorkDir,	//	���룺����Ŀ¼
//							FE_HDR&	feInfo		//	�����ģ����Ϣ
//							);
//
////	������ȡ���������� FE_OK ��ʾ�ɹ�
//typedef FE_CODE (*lpRunFE)(HANDLE	hFE,			//  �������
//						   short*	pWave,			//	��������
//						   int		nLen,			//	��������
//						   float*&	pFeature,		//	��������
//						   int*&	pSNR,			//	snr ��Ϣ
//						   int&		nFrm,			//	֡��
//						   bool		bComp			//	����
//						   );
//
////	������ȡģ���˳�����������ռ䣬���� FE_OK ��ʾ�ɹ�
//typedef FE_CODE (*lpCloseFE)( HANDLE hFE );
//
////new interface for VTLN Training
//
//typedef FE_CODE (*lpWarpFE)( HANDLE hFE,float Alpha );
//
////һ���µ�������ʼǰ����֤�ڴ����
//typedef FE_CODE (*lpResetFE)( HANDLE hFE );
//
////������������
//typedef FE_CODE (*lpContinueFE)(HANDLE	hFE,
//								short*	pWave, 
//								int		nLen 
//								);
////�����������
//typedef FE_CODE (*lpNormDiffFE)(HANDLE		hFE, 
//								float*&	pFeature, 
//								int*&		pSNR, 
//								int&		nFrm, 
//								bool		bEndflag 
//									 );
//

#endif
