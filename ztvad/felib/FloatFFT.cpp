#include "FloatFFT.h"

CFloatFFT::CFloatFFT(int nSize)
{
	m_pRFFT = new KRFFT<float>(nSize);
	m_nSize = nSize;
}

CFloatFFT::~CFloatFFT(void)
{
}

bool CFloatFFT::SetSize(int nSize)
{
	if (nSize == m_nSize)
		return true;
	delete m_pRFFT;
	m_pRFFT = new KRFFT<float>(nSize);
	m_nSize = nSize;
    return true;
}


void CFloatFFT::Execute(float *real, float *img, bool bInv)
{
	if(!bInv)
	{
		m_pRFFT->do_fft(img,real);

		int i,n2 = m_nSize / 2;
		for(i=0;i<=n2;i++){
			real[i] = img[i];
			if(i>0 && i<n2){
				img[i] = img[i+n2];
			}
		}
		for(;i<m_nSize;i++){
			real[i] = img[i] = 0;
		}
		img[0] = img[n2] = 0;
		for(i=1;i<n2;i++){
			real[n2+i] = real[n2-i];
			img[n2+i] = -img[n2-i];
		}
	}
	else
	{
		// do not come here
	}
}
