#pragma once

#include "KRFFT.H"

class CFloatFFT
{
public:
	CFloatFFT(int nSize);
	~CFloatFFT(void);

	bool SetSize(int nSize);
	void Execute(float *real, float *img, bool bInv = false);
protected:
	int	m_nSize;
	KRFFT<float> * m_pRFFT;
};
