// FFTWrapper.cpp: implementation of the FFTWrapper class.

#include "fftw3.h"
#include "fftwrapper.h"

cs_lock_t FFTWrapper::m_lock;

void FFTWrapper::Init()
{
	init_lock(&m_lock);
}

void FFTWrapper::Destroy()
{
	destroy_lock(&m_lock);
}

FFTWrapper::FFTWrapper(UINT nSize)
{
	m_nSize = nSize;

	cs_lock(&m_lock);
	m_in = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*nSize);
	m_out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*nSize);
	m_plan = fftwf_plan_dft_1d(nSize, m_in, m_out, FFTW_FORWARD, FFTW_ESTIMATE);
	m_inv = fftwf_plan_dft_1d(nSize, m_in, m_out, FFTW_BACKWARD, FFTW_ESTIMATE);
	cs_unlock(&m_lock);
}

FFTWrapper::~FFTWrapper()
{
	cs_lock(&m_lock);
	fftwf_destroy_plan(m_plan);
	fftwf_destroy_plan(m_inv);
	fftwf_free(m_in);
	fftwf_free(m_out);
	cs_unlock(&m_lock);
}

bool FFTWrapper::SetSize(UINT nSize)
{
	if (nSize == m_nSize)
		return true;

	cs_lock(&m_lock);

	fftwf_destroy_plan(m_plan);
	fftwf_destroy_plan(m_inv);
	fftwf_free(m_in);
	fftwf_free(m_out);

	m_nSize = nSize;
	m_in = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*nSize);
	m_out = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*nSize);
	m_plan = fftwf_plan_dft_1d(nSize, m_in, m_out, FFTW_FORWARD, FFTW_ESTIMATE);
	m_inv = fftwf_plan_dft_1d(nSize, m_in, m_out, FFTW_BACKWARD, FFTW_ESTIMATE);

	cs_unlock(&m_lock);

	return true;
}

void FFTWrapper::Execute(float *real, float *img, bool bInv)
{
	unsigned i;
	for ( i=0; i<m_nSize; i++) {
		m_in[i][0] = real[i];
		m_in[i][1] = img[i];
	}
	fftwf_execute(bInv ? m_inv : m_plan);
	for (i=0; i<m_nSize; i++) {
		real[i] = m_out[i][0];//m_nSize;
		img[i] = m_out[i][1];//m_nSize;
	}
}

void FFTWrapper::Execute(short *real, short *img, bool bInv)
{
	unsigned i;
	for ( i=0; i<m_nSize; i++) {
		m_in[i][0] = real[i];
		m_in[i][1] = img[i];
	}
	fftwf_execute(bInv ? m_inv : m_plan);
	for (i=0; i<m_nSize; i++) {
		real[i] = m_out[i][0]/m_nSize;
		img[i] = m_out[i][1]/m_nSize;
	}
}

