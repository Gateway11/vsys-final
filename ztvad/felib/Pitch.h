/*
 * This software has been licensed to the Centre of Speech Technology, KTH
 * by Microsoft Corp. with the terms in the accompanying file BSD.txt,
 * which is a BSD style license.
 *
 *    "Copyright (c) 1990-1996 Entropic Research Laboratory, Inc. 
 *                   All rights reserved"
 *
 * Written by:  David Talkin
 * Checked by:
 * Revised by:
 * f0.h	1.4 9/9/96 ERL
 * Brief description:
 *
 */

/*
 * getf0.h
 * 
 * $Id: getf0.h,v 1.1.1.1 2006/10/16 03:48:32 sako Exp $
 *
 */

#ifndef _PITCH_H_
#define _PITCH_H_

/* f0.h */
/* Some definitions used by the "Pitch Tracker Software". */
typedef struct f0_params {
float cand_thresh,	/* only correlation peaks above this are considered */
      lag_weight,	/* degree to which shorter lags are weighted */
      freq_weight,	/* weighting given to F0 trajectory smoothness */
      trans_cost,	/* fixed cost for a voicing-state transition */
      trans_amp,	/* amplitude-change-modulated VUV trans. cost */
      trans_spec,	/* spectral-change-modulated VUV trans. cost */
      voice_bias,	/* fixed bias towards the voiced hypothesis */
      double_cost,	/* cost for octave F0 jumps */
      mean_f0,		/* talker-specific mean F0 (Hz) */
      mean_f0_weight,	/* weight to be given to deviations from mean F0 */
      min_f0,		/* min. F0 to search for (Hz) */
      max_f0,		/* max. F0 to search for (Hz) */
      frame_step,	/* inter-frame-interval (sec) */
      wind_dur;		/* duration of correlation window (sec) */
int   n_cands,		/* max. # of F0 cands. to consider at each frame */
      conditioning;     /* Specify optional signal pre-conditioning. */
} F0_params;

/* Possible values returned by the function f0(). */
#define F0_OK		0
#define F0_NO_RETURNS	1
#define F0_TOO_FEW_SAMPLES	2
#define F0_NO_INPUT	3
#define F0_NO_PAR	4
#define F0_BAD_PAR	5
#define F0_BAD_INPUT	6
#define F0_INTERNAL_ERR	7

/* Bits to specify optional pre-conditioning of speech signals by f0() */
/* These may be OR'ed together to specify all preprocessing. */
#define F0_PC_NONE	0x00		/* no pre-processing */
#define F0_PC_DC	0x01		/* remove DC */
#define F0_PC_LP2000	0x02		/* 2000 Hz lowpass */
#define F0_PC_HP100	0x04		/* 100 Hz highpass */
#define F0_PC_AR	0x08		/* inf_order-order LPC inverse filter */
#define F0_PC_DIFF	0x010		/* 1st-order difference */

#define Fprintf (void)fprintf

/* f0_structs.h */

#define BIGSORD 100

typedef struct cross_rec { /* for storing the crosscorrelation information */
	float	rms;	/* rms energy in the reference window */
	float	maxval;	/* max in the crosscorr. fun. q15 */
	short	maxloc; /* lag # at which max occured	*/
	short	firstlag; /* the first non-zero lag computed */
	float	*correl; /* the normalized corsscor. fun. q15 */
} Cross;

typedef struct dp_rec { /* for storing the DP information */
	short	ncands;	/* # of candidate pitch intervals in the frame */
	short	*locs; /* locations of the candidates */
	float	*pvals; /* peak values of the candidates */
	float	*mpvals; /* modified peak values of the candidates */
	short	*prept; /* pointers to best previous cands. */
	float	*dpvals; /* cumulative error for each candidate */
} Dprec;

typedef struct windstat_rec {  /* for lpc stat measure in a window */
    float rho[BIGSORD+1];
    float err;
    float rms;
} Windstat;

typedef struct sta_rec {  /* for stationarity measure */
  float *stat;
  float *rms;
  float *rms_ratio;
} Stat;


typedef struct frame_rec{
  Cross *cp;
  Dprec *dp;
  float rms;
  struct frame_rec *next;
  struct frame_rec *prev;
} Frame;

#ifndef min
//#define min(x,y)  ((x)>(y)?(y):(x))
//#define max(x,y)  ((x)<(y)?(y):(x))
#endif

/* wave data structure */
typedef struct _wav_params {
	char *file;
	int rate;
	int startpos;
	int nan;
	int size;
	int swap;
	int padding;
	int length;
	int head_pad;
	int tail_pad;
	float *data;
} wav_params;

/* output data structure */
typedef struct _out_params {
	int nframe;
	float *f0p, *vuvp, *rms_speech, *acpkp;
} out_params;

/* unit size of IO buffer */
#define INBUF_LEN 1024

/* IO buffer structure */
typedef struct _bufcell{
	int len;
	short *data;
	struct _bufcell *next;
} bufcell;

//int Get_SoundData( wav_params *, int, float *, int);
//int load_wavfile( wav_params *);
//int dump_output( char *, out_params *, int, int);
//void swap2w( char *);
//void usage();

//bufcell *bufcell_new();
//void bufcell_free( bufcell *);
//int init_out_params( out_params *, int);

///// sigproc.cc

#define ckalloc(x) malloc(x)
#define ckfree(x) free(x)
#define ckrealloc(x,y) realloc(x,y)


class CF0
{
public:
	CF0(void) {
		headF = NULL; tailF = NULL; cmpthF = NULL;
		pcands = NULL;
		cir_buff_growth_count = 0;
		windstat = NULL;
		f0p = NULL; vuvp = NULL; rms_speech = NULL; acpkp = NULL; g_peaks = NULL;
		first_time = 1;
		debug_level = 0;
		stat = NULL;
		mem = NULL;
		framestep = -1;
		foutput = NULL;
		ncoeff = 127; ncoefft = 0;
		co=NULL; mem_=NULL;
		fsize=0; resid=0;
		nframes_old = 0;
		din = NULL;
		n0 = 0;
		wsize = 0;
		wind=NULL;
		nwind_ = 0;
		dwind_ = NULL;
		nwind=0;
		dwind=NULL;
		dbdata_=NULL;
		dbsize_ = 0;
		dbdata=NULL;
		dbsize = 0;
		locs = NULL;
		wReuse = 0;
	}
	~CF0(void) {};

	int RunPitch(short* ptr, int num_samples, float* out, int buf_size, bool is_smooth = false);
private:

	/*
	* headF points to current frame in the circular buffer, 
	* tailF points to the frame where tracks start
	* cmpthF points to starting frame of converged path to backtrack
	*/
	float *din;
	int n0;

	int wsize;
	float *wind;
	//static int wsize = 0;
	//static float *wind=NULL;
	//static int wsize = 0;
	//static float *wind=NULL;

	int nwind_;
	float *dwind_;
	int nwind;
	float *dwind;

	float *dbdata_;
	int dbsize_;
	float *dbdata;
	int dbsize;

	int nframes_old, memsize;

	float *co, *mem_;
	float state[1000];
	int fsize, resid;

	float	b[2048];
	float *foutput;
	int	ncoeff, ncoefft;
	int framestep;
	Frame *headF, *tailF, *cmpthF;
	//static Frame *headF = NULL, *tailF = NULL, *cmpthF = NULL;

	int *pcands;	/* array for backtracking in convergence check */
	//static  int *pcands = NULL;	/* array for backtracking in convergence check */
	int cir_buff_growth_count;
	//static int cir_buff_growth_count = 0;

	int size_cir_buffer,	/* # of frames in circular DP buffer */
		size_frame_hist,	/* # of frames required before convergence test */
		size_frame_out,	/* # of frames before forcing output */
		num_active_frames,	/* # of frames from tailF to headF */
		output_buf_size;	/* # of frames allocated to output buffers */

	/* 
	* DP parameters
	*/
	float tcost, tfact_a, tfact_s, frame_int, vbias, fdouble, wdur, ln2,
		freqwt, lagwt;
	int step, size, nlags, start, stop, ncomp, *locs;
	short maxpeaks;

	int wReuse;  /* number of windows seen before resued */
	Windstat *windstat;

	float *f0p, *vuvp, *rms_speech, *acpkp, *g_peaks;
	//static float *f0p = NULL, *vuvp = NULL, *rms_speech = NULL, 
	//	*acpkp = NULL, *g_peaks = NULL;
	int first_time, pad;
	//static int first_time = 1, pad;

	int debug_level;
	Stat *stat;
	float *mem;


	//F0_params *new_f0_params();
	int xget_window( register float *dout,
		register int n, register int type);
	void xrwindow( 
		register float *din,
		register float *dout, 
		register int n, register float preemp );
	void xcwindow(
		register float *din,
		register float *dout,
		register int n, register float preemp);
	void xhwindow( 
		register float *din,
		register float *dout,
		register int n, register float  preemp);
	void xhnwindow( 
		register float *din,
		register float *dout,
		register int n, register float preemp);
	int window(
		register float *din,
		register float *dout, 
		register int n,register float preemp,
		int type);

	void xautoc( 
		register int windowsize,
		register float *s, register int p,
		register float *r, register float *e);
	void xdurbin ( 
		/* analysis order */
		register float *r, register float *k, register float *a,register int p, register float *ex);
	void xa_to_aca ( 
		float *a, float*b, float*c,
		register int p);

	void crossf( float *data, int size, int start, int nlags, float *engref, int *maxloc, float *maxval,float *correl );
	void crossfi( float *data, int size, int start0, int nlags0, int nlags, 
		float *engref, int *maxloc, float *maxval,float *correl,
		int *locs, int nlocs);


	int  eround(register double), lpc(), get_window(register float *dout,
		register int n, register int type);

	//Frame *alloc_frame(int , int );

	void get_fast_cands(
		float *fdata, float *fdsdata,  int ind, int step,  int size, int dec, int start, int nlags,
		float *engref, int *maxloc,float *maxval, Cross *cp, float *peaks,  int *locs, int *ncand,  F0_params *par);
	//void get_fast_cands( float *fdata, float *fdsdata,  int ind, int step,  int size, int dec, int start, int nlags,
	//	float *engref, int *maxloc,float *maxval, Cross *cp, float *peaks,  int *locs, int *ncand,  F0_params *par);

	float xitakura (
		register int p,register float *b, register float *c,register float *r,register float *gain ); 
	int xlpc(
		int lpc_ord,		/* Analysis order */
		float lpc_stabl,	/* Stability factor to prevent numerical problems. */
		int  wsize,			/* window size in points */
		float *data,
		float  *lpca,		/* if non-NULL, return vvector for predictors */
		float  *ar,		/* if non-NULL, return vector for normalized autoc. */
		float  *lpck,		/* if non-NULL, return vector for PARCOR's */
		float  *normerr,		/* return scaler for normalized error */
		float  *rms,		/* return scaler for energy in preemphasized window */
		float  preemp,
		int  type		/* window type (decoded in window() above) */
		);	/* input data sequence; assumed to be wsize+1 long */
	float wind_energy(
		register float *data,	/* input PCM data */
		register int size,		/* size of window */
		register int   w_type);			/* window type */
	bufcell *bufcell_new();
	void bufcell_free( bufcell *p);

	int load_wavfile( wav_params *par);
	void swap2w( char *p);
	int Get_SoundData( wav_params *par, int ndone , float *fdata, int actsize);

	int init_out_params( out_params *par, int len);
	void free_out_params( out_params *par);

	int dump_output( char *filename, out_params *par, int full, int binary);
	int Get_f0(wav_params *wpar, F0_params *par, out_params *opar);
	int Smooth_f0(out_params *opar, float* out);
	int check_f0_params( F0_params *par, double sample_freq);
	float *downsample(float *input,int samsin,int state_idx, double freq, int *samsout, int decimate, int first_time, int last_time);
	void get_cand(Cross *cross,float *peak,int *loc,int nlags,int *ncand,float cand_thresh);
	int downsamp(float *in, float *out, int samples, int *outsamps, int state_idx, int decimate, int ncoef, float fc[], int init);
	void do_ffir(register float *buf,register int in_samps,register float *bufo, 
		register int *out_samps,int idx, register int ncoef,float *fc,
		register int invert,register int skip,register int init);
	int lc_lin_fir(register float fc,int *nf,float *coef);
	void peak(float *y, float *xp, float *yp);
	int get_Nframes(long buffsize, int pad,int  step);

	int init_dp_f0(double freq,F0_params* par, long *buffsize, long *sdstep);
	int dp_f0(float *fdata, int buff_size, int sdstep,double freq,
		F0_params*	par, float **f0p_pt, float **vuvp_pt, float **rms_speech_pt, float **acpkp_pt, int *vecsize, int last_time);
	void free_dp_f0();

	Frame * alloc_frame(int nlags, int ncands);
	int save_windstat(float *rho,int  order,float  err, float rms);
	int retrieve_windstat(float *rho, int order,float * err, float *rms);
	float get_similarity(int order, int size,float * pdata,float * cdata,
		float *rmsa, float *rms_ratio,float pre, float stab, int w_type, int init);

	Stat* get_stationarity(float *fdata, double freq, int buff_size, int nframes, int frame_step,int first_time);


	int sign(float x);

	float pchipend(int h1, int h2, float del1, float del2);

	int Smooth_f0(float* out, int nframe);
};

#endif // _PITCH_H_

