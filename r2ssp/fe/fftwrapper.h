#ifndef _FFTWRAPPER_H_
#define _FFTWRAPPER_H_

#include "fftw3.h"
#include "lockwrapper.h"

#pragma comment(lib, "mkl_core_dll.lib")
#pragma comment(lib, "mkl_sequential_dll.lib")
#ifdef _WIN64
#pragma comment(lib, "mkl_intel_lp64_dll.lib")
#else
#pragma comment(lib, "mkl_intel_c_dll.lib")
#endif

typedef unsigned int UINT;
class FFTWrapper  
{
public:
	// Add by Gao Peng, 2006.2.24
	// !!!!! Must call this Init method at a one-time global init place
	// sample code:
	// ...
	// FFTWrapper::Init();
	//...
	// DO NOT call this method on each FFTWrapper instance
	static void Init();
	static void Destroy();

	FFTWrapper(UINT nSize);
	virtual ~FFTWrapper();

	bool SetSize(UINT nSize);

	void Execute(float *real, float *img, bool bInv = false);
	void Execute(short *real, short *img, bool bInv = false);

protected:
	UINT	m_nSize;

	static cs_lock_t m_lock;

	fftwf_complex	*m_in, *m_out;
	fftwf_plan	m_plan, m_inv;
};

#endif
