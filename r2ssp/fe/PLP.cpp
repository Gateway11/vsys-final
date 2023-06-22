/****************************************************************** 
**  File name    : PLP.CPP
**  Created by   : Mu Su (msu@hitic.ia.ac.cn)
**  Created date : 2005 / 08 / 28
**  Adapted		 : Front End using Mel-Filter-Bank-based
				   PLP Front End, suitable for 8K and 16K
				   with multiple Frame Length and Shift,
				   extended for multi-thread use and VTLN
**  Description  : Implementation of PLP
**  Copyright (C): 2005 - 2006
**  Version      : 1.0
******************************************************************/ 

//#include "stdafx.h"
#include "PLP.h"
#include "math.h"
//#include "assert.h"
#include "AllocSpace.h"
#include "string.h"
//#include "MyDebug.h"
//#include "pitch.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PLP::PLP( int nFrmMs /* = FRM_MS */,int nSftMs /* = SFT_MS  */, bool bNarrow): 
m_bNarrow( bNarrow ), 
m_pFFT( NULL ), 
//m_pEqLCurv( NULL ), 
//m_pIDFTMat( NULL ), 
m_pHamWin( NULL ), 
m_frmOffset( 0 ), 
//m_PX( NULL ),
m_Lifter( -1 )
{
	if (m_bNarrow) {
		m_nFrmLen = FS_KHZ/2*nFrmMs;
		m_nSftLen = FS_KHZ/2*nSftMs;
	} else {
		m_nFrmLen = FS_KHZ*nFrmMs;
		m_nSftLen = FS_KHZ*nSftMs;
	}
	//p_len	  = FS_KHZ*1e3;     //ÿ������1s����
	m_bCVN = true;
}

PLP::~PLP()
{
	if( m_pFFT ){
		delete m_pFFT;
		m_pFFT = NULL;
	}
//	if( m_pEqLCurv ){
//		delete []m_pEqLCurv;
//		m_pEqLCurv = NULL;
//	}
//	if( m_pIDFTMat ){
//		Free2d( (char** )m_pIDFTMat );
//		m_pIDFTMat = NULL;
//	}
	if( m_pHamWin ){
		delete []m_pHamWin;
		m_pHamWin = NULL;
	}
//	if( m_PX ){
//		delete m_PX;
//		m_PX = NULL;
//	}
	if( FrameBuf ){
		delete []FrameBuf;
		FrameBuf = NULL;
	}
	if( Real ){
		delete []Real;
		Real = NULL;
	}
	if( Imag ){
		delete []Imag;
		Imag = NULL;
	}
	if( FBOut ){
		delete []FBOut;
		FBOut = NULL;
	}
//	if( ASpec ){
//		delete []ASpec;			
//		ASpec = NULL;
//	}

}

int PLP::InitPar(	char* szFEDir,float LoPass /* = -1 */,float HiPass /* = -1 */, \
					float WLoPass /* = -1 */,float WHiPass /* = -1 */, \
					bool bUsePow /* = true */, bool bUseDither, \
					int nFirstBuf, int nIncBuf, \
					int nNormDelay, int nNormHist, int nDelayOff, int nPreAvgWeight)
{
	if (m_bNarrow) {
		FBank.nFsKHz = FS_KHZ/2;
	} else {
		FBank.nFsKHz = FS_KHZ;
	}
	FBank.nChans = CHS_NUM + CHS_NUM_W;
	FBank.nfftN = 2;
	FBank.bUsePower = bUsePow;

	m_nFirstBuf = nFirstBuf;
	m_nIncBuf = nIncBuf;
	m_nNormHistory = nNormHist;
	m_nNormDelay = nNormDelay;
	m_nDelayOff = nDelayOff;
	m_bUseDither = bUseDither;
	Static_Frm_num = nPreAvgWeight;
	//printf("m_nNullPitchDelay:%d\n", m_nNullPitchDelay);

	while( FBank.nfftN<m_nFrmLen )	FBank.nfftN *= 2;
	//if (m_bNarrow) FBank.nfftN *= 2;
	
	m_pFFT = new FFTWrapper( FBank.nfftN );
	if( m_pFFT==NULL ){
		printf( "FFT_SSE Init Failed!\n" );
		return -1;
	}
	InitFB( LoPass,HiPass,WLoPass,WHiPass );
	
	float a = TPI/(m_nFrmLen - 1);
	m_pHamWin = new float[m_nFrmLen];
	for( int i=0;i<m_nFrmLen;i++ )
		m_pHamWin[i] = 0.54 - 0.46 * cos(a*i);

	//m_PX = new CPitchX( m_nSftLen/FS_KHZ,FS_KHZ );
	//m_PX = new CPitchX;
//	m_PX = new CF0;

	//LDA&MLLR����
	//memset(m_LDAmat,'\0',sizeof(float)*CONTEXT_SPAN*VEC_DIM*FUL_DIM);
	//char ldaDir[256];
	//strcpy(ldaDir,szFEDir);
	//strcat(ldaDir,"LDA.dat");
	//FILE *pf = fopen(ldaDir,"rb");
	//if (pf==NULL)
	//{
	//	printf("open LDA.dat Fail!\n");
	//	return -1;
	//}
	//fread(m_LDAmat,sizeof(float),CONTEXT_SPAN*VEC_DIM*FUL_DIM,pf);
	//fclose(pf);

	char avgDir[256];
	strcpy(avgDir,szFEDir);
	strcat(avgDir,"avg.dat");
	FILE *pfavg = fopen(avgDir,"rt");
	printf("...load avg.dat %s\n", avgDir);
	if (pfavg==NULL)
	{
		Static_Frm_num = 0;
		printf("open avg.dat Fail!\n");
	} else {
		for (int i=0; i<VEC_DIM; i++)
			fscanf(pfavg, "%f ", &Static_Sum_Mu[i]);
		for (int i=0; i<VEC_DIM; i++)
			fscanf(pfavg, "%f ", &Static_Sum_Mu1[i]);
		for (int i=0; i<VEC_DIM; i++)
			fscanf(pfavg, "%f ", &Static_Sum_Mu2[i]);
		for (int i=0; i<VEC_DIM; i++)
			fscanf(pfavg, "%f ", &Static_Sum_Va[i]);
		for (int i=0; i<VEC_DIM; i++)
			fscanf(pfavg, "%f ", &Static_Sum_Va1[i]);
		for (int i=0; i<VEC_DIM; i++)
			fscanf(pfavg, "%f ", &Static_Sum_Va2[i]);
		fclose(pfavg);
	}
#ifdef _WRITESYS
	Static_Frm_num = 0;
	Static_Frm_num2 = 0;
	memset(Static_Sum_Mu,'\0',sizeof(float)*(VEC_DIM));
	memset(Static_Sum_Va,'\0',sizeof(float)*(VEC_DIM));
	memset(Static_Sum_Mu1,'\0',sizeof(float)*(VEC_DIM));
	memset(Static_Sum_Va1,'\0',sizeof(float)*(VEC_DIM));
	memset(Static_Sum_Mu2,'\0',sizeof(float)*(VEC_DIM));
	memset(Static_Sum_Va2,'\0',sizeof(float)*(VEC_DIM));
	printf("Write cep sys info\n");
#endif
#ifdef _DEBUG
	for (int i=0; i<VEC_DIM; i++)
		printf("%lg\t", Static_Sum_Mu[i]);
	printf("\n");
	for (int i=0; i<VEC_DIM; i++)
		printf("%lg\t", Static_Sum_Mu1[i]);
	printf("\n");
	for (int i=0; i<VEC_DIM; i++)
		printf("%lg\t", Static_Sum_Mu2[i]);
	printf("\n");
	for (int i=0; i<VEC_DIM; i++)
		printf("%lg\t", Static_Sum_Va[i]);
	printf("\n");
	for (int i=0; i<VEC_DIM; i++)
		printf("%lg\t", Static_Sum_Va1[i]);
	printf("\n");
	for (int i=0; i<VEC_DIM; i++)
		printf("%lg\t", Static_Sum_Va2[i]);
	printf("\n");
#endif

	FrameBuf = new float[m_nFrmLen];
	Real	= new float[FBank.nfftN];
	Imag	= new float[FBank.nfftN];
	FBOut	= new float[FBank.nChans+1];
	//ASpec	= new float[FBank.nChans+3];

	//Static_Frm_num = 0;

	return 0;
}

void	PLP::Dither( float* waveform,int Len , float dither_value) {
	for (int i = 0; i < Len; i++)
		waveform[i] += RandGauss() * dither_value;
}
	
int PLP::InitFB( float LoPass,float HiPass, float WLoPass,float WHiPass )
{
	FBank.klo = 2;
	FBank.khi = FBank.nfftN/2;
	float Fres = FBank.nFsKHz*1000.f/(FBank.nfftN*700);
	m_LoPass = LoPass;
	m_hiPass = HiPass;
	m_MelLo = 0;
	m_MelHi = MEL( FBank.khi+1,Fres );
	if( LoPass>=0.0f ){
		m_MelLo		= MEL(LoPass);
//		FBank.klo	= (int)(LoPass*FBank.nfftN/(FS_KHZ*1000)+2.5);    //+0.5
		FBank.klo	= (int)(LoPass*FBank.nfftN/(FBank.nFsKHz*1000)+0.5);    //+0.5
		if( FBank.klo<2 )	FBank.klo = 2;
	}
	if( HiPass>=0.0f ){
		m_MelHi		= MEL(HiPass);
		FBank.khi	= (int)(HiPass*FBank.nfftN/(FBank.nFsKHz*1000)+0.5);
		if( FBank.khi>FBank.nfftN/2 )	FBank.khi = FBank.nfftN/2;
	}

	
	float MelSpan = (m_MelHi-m_MelLo);
	int maxChans = CHS_NUM+1;
	
	FBank.cf = new float[CHS_NUM+2];
	if( FBank.cf==NULL ){
		printf( "FBank.cf Alloc Failed!\n" );
		return -1;
	}

	int i;
	for( i=0;i<=CHS_NUM+1;i++ ){
		FBank.cf[i] = i*MelSpan/maxChans+m_MelLo;
	}
	
	
	if (!m_bNarrow) {
		m_WMelLo = MEL(WLoPass);
		m_WMelHi = MEL(WHiPass);
		float MelSpanH = (m_WMelHi-m_WMelLo);
		int maxChansH = CHS_NUM_W;
		FBank.cfh = new float[CHS_NUM_W+2];
		if( FBank.cfh==NULL ){
			printf( "FBank.cfh Alloc Failed!\n" );
			return -1;
		}
		for( i=0;i<=CHS_NUM_W+1;i++ ){
			FBank.cfh[i] = (i-1)*MelSpanH/maxChansH+m_WMelLo;
		}
		m_WMelLo = FBank.cfh[0];
		WLoPass = HZ(m_WMelLo);
		FBank.wklo	= (int)(WLoPass*FBank.nfftN/(FBank.nFsKHz*1000)+0.5);    //+0.5
		if( FBank.wklo<2 )	FBank.wklo = 2;
		FBank.wkhi	= (int)(WHiPass*FBank.nfftN/(FBank.nFsKHz*1000)+0.5);
		if( FBank.wkhi>FBank.nfftN/2 )	FBank.wkhi = FBank.nfftN/2;
	}

/*-----------------FFT Bin to Channel Mapping-------------------*/
	float MelK;
	FBank.loChan = new short[FBank.nfftN/2+1];
	if( FBank.loChan==NULL ){
		printf( "FBank.loChan Alloc Failed!\n" );
		return -1;
	}
//	int chan = 1;          //�˴��ĳ� int chan = 0;    ǰ��� FBank.klo	����Ͳ��ö�� 2 �ˣ�
	int chan = 0;
	FBank.loChan[0] = -1;
	for( i=1;i<=FBank.nfftN/2;i++ ){
		MelK = MEL( i,Fres );
		if( i<FBank.klo||i>FBank.khi )	FBank.loChan[i] = -1;
		else{
			while( FBank.cf[chan]<MelK && chan<=maxChans )	chan++;
			FBank.loChan[i] = chan-1;
		}
		
	}


	if (!m_bNarrow) {
		FBank.loChanH = new short[FBank.nfftN/2+1];
		if( FBank.loChanH==NULL ){
			printf( "FBank.loChan Alloc Failed!\n" );
			return -1;
		}
	//	int chan = 1;          //�˴��ĳ� int chan = 0;    ǰ��� FBank.klo	����Ͳ��ö�� 2 �ˣ�
		chan = 0;
		FBank.loChanH[0] = -1;
		for( i=1;i<=FBank.nfftN/2;i++ ){
			MelK = MEL( i,Fres );
			if( i<FBank.wklo||i>FBank.wkhi )	FBank.loChanH[i] = -1;
			else{
				while( FBank.cfh[chan]<MelK && chan<=CHS_NUM_W+1 )	chan++;
				FBank.loChanH[i] = chan-1;
			}
		}
	}

/*----------------Channel Weight Coefficients--------------------*/
	FBank.loWt = new float[FBank.nfftN/2+1];
	if( FBank.loWt==NULL ){
		printf( "FBank.loWt Alloc Failed!\n" );
		return -1;
	}
	FBank.loWt[0] = 0.0f;
	for( i=1;i<=FBank.nfftN/2;i++ ){
		chan = FBank.loChan[i];
		/*
		if( i<FBank.klo||i>FBank.khi )	FBank.loWt[i] = 0.0f;
		else{
			if( chan>0 )		// �˴��������ֻ������һ����֧
				FBank.loWt[i] = (FBank.cf[chan+1]-MEL( i,Fres ))/(FBank.cf[chan+1]-FBank.cf[chan]);
			else{
//				assert( chan==0 );
				FBank.loWt[i] = (FBank.cf[1]-MEL( i,Fres ))/(FBank.cf[1]-m_MelLo);
			}
		}
		//*/
		if( chan < 0 )	FBank.loWt[i] = 0.0f;
		else{
			FBank.loWt[i] = (FBank.cf[chan+1]-MEL( i,Fres ))/(FBank.cf[chan+1]-FBank.cf[chan]);
		}
	}

	if (!m_bNarrow) {
		FBank.loWtH = new float[FBank.nfftN/2+1];
		if( FBank.loWtH==NULL ){
			printf( "FBank.loWtH Alloc Failed!\n" );
			return -1;
		}
		FBank.loWtH[0] = 0.0f;
		for( i=1;i<=FBank.nfftN/2;i++ ){
			chan = FBank.loChanH[i];
			if( chan < 0 )	FBank.loWtH[i] = 0.0f;
			else{
				FBank.loWtH[i] = (FBank.cfh[chan+1]-MEL( i,Fres ))/(FBank.cfh[chan+1]-FBank.cfh[chan]);
			}
		}
#ifdef _DEBUG
	printf("FBank.loWt\tFBank.lowtH:\n");
	for (int tt=0; tt<=FBank.nfftN/2; tt++) {
		printf("%f\t%f\n", FBank.loWt[tt], FBank.loWtH[tt]);
	}
#endif
	}
///*--------------Equal Loudness Curve--------------*/
//	m_pEqLCurv = new float[maxChans];
//	if( m_pEqLCurv==NULL ){
//		printf( "m_pEqLCurv Alloc Failed!\n" );
//		return -1;		
//	}
//
//	float Hz_Cf = 0.0f;
//	float fsub, fsq;
//	
//	for( chan=0;chan<maxChans;chan++ ){
//		Hz_Cf = HZ( FBank.cf[chan] );
//	    fsq = (Hz_Cf * Hz_Cf);
//		fsub = fsq / (fsq + 1.6e5);
//		m_pEqLCurv[chan] = fsub * fsub * ((fsq + 1.44e6)  /(fsq + 9.61e6));
//			  
//	}
//
///*----------------IDFT Matrix for Autocorrelation Computing--------------*/
//	
//	m_pIDFTMat = (double** )Alloc2d( LPC_DIM+1,maxChans+1,sizeof(double) );
//	if( m_pIDFTMat==NULL ){
//		printf( "m_pIDFTMat Alloc Failed!\n" );
//		return -1;
//	}
//	
//	float BaseAng = PI/maxChans;
//	
//	for( i=0;i<=LPC_DIM;i++ ){
//		m_pIDFTMat[i][0] = 1.0f;
//		for( chan = 1;chan<maxChans;chan++ )
//			m_pIDFTMat[i][chan] = 2.0 * cos(BaseAng * (double)i * (double)chan);
//		m_pIDFTMat[i][maxChans] = cos(BaseAng * (double)i * (double)maxChans);
//	}
	
	
	return 0;
}

float PLP::MEL( int k,float fres )
{
	return 1127 * log(1 + (k-1)*fres);	
}

float PLP::HZ( float Mel )
{
	return 700*(exp(Mel/1127)-1);
}

void PLP::PreEmph( float* pBuf,int Len )
{
	for( int i=Len-1;i>=1;i-- )
		pBuf[i] -= pBuf[i-1]*PRE_EMPH;

	pBuf[0] *= ( 1.0f-PRE_EMPH );
}

//float PLP::IDFT( float* ASpectrum,float* AutoCorr )
//{
//   double acc;
//   float E;
//   int nAuto, nFreq;
//   int i, j;
//
//   nFreq = FBank.nChans+2;
//   nAuto = LPC_DIM+1;
//
//   for (i=0; i<nAuto; i++) {
//      acc = m_pIDFTMat[i][0] * (double)ASpectrum[1];
//      for (j=1; j<nFreq; j++)
//         acc += m_pIDFTMat[i][j] * (double)ASpectrum[j+1];
//
//      if (i>0) 
//         AutoCorr[i] = (float)(acc / (double)(2.0 * (nFreq-1)));
//      else  
//         E = (float)(acc / (double)(2.0 * (nFreq-1)));
//   }     
//   return E; /* Return zero'th auto value separately */
//}

/*		������
void PLP::RemoveBias( short* pBuf,int iSam )
{
	double dSum = 0.0;
	int i;
	for( i=0;i<iSam;i++ )
		dSum += pBuf[i];
	
	
	short Bias = (short)(dSum/iSam);
	for( i=0;i<iSam;i++ )
		pBuf[i] -= Bias;
}
//*/

//*		��ʿ����
void PLP::RemoveBias( short* pBuf,int iSam )
{
	double dSum = 0.0;
	int itmp, i;
	for( i=0;i<iSam;i++ )
		dSum += pBuf[i];
	
	
	int Bias = (int)(dSum/iSam);
	for( i=0;i<iSam;i++ ) {
		itmp = (int)pBuf[i] - Bias;
		if(itmp >= 32766) {
			pBuf[i]= 32766;
		}else if(itmp <= -32766) {
			pBuf[i]= -32766;
		}else {
			pBuf[i]= (short)itmp;
		}
	}
}
//*/


int PLP::GetBuf( short* pBuf,int iSam )
{

//	RemoveBias(pBuf,iSam);//add by wsj

	m_nFrame = (iSam - m_nFrmLen)/m_nSftLen + 1;

	//float* FrameBuf = new float[m_nFrmLen];
	//float* Real		= new float[FBank.nfftN];
	//float* Imag		= new float[FBank.nfftN];
	//float* FBOut	= new float[FBank.nChans+1];
	//float* ASpec	= new float[FBank.nChans+3];
//	float AutoCorr[LPC_DIM+1];
//	float LPC[LPC_DIM+2];
//	float Cep[CEP_DIM+1];
	float PE,Tmp,E;
	int i,j,bin;
	const float melFloor = 1.0f;
	for( i=0;i<m_nFrame;i++ ){
		E = 0.0;
		for( j=0;j<m_nFrmLen;j++ ){
			FrameBuf[j] = (float)pBuf[i*m_nSftLen+j];
		}
		
/*-------------Pre-emphasize by 0.97---------------*/ 
		PreEmph( FrameBuf,m_nFrmLen );

/*----------------Hamming Window------------------*/
		
		for( j=0;j<m_nFrmLen;j++ )
			FrameBuf[j] *= m_pHamWin[j];

/*------------------FFT Operation-------------------*/
		memset( Real,0,sizeof(float)*FBank.nfftN );
		memset( Imag,0,sizeof(float)*FBank.nfftN );
		memcpy( Real,FrameBuf,sizeof(float)*m_nFrmLen );

		m_pFFT->Execute( Real,Imag );

/*------------------Mel Filter Bank-----------------*/
//Attention: FBOut[0] = 0.0f; No Use;

		memset( FBOut,0,sizeof(float)*(FBank.nChans+1) );
		for( j=FBank.klo;j<=FBank.khi;j++ ){
			
			bin = FBank.loChan[j];
			if (bin < 0) continue;
			if( FBank.bUsePower )
				PE = Real[j-1]*Real[j-1]+Imag[j-1]*Imag[j-1];
			else
				PE = (float)sqrt(Real[j-1]*Real[j-1]+Imag[j-1]*Imag[j-1]);

			Tmp = FBank.loWt[j]*PE;
			if( bin>0 )
				FBOut[bin] += Tmp;
			if( bin<CHS_NUM )
				FBOut[bin+1] += (PE-Tmp);
			
		}
		if (!m_bNarrow) {
			for( j=FBank.wklo;j<=FBank.wkhi;j++ ){
				
				bin = FBank.loChanH[j];
				if (bin < 0) continue;
				if( FBank.bUsePower )
					PE = Real[j-1]*Real[j-1]+Imag[j-1]*Imag[j-1];
				else
					PE = (float)sqrt(Real[j-1]*Real[j-1]+Imag[j-1]*Imag[j-1]);
  	
				Tmp = FBank.loWtH[j]*PE;
				if( bin>0 )
					FBOut[bin+CHS_NUM] += Tmp;
				if( bin<CHS_NUM_W )
					FBOut[bin+CHS_NUM+1] += (PE-Tmp);
				
			}
		}
		
		printf("%d\t", i);
		for( j=1;j<=CEP_DIM;j++ ) {
			m_pBaseFea[(i+LEFT_CONTEXT)*CEP_DIM+j-1] = log(FBOut[j]);
			printf("%.6f ", m_pBaseFea[(i+LEFT_CONTEXT)*CEP_DIM+j-1]);
		}
		printf("\n");


		
	}

	return 0;
}

void PLP::WeightCep( float* Cepstrum )
{
	//float CepWin[CEP_DIM+1];
	float a, Lby2;
	a = PI/m_Lifter;
	Lby2 = m_Lifter/2;
	
	for( int i=1;i<=CEP_DIM;i++ )
		Cepstrum[i] *= 1.0 + Lby2*sin(i * a);
}

void PLP::GetFE( float*& pFe,int*& pSNR,int& nFrame )
{
	nFrame = m_OutIdx;
	pFe = m_Feature-m_frmOffset*ALN_FUL_DIM;
	pSNR = m_pSNR;
}



float PLP::MEL( float Hz )
{
	return 1127*log( 1+Hz/700 );
}

float PLP::WarpFreq( float fcl,float fcu,float freq,float Alpha )
{
   if (fabs(Alpha-1.0)<1e-3)
      return freq;
   
   float scale = 1.0 / Alpha;
   float cu = fcu * 2 / (1 + scale);
   float cl = fcl * 2 / (1 + scale);
   
   float au = (m_hiPass - cu * scale) / (m_hiPass - cu);
   float al = (cl * scale - m_LoPass) / (cl - m_LoPass);
   
   if (freq > cu)
	   return  au * (freq - cu) + scale * cu ;
   else if (freq < cl)
	   return al * (freq - m_LoPass) + m_LoPass ;
   else
	   return scale * freq ;
	
}

void PLP::PostProc( int nFrame )
{
	Norm( nFrame );
}

void PLP::Norm( int nFrame, bool bEnd )
{
	float Mu[VEC_DIM];
	float Va[VEC_DIM];
	float Mu1[VEC_DIM];
	float Va1[VEC_DIM];
	float Mu2[VEC_DIM];
	float Va2[VEC_DIM];
	float Tmp;
	//int Cnt = 0;
	int i;
	//memset( Mu,0,sizeof(float)*(VEC_DIM) );
	//memset( Va,0,sizeof(float)*(VEC_DIM) );
	//memset( Mu1,0,sizeof(float)*(VEC_DIM) );
	//memset( Va1,0,sizeof(float)*(VEC_DIM) );
	//memset( Mu2,0,sizeof(float)*(VEC_DIM) );
	//memset( Va2,0,sizeof(float)*(VEC_DIM) );
	if (!bEnd)
		nFrame -= 4;

	for( i=m_meanFeatureIdx;i<nFrame;i++ ){
		// 		if( m_pSNR[i]==-1 )
		// 			continue;

		if (i%SUM_SPAN==0) {
			int idx = i/SUM_SPAN;
			idx %= SUM_SIZE;
			memcpy(Pot_Sum_Mu +idx*VEC_DIM, Sum_Mu, sizeof(float)*VEC_DIM);
			memcpy(Pot_Sum_Mu1+idx*VEC_DIM, Sum_Mu1, sizeof(float)*VEC_DIM);
			memcpy(Pot_Sum_Mu2+idx*VEC_DIM, Sum_Mu2, sizeof(float)*VEC_DIM);
		}
		for( int j=0;j<VEC_DIM;j++ ){
			Sum_Mu[j] += m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j];
			Sum_Mu1[j] += m_pDeltFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j];
			Sum_Mu2[j] += m_pAcceFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j];
		}
		m_normCnt++;
	}
	m_meanFeatureIdx = i;

#ifdef _WRITESYS
	if (bEnd) {
		for( int j=0;j<VEC_DIM;j++ ){
			Static_Sum_Mu[j] += Sum_Mu[j];
			Static_Sum_Mu1[j] += Sum_Mu1[j];
			Static_Sum_Mu2[j] += Sum_Mu2[j];
		}
		Static_Frm_num2 += m_normCnt;
		printf("%d\n", Static_Frm_num2);
	}
#endif

	//m_normCnt += m_FeatureIdx;   //�˴��Ǹ������������Pitch���������Σ���ôcntҪ���ȫ�ֵ�!!!!!!!!!!!!!!!

	if( m_normCnt>0 ){
		int nNormSfrm = nFrame - m_nNormHistory;
		if (nNormSfrm < 0) nNormSfrm = 0;
		int nPot = nNormSfrm/SUM_SPAN;
		int nNormCnt = nFrame - nPot*SUM_SPAN;
		nPot %= SUM_SIZE;
#ifdef _DEBUG
		printf("nPot :\t%d\nnNormCnt :\t%d\nnFrame :\t%d\nStatic_Frm_num :\t%d\n", nPot, nNormCnt, nFrame, Static_Frm_num);
#endif
		for( i=0;i<VEC_DIM;i++ ) {
			Mu[i]  = (Sum_Mu[i] -Pot_Sum_Mu[nPot*VEC_DIM+i] +Static_Sum_Mu[i] *Static_Frm_num)/(nNormCnt+Static_Frm_num);
			Mu1[i] = (Sum_Mu1[i]-Pot_Sum_Mu1[nPot*VEC_DIM+i]+Static_Sum_Mu1[i]*Static_Frm_num)/(nNormCnt+Static_Frm_num);
			Mu2[i] = (Sum_Mu2[i]-Pot_Sum_Mu2[nPot*VEC_DIM+i]+Static_Sum_Mu2[i]*Static_Frm_num)/(nNormCnt+Static_Frm_num);
			//Mu[i] = (Sum_Mu[i]+Static_Sum_Mu[i]*Static_Frm_num)/(m_normCnt+Static_Frm_num);
			//Mu1[i] = (Sum_Mu1[i]+Static_Sum_Mu1[i]*Static_Frm_num)/(m_normCnt+Static_Frm_num);
			//Mu2[i] = (Sum_Mu2[i]+Static_Sum_Mu2[i]*Static_Frm_num)/(m_normCnt+Static_Frm_num);
		}

		if( m_bCVN ){
			for( i=m_varFeatureIdx; i<nFrame; i++ ){
				// 			if( m_pSNR[i]==-1 )
				// 				continue;
				if (i%SUM_SPAN==0) {
					int idx = i/SUM_SPAN;
					idx %= SUM_SIZE;
					memcpy(Pot_Sum_Va +idx*VEC_DIM, Sum_Va, sizeof(float)*VEC_DIM);
					memcpy(Pot_Sum_Va1+idx*VEC_DIM, Sum_Va1, sizeof(float)*VEC_DIM);
					memcpy(Pot_Sum_Va2+idx*VEC_DIM, Sum_Va2, sizeof(float)*VEC_DIM);
				}
				for( int j=0; j<VEC_DIM; j++ ){
					Sum_Va[j] += m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] * m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j];
					Sum_Va1[j] += m_pDeltFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] * m_pDeltFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j];
					Sum_Va2[j] += m_pAcceFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] * m_pAcceFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j];
					//Tmp = m_pBaseFea[(i+LEFT_CONTEXT)*VEC_DIM+j] - Mu[j];
					//Tmp *= Tmp;
					//Sum_Va[j] += Tmp;
					//Tmp = m_pDeltFea[(i+LEFT_CONTEXT)*VEC_DIM+j] - Mu1[j];
					//Tmp *= Tmp;
					//Sum_Va1[j] += Tmp;
					//Tmp = m_pAcceFea[(i+LEFT_CONTEXT)*VEC_DIM+j] - Mu2[j];
					//Tmp *= Tmp;
					//Sum_Va2[j] += Tmp;
				}			
			}
			m_varFeatureIdx = i;
			for( i=0;i<VEC_DIM;i++ ){
				Va[i] = (Sum_Va[i]-Pot_Sum_Va[nPot*VEC_DIM+i]+Static_Sum_Va[i]*Static_Frm_num)/(nNormCnt+Static_Frm_num) - Mu[i]*Mu[i];
				//Va[i] = (Sum_Va[i]+Static_Sum_Va[i]*Static_Frm_num)/(m_normCnt+Static_Frm_num) - Mu[i]*Mu[i];
				Va[i] = sqrt( Va[i] );
				if( Va[i]<0.001f )	Va[i] = 0.001f;
				Va1[i] = (Sum_Va1[i]-Pot_Sum_Va1[nPot*VEC_DIM+i]+Static_Sum_Va1[i]*Static_Frm_num)/(nNormCnt+Static_Frm_num) - Mu1[i]*Mu1[i];
				//Va1[i] = (Sum_Va1[i]+Static_Sum_Va1[i]*Static_Frm_num)/(m_normCnt+Static_Frm_num) - Mu1[i]*Mu1[i];
				Va1[i] = sqrt( Va1[i] );
				if( Va1[i]<0.001f )	Va1[i] = 0.001f;
				Va2[i] = (Sum_Va2[i]-Pot_Sum_Va2[nPot*VEC_DIM+i]+Static_Sum_Va2[i]*Static_Frm_num)/(nNormCnt+Static_Frm_num) - Mu2[i]*Mu2[i];
				//Va2[i] = (Sum_Va2[i]+Static_Sum_Va2[i]*Static_Frm_num)/(m_normCnt+Static_Frm_num) - Mu2[i]*Mu2[i];
				Va2[i] = sqrt( Va2[i] );
				if( Va2[i]<0.001f )	Va2[i] = 0.001f;
#ifdef _DEBUG
				printf ("%.6f %.6f, ", Mu[i], Va[i]);
				printf ("%.6f %.6f, ", Mu1[i], Va1[i]);
				printf ("%.6f %.6f, %.6f\n", Mu2[i], Va2[i], Sum_Mu[i]);
#endif
			}
#ifdef _WRITESYS
			if (bEnd) {
				for( int j=0;j<VEC_DIM;j++ ){
					Static_Sum_Va[j] += Sum_Va[j];
					Static_Sum_Va1[j] += Sum_Va1[j];
					Static_Sum_Va2[j] += Sum_Va2[j];
				}
				FILE *fpavg =fopen("muva_avg.cep","a+");
				fprintf(fpavg, "%u\n", Static_Frm_num2);
				for( int j=0;j<VEC_DIM;j++ ){
					fprintf(fpavg, "%lg ", Static_Sum_Mu[j]/Static_Frm_num2);
				}
				fprintf(fpavg, "\n");
				for( int j=0;j<VEC_DIM;j++ ){
					fprintf(fpavg, "%lg ", Static_Sum_Mu1[j]/Static_Frm_num2);
				}
				fprintf(fpavg, "\n");
				for( int j=0;j<VEC_DIM;j++ ){
					fprintf(fpavg, "%lg ", Static_Sum_Mu2[j]/Static_Frm_num2);
				}
				fprintf(fpavg, "\n");
				for( int j=0;j<VEC_DIM;j++ ){
					fprintf(fpavg, "%lg ", Static_Sum_Va[j]/Static_Frm_num2);
				}
				fprintf(fpavg, "\n");
				for( int j=0;j<VEC_DIM;j++ ){
					fprintf(fpavg, "%lg ", Static_Sum_Va1[j]/Static_Frm_num2);
				}
				fprintf(fpavg, "\n");
				for( int j=0;j<VEC_DIM;j++ ){
					fprintf(fpavg, "%lg ", Static_Sum_Va2[j]/Static_Frm_num2);
				}
				fprintf(fpavg, "\n");
				fclose(fpavg);
			}
#endif
		}

#ifdef _DEBUG
		FILE *fp =fopen("log_base.cep","a+");
		FILE *fp2 =fopen("log_cmvn.cep","a+");
#endif
		for( i=m_normedFeatureIdx;i<((bEnd||nFrame>m_nDelayOff)?nFrame:nFrame-m_nNormDelay);i++ ){
#ifdef _DEBUG
			fprintf(fp, "%d:\t", i);
			for( int j=0;j<VEC_DIM;j++ ){
				fprintf(fp, "%.6f ", m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j]);
			}
			fprintf(fp, "\n\t");
			for( int j=0;j<VEC_DIM;j++ ){
				fprintf(fp, "%.6f ", m_pDeltFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j]);
			}
			fprintf(fp, "\n\t");
			for( int j=0;j<VEC_DIM;j++ ){
				fprintf(fp, "%.6f ", m_pAcceFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j]);
			}
			fprintf(fp, "\n");
#endif
			if (!m_bNarrow) {
				for( int j=0;j<VEC_DIM;j++ ){
					m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] -= Mu[j];
					m_pDeltFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] -= Mu1[j];
					m_pAcceFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] -= Mu2[j];
					//fprintf(fp, "%.6f ", m_pBaseFea[(i+LEFT_CONTEXT)*VEC_DIM+j]);
					if( m_bCVN ) {
						m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] /= Va[j];
						m_pDeltFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] /= Va1[j];
						m_pAcceFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] /= Va2[j];
					}
				}
			} else {
				for( int j=0;j<CHS_NUM;j++ ){
					m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] -= Mu[j];
					m_pDeltFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] -= Mu1[j];
					m_pAcceFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] -= Mu2[j];
					//fprintf(fp, "%.6f ", m_pBaseFea[(i+LEFT_CONTEXT)*VEC_DIM+j]);
					if( m_bCVN ) {
						m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] /= Va[j];
						m_pDeltFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] /= Va1[j];
						m_pAcceFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j] /= Va2[j];
					}
				}
			}
#ifdef _DEBUG
			fprintf(fp2, "%d:\t", i);
			for( int j=0;j<VEC_DIM;j++ ){
				fprintf(fp2, "%.6f ", m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j]);
			}
			fprintf(fp2, "\n\t");
			for( int j=0;j<VEC_DIM;j++ ){
				fprintf(fp2, "%.6f ", m_pDeltFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j]);
			}
			fprintf(fp2, "\n\t");
			for( int j=0;j<VEC_DIM;j++ ){
				fprintf(fp2, "%.6f ", m_pAcceFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+j]);
			}
			fprintf(fp2, "\n");
#endif
		}
#ifdef _DEBUG
		fclose(fp);
		fclose(fp2);
#endif
		m_normedFeatureIdx = i;
		//m_FeatureIdx = nFrame;
	}

}

//void PLP::DifFE( int nFrame )
//{
//	int nFrmCnt,nDimCnt;
//	int nDim = VEC_DIM;
///*--------------First Order Differentiation----------------*/
//	for( nFrmCnt=2;nFrmCnt<nFrame-2;nFrmCnt++ ){
//		for( nDimCnt=0;nDimCnt<VEC_DIM;nDimCnt++ ){
//			m_pBaseFea[nFrmCnt*FUL_DIM+nDim+nDimCnt] = \
//			m_pBaseFea[(nFrmCnt+2)*FUL_DIM+nDimCnt] - \
//			m_pBaseFea[(nFrmCnt-2)*FUL_DIM+nDimCnt];
//
//		}
//	}
//
//	for( nDimCnt=0;nDimCnt<VEC_DIM;nDimCnt++ ){
//		m_pBaseFea[nDim+nDimCnt] = m_pBaseFea[FUL_DIM+nDim+nDimCnt] = \
//			m_pBaseFea[2*FUL_DIM+nDim+nDimCnt];
//
//		m_pBaseFea[(nFrame-2)*FUL_DIM+nDim+nDimCnt] = \
//			m_pBaseFea[(nFrame-1)*FUL_DIM+nDim+nDimCnt] = \
//			m_pBaseFea[(nFrame-3)*FUL_DIM+nDim+nDimCnt];
//	}
//
///*---------------Second Order Differentiation-------------------*/
//
//	for( nFrmCnt=2;nFrmCnt<nFrame-2;nFrmCnt++ ){
//		for( nDimCnt=0;nDimCnt<VEC_DIM;nDimCnt++ ){
//			m_pBaseFea[nFrmCnt*FUL_DIM+2*nDim+nDimCnt] = \
//			m_pBaseFea[(nFrmCnt+1)*FUL_DIM+nDim+nDimCnt] - \
//			m_pBaseFea[(nFrmCnt-1)*FUL_DIM+nDim+nDimCnt];
//		}
//	}
//
//	for( nDimCnt=0;nDimCnt<VEC_DIM;nDimCnt++ ){
//		m_pBaseFea[2*nDim+nDimCnt] = m_pBaseFea[FUL_DIM+2*nDim+nDimCnt] = \
//			m_pBaseFea[2*FUL_DIM+2*nDim+nDimCnt];
//		
//		m_pBaseFea[(nFrame-2)*FUL_DIM+2*nDim+nDimCnt] = \
//			m_pBaseFea[(nFrame-1)*FUL_DIM+2*nDim+nDimCnt] = \
//			m_pBaseFea[(nFrame-3)*FUL_DIM+2*nDim+nDimCnt];
//	}
//
//
//}

void PLP::Resume()
{
	//p_nframe	= 0;
	m_frmOffset	= 0;
	m_nFrame	= 0;
	m_nbaseFrame	= 0;
	m_iSamples	= 0;
//	m_ipitchSamples = 0;
	//p_nframe	= 0;
	m_meanFeatureIdx= 0; 
	m_varFeatureIdx= 0; 
	m_normedFeatureIdx = 0;
//	m_PitchIdx	= 0;  
//	m_startPitchIdx	= 0;  
	m_OutIdx	= 0;  
	m_LDAmatIdx = 0;
	m_normCnt = 0;
//	m_unpitchAcced = 0;
	//m_frontLDA	= false;
//	memset(m_pBaseFea,'\0',sizeof(float)*(MAX_SPEECH+CONTEXT_SPAN)*VEC_DIM);
//	memset(m_pDeltFea,'\0',sizeof(float)*(MAX_SPEECH+CONTEXT_SPAN)*VEC_DIM);
//	memset(m_pAcceFea,'\0',sizeof(float)*(MAX_SPEECH+CONTEXT_SPAN)*VEC_DIM);
//	memset(m_pSNR,'\0',sizeof(int)*MAX_SPEECH);
//	memset(m_pBuf,'\0',sizeof(short)*(MAX_SPEECH*FS_KHZ*SFT_MS+FS_KHZ*FRM_MS));
	memset(Sum_Mu,'\0',sizeof(float)*(VEC_DIM));
	memset(Sum_Va,'\0',sizeof(float)*(VEC_DIM));
	memset(Sum_Mu1,'\0',sizeof(float)*(VEC_DIM));
	memset(Sum_Va1,'\0',sizeof(float)*(VEC_DIM));
	memset(Sum_Mu2,'\0',sizeof(float)*(VEC_DIM));
	memset(Sum_Va2,'\0',sizeof(float)*(VEC_DIM));
//	memset(m_Feature,'\0',sizeof(float)*MAX_SPEECH*ALN_FUL_DIM);

#ifndef _WRITESYS
	if (Static_Frm_num == 0) {
		memset(Static_Sum_Mu,'\0',sizeof(float)*(VEC_DIM));
		memset(Static_Sum_Va,'\0',sizeof(float)*(VEC_DIM));
		memset(Static_Sum_Mu1,'\0',sizeof(float)*(VEC_DIM));
		memset(Static_Sum_Va1,'\0',sizeof(float)*(VEC_DIM));
		memset(Static_Sum_Mu2,'\0',sizeof(float)*(VEC_DIM));
		memset(Static_Sum_Va2,'\0',sizeof(float)*(VEC_DIM));
	}
#endif
	//m_PX->InitSent();
}

int PLP::AddBuf(const float *pBuf, int iSamples)
{
	//short * p = m_pBuf;
	//if(true) for(int i=0;i<iSamples;i++)
	//{
	//	m_pBuf[m_iSamples+i] = pBuf[i];
	//}
	int newfrmOffset = m_frmOffset;
	if (m_iSamples + iSamples - m_frmOffset*m_nSftLen > (MAX_SPEECH*m_nSftLen+m_nSftLen)) {
		newfrmOffset = m_nFrame - 1000;
		if (newfrmOffset < m_frmOffset) {
			printf("newfrmOffset < m_frmOffset: %d < %d\n", newfrmOffset, m_frmOffset);
		} else {
			printf("newfrmOffset > m_frmOffset: %d > %d\n", newfrmOffset, m_frmOffset);
		}
		if (m_iSamples + iSamples - newfrmOffset*m_nSftLen > (MAX_SPEECH*m_nSftLen+m_nSftLen)) {
			printf("Too Long Samples is sented: %d <> %d frm\n", iSamples, iSamples/m_nSftLen);
			printf("m_frmOffset = %d\nnewfrmOffset = %d\n", m_frmOffset, newfrmOffset);
			printf("MAX_SPEECH = %d ms\n", MAX_SPEECH);
//			printf("m_nPitchHistory = %d frm\n", m_nPitchHistory);
			return -1;
		}
		memcpy(m_pBuf, m_pBuf + (newfrmOffset-m_frmOffset)*m_nSftLen, sizeof(float)*(m_iSamples - newfrmOffset*m_nSftLen));
		memcpy(m_pBaseFea, m_pBaseFea + (newfrmOffset-m_frmOffset)*VEC_DIM, sizeof(float)*(MAX_SPEECH+CONTEXT_SPAN + m_frmOffset - newfrmOffset)*VEC_DIM);
		memcpy(m_pDeltFea, m_pDeltFea + (newfrmOffset-m_frmOffset)*VEC_DIM, sizeof(float)*(MAX_SPEECH+CONTEXT_SPAN + m_frmOffset - newfrmOffset)*VEC_DIM);
		memcpy(m_pAcceFea, m_pAcceFea + (newfrmOffset-m_frmOffset)*VEC_DIM, sizeof(float)*(MAX_SPEECH+CONTEXT_SPAN + m_frmOffset - newfrmOffset)*VEC_DIM);
		memcpy(m_Feature, m_Feature + (newfrmOffset-m_frmOffset)*ALN_FUL_DIM, sizeof(float)*(MAX_SPEECH + m_frmOffset - newfrmOffset)*ALN_FUL_DIM);
		//memcpy(m_tmpF0, m_tmpF0 + (newfrmOffset-m_frmOffset), sizeof(float)*(MAX_SPEECH + m_frmOffset - newfrmOffset));
		m_frmOffset = newfrmOffset;
	}
	int bufOffset = -m_frmOffset*m_nSftLen;
	memcpy(m_pBuf+bufOffset+m_iSamples,pBuf,sizeof(float)*iSamples);
	int nFrame = 0;
	if(iSamples+m_iSamples >= m_nFrmLen) 
		nFrame = (iSamples+m_iSamples-m_nFrmLen)/m_nSftLen+1;
	//int pFrame = (iSamples+m_iSamples)/p_len;
	//if(nFrame<0) nFrame=0;
//	m_unpitchAcced += nFrame - m_nFrame;

//	if (nFrame>MAX_SPEECH)
//	{
//		printf("Too Long Frames Detected\n");
//		return -1;
	//}

	//float* FrameBuf = new float[m_nFrmLen];
	//float* Real		= new float[FBank.nfftN];
	//float* Imag		= new float[FBank.nfftN];
	//float* FBOut	= new float[FBank.nChans+1];
	//float* ASpec	= new float[FBank.nChans+3];
//	float AutoCorr[LPC_DIM+1];
//	float LPC[LPC_DIM+2];
//	float Cep[CEP_DIM+1];
	float PE,Tmp,E;
	int i,j,bin;
	const float melFloor = 1.0f;
    memset(m_energies, 0, sizeof(float)*2);
	//ǰ13ά����
	for( i=m_nFrame;i<nFrame;i++ ){
		E = 0.0;
		for( j=0;j<m_nFrmLen;j++ ){
			FrameBuf[j] = (float)m_pBuf[bufOffset+i*m_nSftLen+j];
		}
		
		if (m_bUseDither)
		Dither(FrameBuf, m_nFrmLen, 1.0f);
		float dSum = 0.f;
		for( j=0;j<m_nFrmLen;j++ )
			dSum += FrameBuf[j];
		dSum /= m_nFrmLen;
		for( j=0;j<m_nFrmLen;j++ )
			FrameBuf[j] -= dSum;
	//	if (i==0) {
	//		printf("%d,%d\n", iSamples, m_iSamples);
	//		for (j=0; j<m_nFrmLen; j++) {
	//			printf("%.6f/%d/%d ", FrameBuf[j], m_pBuf[j], pBuf[j]); 
	//		}
	//		printf("\n");
	//	}
			
			
/*-------------Pre-emphasize by 0.97---------------*/ 
		PreEmph( FrameBuf,m_nFrmLen );
	//	if (i==0) {
	//		for (j=0; j<m_nFrmLen; j++) {
	//			printf("%.6f ", FrameBuf[j]); 
	//		}
	//		printf("\n");
	//	}

/*----------------Hamming Window------------------*/
		
		for( j=0;j<m_nFrmLen;j++ )
			FrameBuf[j] *= m_pHamWin[j];

		//if (i==0) {
		//	for (j=0; j<m_nFrmLen; j++) {
		//		printf("%.6f ", FrameBuf[j]); 
		//	}
		//	printf("\n");
		//}
/*------------------FFT Operation-------------------*/
		memset( Real,0,sizeof(float)*FBank.nfftN );
		memset( Imag,0,sizeof(float)*FBank.nfftN );
		memcpy( Real,FrameBuf,sizeof(float)*m_nFrmLen );

		m_pFFT->Execute( Real,Imag );

        /* calc speech energy */
        int eidx = i-m_nFrame;
        if (eidx < PLP_MAX_ENERGY_FRAMES) {
            m_energies[eidx] = 0;
            int n = FBank.nfftN / 2-2;
            for (int k=1; k<FBank.nfftN/2-1; k++) {
                m_energies[eidx] += sqrt(Real[k]*Real[k] + Imag[k]*Imag[k])/4;
            }
            m_energies[eidx] /= n;
        }

/*------------------Mel Filter Bank-----------------*/
//Attention: FBOut[0] = 0.0f; No Use;

		memset( FBOut,0,sizeof(float)*(FBank.nChans+1) );
		for( j=FBank.klo;j<=FBank.khi;j++ ){
			
			bin = FBank.loChan[j];
			if (bin < 0) continue;
			if( FBank.bUsePower )
				PE = Real[j-1]*Real[j-1]+Imag[j-1]*Imag[j-1];
			else
				PE = (float)sqrt(Real[j-1]*Real[j-1]+Imag[j-1]*Imag[j-1]);

			Tmp = FBank.loWt[j]*PE;
			if( bin>0 )
				FBOut[bin] += Tmp;
			if( bin<CHS_NUM )
				FBOut[bin+1] += (PE-Tmp);
			
		}
		if (!m_bNarrow) {
			for( j=FBank.wklo;j<=FBank.wkhi;j++ ){
				
				bin = FBank.loChanH[j];
				if (bin < 0) continue;
				if( FBank.bUsePower )
					PE = Real[j-1]*Real[j-1]+Imag[j-1]*Imag[j-1];
				else
					PE = (float)sqrt(Real[j-1]*Real[j-1]+Imag[j-1]*Imag[j-1]);
	
				Tmp = FBank.loWtH[j]*PE;
				if( bin>0 )
					FBOut[bin+CHS_NUM] += Tmp;
				if( bin<CHS_NUM_W )
					FBOut[bin+CHS_NUM+1] += (PE-Tmp);
				
			}
		}
///*----Pre-emphasize filter bank output with the simulated-----*/ 
///*---equal-loudness curve and perform amplitude compression---*/
//		
//		ASpec[0] = 0.0f;
//		for( j=1;j<=FBank.nChans;j++ ){
//			if( FBOut[j]<melFloor )		FBOut[j] = melFloor;
//			ASpec[j+1] = FBOut[j]*m_pEqLCurv[j];
//			ASpec[j+1] = pow( (double)ASpec[j+1],0.33 );
//		}
//		ASpec[FBank.nChans+2] = ASpec[FBank.nChans+1];
//		ASpec[1] = ASpec[2];
//
///*-------------------------IDFT-------------------------*/
//		E = IDFT( ASpec,AutoCorr );
//
///*----------------Get LP Coefficients-----------------*/
//		LPC[LPC_DIM+1] = 0.0f;
//
//		E = Auto2LPC( AutoCorr,LPC,E );
//		if( E<=0 ){
//			printf( "lpcGain Negative\n" );
//			//getchar();
//			//delete []FrameBuf;
//			//delete []Real;
//			//delete []Imag;
//			//delete []FBOut;
//			//delete []ASpec;	
//			E = 1e-9;
//			//return -1;
//		}//Durbin Recursion
//
//		
//		LPC2Cep( LPC,Cep );
//
//		m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*VEC_DIM] = log(E);//log( 1.0e-9+E );
////Use C0 instead of Energy(En)
//		if( m_Lifter>0 )
//			WeightCep( Cep );

		
		//for( j=1;j<=CEP_DIM;j++ )
		//	m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*CEP_DIM+j-1] = FBOut[j];
#ifdef _DEBUG
//		printf("%d\t", i);
#endif
		if (!m_bNarrow) {
			for( j=1;j<=CEP_DIM;j++ ) {
				m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*CEP_DIM+j-1] = log(FBOut[j]);
#ifdef _DEBUG
//				printf("%.6f ", m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*CEP_DIM+j-1]);
#endif
			}
		} else {
			for( j=1;j<=CHS_NUM;j++ ) {
				m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*CEP_DIM+j-1] = log(FBOut[j]);
#ifdef _DEBUG
//				printf("%.6f ", m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*CEP_DIM+j-1]);
#endif
			}
			for( j=CHS_NUM+1;j<=CEP_DIM;j++ ) {
				m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*CEP_DIM+j-1] = 0.f;//log(FBOut[j]);
#ifdef _DEBUG
//				printf("%.6f ", m_pBaseFea[(i+LEFT_CONTEXT-m_frmOffset)*CEP_DIM+j-1]);
#endif
			}
		}
#ifdef _DEBUG
//		printf("\n");
#endif
	
	}
	//pitch����
	//float *PF0;
	//int nPF0;
	//for (i=p_nframe;i<pFrame;i++)
	//{
	//	nPF0=m_PX->Add2Sent(m_pBuf+i*p_len,p_len,PF0);
	//	if (nPF0 > 0)
	//	{
	//		for (j=0;j<nPF0;j++)
	//			m_pBaseFea[j*FUL_DIM+CEP_DIM] = PF0[j];
	//	}
	//}
	//m_PitchIdx = nPF0;
	m_nFrame = nFrame;
	m_iSamples += iSamples;
	return 0;
}

int PLP::Norm_Diff2(bool bEndflag)
{
	//float ComFeature[CONTEXT_SPAN*14];
	int ii,jj,kk;
	//int D=14;
	//int left_context = LEFT_CONTEXT, right_context = RIGHT_CONTEXT;
	//int N = CONTEXT_SPAN;

//	if (!bEndflag && m_unpitchAcced < m_nFirstBuf)
//		return 0;
//
//	m_unpitchAcced = (m_nFirstBuf-m_nIncBuf);
//
//	while (m_startPitchIdx < m_PitchIdx \
//		&& m_iSamples > (m_nPitchHistory + m_startPitchIdx) * SFT_MS * FS_KHZ)
//		m_startPitchIdx++;
//	int offset = m_startPitchIdx * SFT_MS * FS_KHZ;
//	int bufOffset = -m_frmOffset*FS_KHZ*SFT_MS;
//	//float *f0 = new float [m_nFrame];
//	if (m_UsePitch) {
//		m_PitchIdx = m_startPitchIdx + m_PX->RunPitch(m_pBuf + bufOffset + offset, m_iSamples - offset, m_tmpF0, m_nFrame, true, m_nNullPitchDelay);
//		if (bEndflag) {
//			if (m_PitchIdx < m_nFrame) {
//				for(int kk=m_PitchIdx; kk<m_nFrame; ++kk) {
//					m_tmpF0[kk-m_startPitchIdx] = 0.f; 
//				}
//			}
//			m_PitchIdx = m_nFrame;
//		} else {
//			m_PitchIdx -= m_nPitchDelay;
//			if (m_PitchIdx < 0) m_PitchIdx = 0;
//		}
//		for (ii=m_startPitchIdx;ii<m_PitchIdx;ii++)
//			m_pBaseFea[(ii+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+CEP_DIM+1] = m_tmpF0[ii-m_startPitchIdx];
//	} else {
//		m_PitchIdx = m_nFrame;
//		for (ii=m_startPitchIdx;ii<m_PitchIdx;ii++)
//			m_pBaseFea[(ii+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+CEP_DIM+1] = 0.f;
//	}
	//delete [] f0;

	int nFrame = m_nFrame;
	for (ii = m_nbaseFrame; ii<(bEndflag?(nFrame+2):nFrame); ii++) {
		int nDelta = ii - 2;
		if (nDelta < 0) continue;
		int nDeltaA1 = nDelta + 1;
		int nDeltaA2 = nDelta + 2;
		int nDeltaS1 = nDelta - 1;
		int nDeltaS2 = nDelta - 2;
		if (nDeltaA1 > nFrame-1) nDeltaA1 = nFrame-1;
		if (nDeltaA2 > nFrame-1) nDeltaA2 = nFrame-1;
		if (nDeltaS1 < 0) nDeltaS1 = 0;
		if (nDeltaS2 < 0) nDeltaS2 = 0;
		for (jj=0; jj<VEC_DIM; jj++) {
			m_pDeltFea[(nDelta+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+jj] = 
				(
					(m_pBaseFea[(nDeltaA1+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+jj] - m_pBaseFea[(nDeltaS1+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+jj])
					+ 2 * (m_pBaseFea[(nDeltaA2+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+jj] - m_pBaseFea[(nDeltaS2+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+jj])
				) / 10;
		}
	}
	for (ii = m_nbaseFrame; ii<(bEndflag?(nFrame+4):nFrame); ii++) {
		int nDelta = ii - 4;
		if (nDelta < 0) continue;
		//int nDeltaS1 = nDelta - 1;
		//if (nDeltaS1 < 0) nDeltaS1 = 0;
		//int nDeltaS2 = nDelta - 2;
		//if (nDeltaS2 < 0) nDeltaS2 = 0;
		int nDeltaA1 = nDelta + 1;
		int nDeltaA2 = nDelta + 2;
		int nDeltaS1 = nDelta - 1;
		int nDeltaS2 = nDelta - 2;
		if (nDeltaA1 > nFrame-1) nDeltaA1 = nFrame-1;
		if (nDeltaA2 > nFrame-1) nDeltaA2 = nFrame-1;
		if (nDeltaS1 < 0) nDeltaS1 = 0;
		if (nDeltaS2 < 0) nDeltaS2 = 0;
		for (jj=0; jj<VEC_DIM; jj++) {
			m_pAcceFea[(nDelta+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+jj] = 
				(
					(m_pDeltFea[(nDeltaA1+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+jj] - m_pDeltFea[(nDeltaS1+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+jj])
					+ 2 * (m_pDeltFea[(nDeltaA2+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+jj] - m_pDeltFea[(nDeltaS2+LEFT_CONTEXT-m_frmOffset)*VEC_DIM+jj])
				) / 10;
		}
	}
	m_nbaseFrame = nFrame;
#ifdef _DEBUG
	printf("[%d\t", nFrame);// << endl;
#endif
	//CMVN
	//if (nFrame>100 || bEndflag)
	//Norm(bEndflag?nFrame:(nFrame-4));    
//	int nFrame = m_nFrame;
	Norm(nFrame, bEndflag);
#ifdef _DEBUG
	printf("%d\t", m_normedFeatureIdx);// << endl;
#endif
	//LDA&MLLR
	if (m_normedFeatureIdx>RIGHT_CONTEXT) {

		int end_idx = m_normedFeatureIdx;
		if (!bEndflag)
			end_idx = m_normedFeatureIdx - RIGHT_CONTEXT;

		if (m_LDAmatIdx == 0) {
			for (int tt = 0; tt<LEFT_CONTEXT; tt++) {
				memcpy(m_pBaseFea+(tt-m_frmOffset)*VEC_DIM,m_pBaseFea+(LEFT_CONTEXT-m_frmOffset)*VEC_DIM,sizeof(float)*VEC_DIM);
				memcpy(m_pDeltFea+(tt-m_frmOffset)*VEC_DIM,m_pDeltFea+(LEFT_CONTEXT-m_frmOffset)*VEC_DIM,sizeof(float)*VEC_DIM);
				memcpy(m_pAcceFea+(tt-m_frmOffset)*VEC_DIM,m_pAcceFea+(LEFT_CONTEXT-m_frmOffset)*VEC_DIM,sizeof(float)*VEC_DIM);
			}
		}
		if (bEndflag) {
			for (int tt = 0; tt<RIGHT_CONTEXT; tt++) {
				memcpy(m_pBaseFea+(m_normedFeatureIdx+tt+LEFT_CONTEXT-m_frmOffset)*VEC_DIM,m_pBaseFea+(m_normedFeatureIdx-1+LEFT_CONTEXT-m_frmOffset)*VEC_DIM,sizeof(float)*VEC_DIM);
				memcpy(m_pDeltFea+(m_normedFeatureIdx+tt+LEFT_CONTEXT-m_frmOffset)*VEC_DIM,m_pDeltFea+(m_normedFeatureIdx-1+LEFT_CONTEXT-m_frmOffset)*VEC_DIM,sizeof(float)*VEC_DIM);
				memcpy(m_pAcceFea+(m_normedFeatureIdx+tt+LEFT_CONTEXT-m_frmOffset)*VEC_DIM,m_pAcceFea+(m_normedFeatureIdx-1+LEFT_CONTEXT-m_frmOffset)*VEC_DIM,sizeof(float)*VEC_DIM);
			}
		}
		for (ii = m_LDAmatIdx;ii<end_idx;ii++) {
			for (kk = 0; kk<CONTEXT_SPAN; kk++) {
				for (jj=0;jj<VEC_DIM;jj++)
				{
					m_Feature[(ii-m_frmOffset)*ALN_FUL_DIM+(VEC_DIM*3)*kk+jj] = m_pBaseFea[(ii+kk-m_frmOffset)*VEC_DIM+jj];
					m_Feature[(ii-m_frmOffset)*ALN_FUL_DIM+(VEC_DIM*3)*kk+VEC_DIM+jj] = m_pDeltFea[(ii+kk-m_frmOffset)*VEC_DIM+jj];
					m_Feature[(ii-m_frmOffset)*ALN_FUL_DIM+(VEC_DIM*3)*kk+VEC_DIM*2+jj] = m_pAcceFea[(ii+kk-m_frmOffset)*VEC_DIM+jj];
				}
			}
			for (jj=FUL_DIM;jj<ALN_FUL_DIM;jj++)
				m_Feature[(ii-m_frmOffset)*ALN_FUL_DIM+jj] = 0.f;
		}
#ifdef _DEBUG
		FILE *fp =fopen("log.cep2","a+");
		for (int i = m_LDAmatIdx;i<end_idx;i++) {
			fprintf(fp, "%d:\t", i);
			for( int j=0;j<ALN_FUL_DIM;j++ ){
				fprintf(fp, "%.6f ", m_Feature[(i-m_frmOffset)*ALN_FUL_DIM+j]);
			}
			fprintf(fp, "\n");
		}
		fclose(fp);
#endif
	
		m_LDAmatIdx = ii;
	}
#ifdef _DEBUG
		printf("%d]\t", m_LDAmatIdx);// << endl;
#endif
	////���֡��
	m_OutIdx = m_LDAmatIdx;
	return 0;
}


//int PLP::Norm_Diff(bool bEndflag)
//{
//	//float ComFeature[CONTEXT_SPAN*14];
//	int ii,jj,kk;
//	//int D=14;
//	//int left_context = LEFT_CONTEXT, right_context = RIGHT_CONTEXT;
//	//int N = CONTEXT_SPAN;
//
//	if (!bEndflag && m_unpitchAcced < m_nFirstBuf)
//		return 0;
//
//	m_unpitchAcced = (m_nFirstBuf-m_nIncBuf);
//
//	while (m_startPitchIdx < m_PitchIdx && m_iSamples > (m_nPitchHistory + m_startPitchIdx) * SFT_MS * FS_KHZ)
//		m_startPitchIdx++;
//	int offset = m_startPitchIdx * SFT_MS * FS_KHZ;
//	//float *f0 = new float [m_nFrame];
//	m_PitchIdx = m_startPitchIdx + m_PX->RunPitch(m_pBuf + offset, m_iSamples - offset, m_tmpF0, m_nFrame, true, m_nNullPitchDelay);
//	if (bEndflag) {
//		if (m_PitchIdx < m_nFrame) {
//			for(int kk=m_PitchIdx; kk<m_nFrame; ++kk) {
//				m_tmpF0[kk-m_startPitchIdx] = 0.f; 
//			}
//		}
//		m_PitchIdx = m_nFrame;
//	} else {
//		m_PitchIdx -= m_nPitchDelay;
//		if (m_PitchIdx < 0) m_PitchIdx = 0;
//	}
//	for (ii=m_startPitchIdx;ii<m_PitchIdx;ii++)
//		m_pBaseFea[(ii+LEFT_CONTEXT)*VEC_DIM+CEP_DIM+1] = m_tmpF0[ii-m_startPitchIdx];
//	//delete [] f0;
//
//	int nFrame = m_nFrame<m_PitchIdx?m_nFrame:m_PitchIdx;
//#ifdef _DEBUG
//	printf("[%d\t", nFrame);// << endl;
//#endif
//	//CMVN
//	//if (nFrame>100 || bEndflag)
//	Norm(nFrame);
//#ifdef _DEBUG
//	printf("%d\t", m_normedFeatureIdx);// << endl;
//#endif
//	//LDA&MLLR
//	if (m_normedFeatureIdx>RIGHT_CONTEXT) {
//
//		int end_idx = m_normedFeatureIdx;
//		if (!bEndflag)
//			end_idx = m_normedFeatureIdx - RIGHT_CONTEXT;
//
//		if (m_LDAmatIdx == 0) {
//			for (int tt = 0; tt<LEFT_CONTEXT; tt++) {
//				memcpy(m_pBaseFea+tt*VEC_DIM,m_pBaseFea+LEFT_CONTEXT*VEC_DIM,sizeof(float)*VEC_DIM);
//			}
//		}
//		if (bEndflag) {
//			for (int tt = 0; tt<RIGHT_CONTEXT; tt++) {
//				memcpy(m_pBaseFea+(m_normedFeatureIdx+LEFT_CONTEXT+tt)*VEC_DIM,m_pBaseFea+(m_normedFeatureIdx+LEFT_CONTEXT-1)*VEC_DIM,sizeof(float)*VEC_DIM);
//			}
//		}
//		for (ii = m_LDAmatIdx;ii<end_idx;ii++) {
//			for (jj=0;jj<FUL_DIM;jj++)
//			{
//				m_Feature[ii*FUL_DIM+jj] = 0;
//				for (kk=0;kk<VEC_DIM*CONTEXT_SPAN;kk++)
//				{
//					//m_Feature[ii*FUL_DIM+jj] += ComFeature[kk]*m_LDAmat[kk*FUL_DIM+jj];
//					m_Feature[ii*FUL_DIM+jj] += m_pBaseFea[ii*VEC_DIM+kk]*m_LDAmat[jj*CONTEXT_SPAN*VEC_DIM+kk];
//				}
//			}
//		}
//		m_LDAmatIdx = ii;
//	}
//#ifdef _DEBUG
//	printf("%d]\t", m_LDAmatIdx);// << endl;
//#endif
//	
//	m_OutIdx = m_LDAmatIdx;
//	return 0;
//}
    
