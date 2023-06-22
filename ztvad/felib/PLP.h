/****************************************************************** 
**  File name    : PLP.H
**  Created by   : Mu Su (msu@hitic.ia.ac.cn)
**  Created date : 2005 / 08 / 28
**  Adapted		 : Front End using Mel-Filter-Bank-based
				   PLP Front End, suitable for 8K and 16K
				   with multiple Frame Length and Shift,
				   extended for multi-thread use and VTLN
**  Description  : Declarations of PLP
**  Copyright (C): 2005 - 2006
**  Version      : 1.0
******************************************************************/ 

#ifndef _MF_PLP_H
#define _MF_PLP_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "FloatFFT.h"
#include "fftwrapper.h"
#include "Pitch.h"

//#define _SAM_6K
//#define _SAM_8K
//#define _SAM_16K
#define _SAM_16K8K
//define 8K or 16K only by switching _SAM_8K and _SAM_16K

#if defined (_SAM_6K) && !defined (_SAM_8K) && !defined (_SAM_16K)
#define	FS_KHZ			6
#define CHS_NUM			21
#endif

#if defined (_SAM_8K) && !defined (_SAM_6K) && !defined (_SAM_16K)
#define	FS_KHZ			8
#define CHS_NUM			23
#endif


#if defined (_SAM_16K) && !defined (_SAM_6K) && !defined (_SAM_8K)
#define FS_KHZ			16
#define CHS_NUM			26
#endif

#if defined (_SAM_16K8K)
#define FS_KHZ			16
#define CHS_NUM			22
#define CHS_NUM_W		7
#endif

#ifdef PI
#undef PI                /* PI is defined in Linux */
#endif
#define PI				3.14159265358979
#define TPI				6.28318530717959
#ifndef FLT_MAX
#define FLT_MAX         3.402823466e+38F        /* max value */
#endif

#define SFT_MS			10	//Default value of frame shift(Mili Second)
#define FRM_MS			25  //Default value of frame length(Mili Second)

#define LEFT_CONTEXT	1
#define RIGHT_CONTEXT	1
#define CONTEXT_SPAN	(LEFT_CONTEXT+RIGHT_CONTEXT+1)

//#define LPC_DIM			12//17
#define CEP_DIM			(CHS_NUM+CHS_NUM_W)
#define VEC_DIM			CEP_DIM
#define FUL_DIM			((VEC_DIM*3)*CONTEXT_SPAN)//42//42
#define ALN_FUL_DIM		((FUL_DIM+3)/4*4)//42//42
#define MAX_SPEECH		6000	// ms
#define MIN_SPEECH		30
#define PRE_EMPH		0.97
#define SUM_SPAN		100	// save Sum_Mu per 100 frames (1s)
#define SUM_SIZE		120	// max norm history must less 120s

typedef struct FBankInfo{
	bool	bUsePower;
	int		nChans;	        /* number of channels */
	int		nFsKHz;			/* sample Frequency in KHz */
	int		nfftN;          /* fft size */
	int		klo,khi;        /* lopass to hipass cut-off fft indices */
	float*	cf;				/* array[1..pOrder+1] of centre freqs */
	float*	loWt;			/* array[1..fftN/2] of loChan weighting */
	short*	loChan;			/* array[1..fftN/2] of loChan index */
	int		wklo,wkhi;        /* lopass to hipass cut-off fft indices */
	float*	cfh;				/* array[1..pOrder+1] of centre freqs */
	float*	loWtH;			/* array[1..fftN/2] of loChan weighting */
	short*	loChanH;			/* array[1..fftN/2] of loChan index */
	FBankInfo(){ cfh = cf = NULL;loWtH = loWt = NULL;loChanH = loChan = NULL;bUsePower = false; }
	~FBankInfo(){ if( cf ) delete []cf; if( loWt ) delete []loWt; if( cfh ) delete []cfh; if( loWtH ) delete []loWtH; \
				  if( loChan ) delete []loChan; if( loChanH ) delete []loChanH; }
}FBankInfo;

class PLP  
{
public:
	PLP( int nFrmMs = FRM_MS,int nSftMs = SFT_MS, bool bNarrow=false );
	int		InitPar( char* szFEDir,float LoPass = -1,float HiPass = -1, \
					float WLoPass = -1,float WHiPass = -1, \
					bool bUsePow = true, \
					int nFirstBuf = 150, int nIncBuf = 50, \
					int nNormDelay = 0, int nNormHist = 1000, int nDelayOff = 2000, int nPreAvgWeight = 0);
	int		GetBuf( short* pBuf,int iSam );
	//int		ResetFB( float Alpha );

	
	virtual ~PLP();

	bool	m_bCVN;

private:
	void	WeightCep( float* Cepstrum );
	void	RemoveBias( short* pBuf,int iSam );
	void	PreEmph( float* pBuf,int Len );
	void	Dither( float* waveform,int Len , float dither_value);

	int		InitFB( float LoPass,float HiPass, float WLoPass,float WHiPass );
	float	IDFT( float* ASpectrum,float* AutoCorr );
	float	Auto2LPC( float* AutoCorr,float* LPC,float E );
	void	LPC2Cep( float* LPC,float* Cep );
	inline	float MEL( int k,float fres );
	inline  float MEL( float Hz );	
	inline	float HZ( float Mel );
	int		m_nFrmLen;
	int		m_nSftLen;
	bool		m_UsePitch;
	inline	float RandUniform() {  // random between 0 and 1.
	  return (float)((rand() + 1.0) / (RAND_MAX+2.0)); 
	}
	
	inline float RandGauss() {
	  return (float)(sqrt (-2 * log(RandUniform()))
	                            * cos(2*PI*RandUniform()));
	}


	FBankInfo	FBank;
	CFloatFFT  * m_pFFT;
	FFTWrapper * m_pFFTWrapper;
//	float*		m_pEqLCurv;
	float*		m_pHamWin;
//	double**	m_pIDFTMat;
	int			m_pSNR[MAX_SPEECH];
	//CPitchX*		m_PX;
	int			m_nFrame;
	int			m_Lifter;
	int			m_normCnt;
//	int			m_unpitchAcced;
	bool		m_bNarrow;
//	CF0*		m_PX;

	
//On-line Parameter
	void	PostProc( int nFrame );
	void	Norm( int nFrame, bool bEnd = false);
	//void	DifFE( int nFrame );
//VTLN Extension
	float	WarpFreq( float fcl,float fcu,float freq,float Alpha );
	float		m_MelLo;
	float		m_MelHi;
	float		m_WMelLo;
	float		m_WMelHi;
	float		m_LoPass;
	float		m_hiPass;
	float		m_WLoPass;
	float		m_WhiPass;
	float		m_LoWarpCut;
	float		m_HiWarpCut;

	int			m_nFirstBuf;
	int			m_nIncBuf;
//	int			m_nPitchDelay;
//	int			m_nPitchHistory;
	int			m_nNormDelay;
	int			m_nNormHistory;
	int			m_nDelayOff;
public:
	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	void Resume();
	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	int AddBuf( float* pBuf,int iSamples );
	//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
//	int Norm_Diff( bool bEndflag );
	int Norm_Diff2( bool bEndflag );
	//锟斤拷锟斤拷锟斤拷锟斤拷
	void GetFE( float*& pFe,int*& pSNR,int& nFrame);
private:
	float		m_pBuf[MAX_SPEECH*FS_KHZ*SFT_MS+FS_KHZ*FRM_MS];
	int			m_iSamples;
	int			m_frmOffset;
//	int			m_ipitchSamples;
	//int			p_nframe;   //pitch锟斤拷锟斤拷锟斤拷锟斤拷,锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
//	int			p_len;      //pitch��伴��椴������㈠��1000ms
	float		Sum_Mu[VEC_DIM];
	float		Sum_Va[VEC_DIM];
	float		Sum_Mu1[VEC_DIM];
	float		Sum_Va1[VEC_DIM];
	float		Sum_Mu2[VEC_DIM];
	float		Sum_Va2[VEC_DIM];
	float		Pot_Sum_Mu [SUM_SIZE*VEC_DIM];
	float		Pot_Sum_Va [SUM_SIZE*VEC_DIM];
	float		Pot_Sum_Mu1[SUM_SIZE*VEC_DIM];
	float		Pot_Sum_Va1[SUM_SIZE*VEC_DIM];
	float		Pot_Sum_Mu2[SUM_SIZE*VEC_DIM];
	float		Pot_Sum_Va2[SUM_SIZE*VEC_DIM];
	float		m_Feature[MAX_SPEECH*ALN_FUL_DIM];
//	float		m_tmpF0[MAX_SPEECH];
	float		m_pBaseFea[(MAX_SPEECH+CONTEXT_SPAN)*VEC_DIM];
	float		m_pDeltFea[(MAX_SPEECH+CONTEXT_SPAN)*VEC_DIM];
	float		m_pAcceFea[(MAX_SPEECH+CONTEXT_SPAN)*VEC_DIM];
	int			m_nbaseFrame;
	int			m_meanFeatureIdx; //锟斤拷CMVN锟斤拷锟斤拷锟斤拷锟斤拷
	int			m_varFeatureIdx; //锟斤拷CMVN锟斤拷锟斤拷锟斤拷锟斤拷
	int			m_normedFeatureIdx; //锟斤拷CMVN锟斤拷锟斤拷锟斤拷锟斤拷
//	int			m_PitchIdx;   //Pitch��伴����㈠����ら��褰����
//	int			m_startPitchIdx;	//��ㄧ����������よ癌pitch纰������������㈠����伴��
	int			m_OutIdx;     //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	float		m_LDAmat[VEC_DIM*CONTEXT_SPAN*FUL_DIM]; //LDA&MLLR锟斤拷锟斤拷锟斤拷锟斤拷
	int			m_LDAmatIdx; //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	//bool		m_frontLDA;  //锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷

	float		Static_Sum_Mu[VEC_DIM];
	float		Static_Sum_Va[VEC_DIM];
	float		Static_Sum_Mu1[VEC_DIM];
	float		Static_Sum_Va1[VEC_DIM];
	float		Static_Sum_Mu2[VEC_DIM];
	float		Static_Sum_Va2[VEC_DIM];
	unsigned		Static_Frm_num;

	float* FrameBuf;
	float* Real;		
	float* Imag;		
	float* FBOut;	
//	float* ASpec;	

};

#endif // _XPLP_H
