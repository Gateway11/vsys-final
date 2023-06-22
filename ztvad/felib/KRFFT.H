/****************************
*
* 语音识别工具包
* 2012年2月22日 柯登峰
*
****************************/
#ifndef _KRFFT_H_2009_OCT_28_BY_KDF_
#define	_KRFFT_H_2009_OCT_28_BY_KDF_

/*
  长语音修改FFT谱幅度再IFFT回去，时序不乱
  长语音修改FFT谱角度再IFFT回去，时序打乱
*/

/*速度测试结果

点数    FFT时间 IFFT时间
4       0.000011826ms   0.000024033ms
8       0.000047302ms   0.000059509ms
16      0.000143433ms   0.000166321ms
32      0.000335693ms   0.000427246ms
64      0.000958252ms   0.000952148ms
128     0.002478027ms   0.002099609ms
256     0.005712891ms   0.005737305ms
512     0.016748047ms   0.016796875ms
1024    0.029003906ms   0.025976563ms
2048    0.073242188ms   0.073242188ms
4096    0.201171875ms   0.232031250ms
8192    0.464062500ms   0.549218750ms
16384   1.025000000ms   1.196875000ms
32768   2.000000000ms   1.709375000ms
65536   7.618750000ms   7.225000000ms
131072  27.150000000ms  27.350000000ms
262144  59.375000000ms  63.275000000ms

*/

#include "krfft.pre"
template<typename flt_t>
class	KRFFT
{
	/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
	
public:
	
	explicit			KRFFT (const long length);
	~KRFFT ();

	void				do_fft (flt_t f [], const flt_t x []) const;
	void				do_ifft (const flt_t f [], flt_t x []) const;
	void				rescale (flt_t x []) const;
	
private:
	
	const BitReversedLUT<flt_t>	_bit_rev_lut;
	const TrigoLUT<flt_t>	_trigo_lut;
	const flt_t		_sqrt2_2;
	const long		_length;
	const int		_nbr_bits;
	flt_t *			_buffer_ptr;
	
	/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
	
private:
	
	KRFFT (const KRFFT &other);
	const KRFFT&	operator = (const KRFFT &other);
	int				operator == (const KRFFT &other);
	int				operator != (const KRFFT &other);
};
#include "KRFFT.inl"

#endif	// _KRFFT_H_2009_OCT_28_BY_KDF_
