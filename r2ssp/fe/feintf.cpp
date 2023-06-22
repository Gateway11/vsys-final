// FE_PLP.cpp : Defines the entry point for the DLL application.
//

#include <iostream>

#include "iniparser.h"
#include "iniwrapper.h"
#include "PLP.h"
#include "math.h"
#include "feintf.h"

#ifdef _WIN32
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch( ul_reason_for_call ){
	case DLL_PROCESS_ATTACH:
		FFTWrapper::Init();
		break;
	}
    return TRUE;
}

#else
/*
void __attribute__ ((constructor)) my_load(void);
void __attribute__ ((destructor)) my_unload(void);

void my_load(void)
{
	FFTWrapper::Init();
	std::cout << "init" << std::endl;
}

void my_unload(void)
{
	FFTWrapper::Destroy();
	std::cout << "unload" << std::endl;
}
*/
#endif

const int nFsKHz  = FS_KHZ;				//	�����ʣ�ǧ���ȣ�
const int nFrmMs  = FRM_MS;				//	֡�������룩
const int nSftMs  = SFT_MS;				//	֡�ƣ����룩
const int nFtrDim = ALN_FUL_DIM;		//	����ά��
const int nMaxFrm = MAX_SPEECH;		//	���֡��
const int nMinFrm = MIN_SPEECH;
FE_HDR fehdr = {
	nFsKHz, nFrmMs, nSftMs,
	nMaxFrm, nFtrDim,
	"(logEn, C1~12, Pitch) * 3 * nFrm",
	""
};

HANDLE OpenFE(char* szWorkDir, FE_HDR& feInfo)
{
	char	szVersion[256];
	char	gWorkDir[256];

	strcpy( gWorkDir, "./" );
	if( szWorkDir!=NULL ) {
		strcpy( gWorkDir, szWorkDir );
		if (gWorkDir[strlen(gWorkDir)-1] != '/' && gWorkDir[strlen(gWorkDir)-1] != '\\') {
			strcat( gWorkDir, "/" );
		}
	}

	char szPLPConfig[256];
	strcpy( szPLPConfig,gWorkDir );
	strcat( szPLPConfig,"PLP.ini" );

	dictionary* inidict = iniparser_load(const_cast<char*>(szPLPConfig));

	int LoPass,HiPass,WLoPass,WHiPass,FrmMs,SftMs;
	int LoWarpCut,HiWarpCut;
	bool bCVN;
	bool bUsePow;
	UINT nRet;
	LoPass = getint(inidict, "PAR","LOFREQ",-1);
	if( LoPass==-1 )	LoPass = 125;

	HiPass = getint(inidict, "PAR","HIFREQ",-1);
	if( HiPass==-1 )	HiPass = nFsKHz*250-200;

	WLoPass = getint(inidict, "PAR","WLOFREQ",-1);
	if( WLoPass==-1 )	WLoPass = nFsKHz*250;

	WHiPass = getint(inidict, "PAR","WHIFREQ",-1);
	if( WHiPass==-1 )	WHiPass = nFsKHz*500-200;


	if( LoPass<0 ){
		printf( "CAUTION:	LOFREQ is Below Zero,Set to Zero\n" );
		LoPass = 0;
	}

	if( HiPass>nFsKHz*500 ){
		printf( "CAUTION:	HIFREQ is Above Nyquist Frequency,Set to Nyquist Frequency\n" );
		LoPass = nFsKHz*500;
	}

	FrmMs = getint(inidict, "PAR","FRM_MS",-1);
	if( FrmMs==-1 ) FrmMs = nFrmMs;

	SftMs = getint(inidict, "PAR","SFT_MS",-1);
	if( SftMs==-1 )	SftMs = nSftMs;

	
	fehdr.nFrmMs = FrmMs;
	fehdr.nSftMs = SftMs;
	sprintf( szVersion,"FE PLP %d KHz, %d ms frame, %d ms shift, 2005/08/24.",nFsKHz,FrmMs,SftMs );	
	strcpy( fehdr.szVersion,szVersion );

	feInfo = fehdr;
	
	bCVN = getint(inidict, "PAR","USE_CVN",1);
	bool bUseDither = getint(inidict, "PAR","USE_DITHER",1);
//	nRet = GetPrivateProfileInt( "PAR","USE_CVN",-1,szPLPConfig );
//	if( nRet==(UINT)-1 )	bCVN = false;
//	else if( nRet==1 )	bCVN = true;
//	else if( nRet==0 )	bCVN = false;
//	else{
//		printf( "USE_CVN is set to %d,Return to Default\n",nRet );
//		bCVN = true;
//	}
	bool bNarrow = getint(inidict, "PAR","NARROW",0);

	bUsePow = getint(inidict, "PAR","USE_POW",1);
	bool bUsePitch = getint(inidict, "PAR","USE_PITCH",1);
//	nRet = GetPrivateProfileInt( "PAR","USE_POW",-1,szPLPConfig );
//	if( nRet==(UINT)-1 )	bUsePow = true;
//	else if( nRet==1 )	bUsePow = true;
//	else if( nRet==0 )	bUsePow = false;
//	else{
//		printf( "USE_POW is set to %d,Return to Default\n",nRet );
//		bUsePow = true;
//	}
	
	LoWarpCut = getint(inidict, "PAR","LoWarpCut",-1);

	HiWarpCut = getint(inidict, "PAR","HiWarpCut",-1);

	int nLifter = getint(inidict, "PAR","CEP_LIF",-1);

	int nFirstBuf = getint(inidict, "PAR","FIRST_BUF",190);
	int nIncBuf = getint(inidict, "PAR","AFTER_BUF",50);
	int nNormDelay = getint(inidict, "PAR","NORM_DELAY",10);
	int nPitchDelay = getint(inidict, "PAR","PITCH_DELAY",0);
	int nPitchHist = getint(inidict, "PAR","PITCH_HISTORY",1000);
	int nNormHist = getint(inidict, "PAR","NORM_HISTORY",1000);
	int nDelayOff = getint(inidict, "PAR","DELAY_OFF",200);
	int nPreAvgWeight = getint(inidict, "PAR","PRE_AVG",200);
	int nNullPitchDelay = getint(inidict, "PAR","NULL_PITCH_DELAY",100);
//	int nFirstBuf = GetPrivateProfileInt( "PAR","FIRST_BUF ",190,szPLPConfig );
//	int nIncBuf = GetPrivateProfileInt( "PAR","AFTER_BUF",50,szPLPConfig );
//	int nNormDelay = GetPrivateProfileInt( "PAR","NORM_DELAY ",0,szPLPConfig );
//	int nPitchDelay = GetPrivateProfileInt( "PAR","PITCH_DELAY ",0,szPLPConfig );
//	int nPitchHist = GetPrivateProfileInt( "PAR","PITCH_HISTORY",3000,szPLPConfig );
//	int nDelayOff = GetPrivateProfileInt( "PAR","DELAY_OFF",200,szPLPConfig );
//	int nPreAvgWeight = GetPrivateProfileInt( "PAR","PRE_AVG",200,szPLPConfig );
	iniparser_freedict(inidict);	
	if (nPitchHist > 2000)
		nPitchHist = 2000;
	if (nNormHist > 10000)
		nNormHist = 10000;

	PLP* pXFE = new PLP( FrmMs, SftMs, bNarrow );
	if( pXFE==NULL )
		return NULL;
	pXFE->m_bCVN = bCVN;

//	pXFE->InitPar( gWorkDir,LoPass,HiPass,bUsePow,bUsePitch,LoWarpCut,HiWarpCut,nLifter, nFirstBuf, nIncBuf, nPitchDelay, nPitchHist, nNormDelay, nNormHist, nDelayOff, nPreAvgWeight, nNullPitchDelay );
	pXFE->InitPar( gWorkDir,LoPass,HiPass,WLoPass,WHiPass,bUsePow,bUseDither, nFirstBuf, nIncBuf, nNormDelay, nNormHist, nDelayOff, nPreAvgWeight );
	
#ifndef _FINAL_RELEASE
	printf( "\n********MF-PLP Par Config Multi-Thread.ver1.1 online ************\n" );
	printf( "\tPLP DLL for		%dK\n",nFsKHz );
	printf( "\tMel-Freq Bank Channel:	%d\n",CHS_NUM );
	printf( "\tFrame Length:		%d Ms\n",FrmMs );
	printf( "\tFrame Shift:		%d Ms\n",SftMs );
	printf( "\tMel Filter Bank LoCut:	%d Hz\n",LoPass );
	printf( "\tMel Filter Bank HiCut:	%d Hz\n",HiPass );
	if( LoWarpCut>0&&HiWarpCut>0 ){
		printf( "\tLoWarpCut for VTLN   	%d Hz\n",LoWarpCut );
		printf( "\tHiWarpCut for VTLN   	%d Hz\n",HiWarpCut );
	}
	printf( "\tUsing CVN	        %s\n",(bCVN)?"YES":"NO" );
	printf( "\tUsing POW	        %s\n",(bUsePow)?"YES":"NO" );
	printf( "\tUsing Dither	        %s\n",(bUseDither)?"YES":"NO" );
	printf( "\tNarrow band          %s\n",(bNarrow)?"YES":"NO" );
	printf( "\tCepstrum Lifter:	%d\n",nLifter );
	printf( "\tPLP Dim			%d\n",fehdr.nFtrDim );
	printf( "\tFIRST_BUF		%d\n",nFirstBuf );
	printf( "\tINC_BUF   		%d\n",nIncBuf );
	printf( "\tNORM_DELAY		%d\n",nNormDelay );
	printf( "\tDELAY_OFF		%d\n",nDelayOff );
	printf( "\tPRE_AVG			%d\n",nPreAvgWeight );
	printf( "\tNULL_PITCH_DELAY:		%d\n",nNullPitchDelay );
	printf( "*********************************************************\n" );
#endif
	return (HANDLE)pXFE;
}

FE_CODE RunFE( HANDLE hFE,short* pWave, int nLen, float*& pFeature,
			  int*& pSNR, int& nFrm, bool bComp)
{
	nFrm = (nLen - fehdr.nFrmMs * fehdr.nFsKHz) / fehdr.nFsKHz / fehdr.nSftMs + 1;
	if( hFE==NULL||pWave==NULL || nFrm <nMinFrm || nFrm >= nMaxFrm ) {
		return FE_OVERFLOW;
	}
	PLP* pXFE = (PLP* )hFE;	
	pXFE->GetBuf( pWave,nLen );
//	pXFE->Norm_Diff( true );
	pXFE->GetFE( pFeature,pSNR,nFrm );
	if (nFrm <nMinFrm || nFrm >= nMaxFrm)
		return FE_OVERFLOW;

	return FE_OK;
}

FE_CODE CloseFE( HANDLE hFE )
{
	PLP* pXFE = (PLP* )hFE;
	if( pXFE ){
		delete pXFE;
		pXFE = NULL;
	}
	return FE_OK;
}

FE_CODE WarpFE( HANDLE hFE,float Alpha )
{
	PLP* pXFE = (PLP* )hFE;	
	if( pXFE==NULL )
		return FE_MEM_ERR;
//	pXFE->ResetFB( Alpha );
	
	return FE_OK;
}

FE_CODE ResetFE( HANDLE hFE )
{
	if( hFE==0 )
		return FE_MEM_ERR;

	PLP* pFE = (PLP* )hFE;
	pFE->Resume();

	return FE_OK;
}

FE_CODE ContinueFE( HANDLE hFE, const float* pWave, int nLen/*, float*& pFeature,
			  int*& pSNR, int& nFrm, bool bComp */)
{
	if( hFE==0 )
		return FE_MEM_ERR;

	PLP* pFE = (PLP* )hFE;
	//nFrm = (nLen - nFrmMs * nFsKHz) / nFsKHz / nSftMs + 1;
	//if( pWave==NULL || nFrm <nMinFrm || nFrm >= nMaxFrm ) {
	//	return FE_OVERFLOW;
	//}
	if (nLen == 0)
		return FE_OK;

	pFE->AddBuf( pWave,nLen );
	//if (bComp)
	//	pFE->GetFE( pFeature,nFrm,pSNR );
	return FE_OK;
}


FE_CODE NormDiffFE( HANDLE hFE, float*& pFeature, 
			  int*& pSNR, int& nFrm, bool bEndflag )
{
	if( hFE==0 )
		return FE_MEM_ERR;

	PLP* pFE = (PLP* )hFE;
	//nFrm = (nLen - nFrmMs * nFsKHz) / nFsKHz / nSftMs + 1;
	//if( pWave==NULL || nFrm <nMinFrm || nFrm >= nMaxFrm ) {
	//	return FE_OVERFLOW;
	//}

	//printf("nFrm, \n");
	//if (bEndflag)
	//	printf("end\n");
	pFE->Norm_Diff2( bEndflag );
	pFE->GetFE( pFeature,pSNR,nFrm);
	return FE_OK;
}

FE_CODE GetEnergyFE(HANDLE hFE, float*& pEnergies)
{
	if( hFE==0 )
		return FE_MEM_ERR;

	PLP* pFE = (PLP* )hFE;
	pEnergies = pFE->GetEnergies();
	return FE_OK;
}
