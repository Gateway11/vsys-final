/*
* This software has been licensed to the Centre of Speech Technology, KTH
* by Microsoft Corp. with the terms in the accompanying file BSD.txt,
* which is a BSD style license.
*
*    "Copyright (c) 1990-1996 Entropic Research Laboratory, Inc. 
*                   All rights reserved"
*
* Written by:  Derek Lin
* Checked by:
* Revised by:  David Talkin
*
* Brief description:  Estimates F0 using normalized cross correlation and
*   dynamic programming.
*
*/

/*
* get_f0s.c
* 
* $Id: get_f0s.c,v 1.2 2006/10/16 04:50:17 sako Exp $
*
*/

//#include "StdAfx.h"

//#include "NormPitch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <vector>

#define GETF0_ERROR 1
#define GETF0_OK 0

#ifndef TRUE
# define TRUE 1
# define FALSE 0
#endif
#ifndef FLT_MAX
# define FLT_MAX (3.40282347E+38f) 
#endif
#ifndef M_PI
# define M_PI (3.1415926536f)
#endif
//#ifndef NULL_PITCH_DELAY 
//# define NULL_PITCH_DELAY (100)
//#endif
#include "Pitch.h"



//void free_dp_f0();
//static int check_f0_params( F0_params *par, double sample_freq);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Return a time-weighting window of type type and length n in dout.
* Dout is assumed to be at least n elements long.  Type is decoded in
* the switch statement below.
*/
int CF0::xget_window( register float *dout,
	register int n, register int type)
{
	//static float *din = NULL;
	//static int n0 = 0;
	float preemp = 0.0;

	if(n > n0) {
		register float *p;
		register int i;

		if(din) ckfree((void *)din);
		din = NULL;
		if(!(din = (float*)ckalloc(sizeof(float)*n))) {
			Fprintf(stderr,"Allocation problems in xget_window()\n");
			return(FALSE);
		}
		for(i=0, p=din; i++ < n; ) *p++ = 1;
		n0 = n;
	}
	return(window(din, dout, n, preemp, type));
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Apply a rectangular window (i.e. none).  Optionally, preemphasize. */
void CF0::xrwindow( register float *din,
	register float *dout, register int n, register float preemp
	)
{
	register float *p;

	/* If preemphasis is to be performed,  this assumes that there are n+1 valid
	samples in the input buffer (din). */
	if(preemp != 0.0) {
		for( p=din+1; n-- > 0; )
			*dout++ = (float)((*p++) - (preemp * *din++));
	} else {
		for( ; n-- > 0; )
			*dout++ =  *din++;
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Generate a cos^4 window, if one does not already exist. */
void CF0::xcwindow(
	register float *din,
	register float *dout,
	register int n, register float preemp)
{
	register int i;
	register float *p;
	//static int wsize = 0;
	//static float *wind=NULL;
	register float *q, co;

	if(wsize != n) {		/* Need to create a new cos**4 window? */
		register double arg, half=0.5;

		if(wind) wind = (float*)ckrealloc((void *)wind,n*sizeof(float));
		else wind = (float*)ckalloc(n*sizeof(float));
		wsize = n;
		for(i=0, arg=3.1415927*2.0/(wsize), q=wind; i < n; ) {
			co = (float) (half*(1.0 - cos((half + (double)i++) * arg)));
			*q++ = co * co * co * co;
		}
	}
	/* If preemphasis is to be performed,  this assumes that there are n+1 valid
	samples in the input buffer (din). */
	if(preemp != 0.0) {
		for(i=n, p=din+1, q=wind; i--; )
			*dout++ = (float) (*q++ * ((float)(*p++) - (preemp * *din++)));
	} else {
		for(i=n, q=wind; i--; )
			*dout++ = *q++ * *din++;
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Generate a Hamming window, if one does not already exist. */
void CF0::xhwindow( register float *din,
	register float *dout,
	register int n, register float  preemp)
{
	register int i;
	register float *p;
	//static int wsize = 0;
	//static float *wind=NULL;
	register float *q;

	if(wsize != n) {		/* Need to create a new Hamming window? */
		register double arg, half=0.5;

		if(wind) wind = (float*)ckrealloc((void *)wind,n*sizeof(float));
		else wind = (float*)ckalloc(n*sizeof(float));
		wsize = n;
		for(i=0, arg=3.1415927*2.0/(wsize), q=wind; i < n; )
			*q++ = (float) (.54 - .46 * cos((half + (double)i++) * arg));
	}
	/* If preemphasis is to be performed,  this assumes that there are n+1 valid
	samples in the input buffer (din). */
	if(preemp != 0.0) {
		for(i=n, p=din+1, q=wind; i--; )
			*dout++ = (float) (*q++ * ((float)(*p++) - (preemp * *din++)));
	} else {
		for(i=n, q=wind; i--; )
			*dout++ = *q++ * *din++;
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Generate a Hanning window, if one does not already exist. */
void CF0::xhnwindow( register float *din,
	register float *dout,
	register int n, register float preemp)
{
	register int i;
	register float *p;
	//static int wsize = 0;
	//static float *wind=NULL;
	register float *q;

	if(wsize != n) {		/* Need to create a new Hanning window? */
		register double arg, half=0.5;

		if(wind) wind = (float*)ckrealloc((void *)wind,n*sizeof(float));
		else wind = (float*)ckalloc(n*sizeof(float));
		wsize = n;
		for(i=0, arg=3.1415927*2.0/(wsize), q=wind; i < n; )
			*q++ = (float) (half - half * cos((half + (double)i++) * arg));
	}
	/* If preemphasis is to be performed,  this assumes that there are n+1 valid
	samples in the input buffer (din). */
	if(preemp != 0.0) {
		for(i=n, p=din+1, q=wind; i--; )
			*dout++ = (float) (*q++ * ((float)(*p++) - (preemp * *din++)));
	} else {
		for(i=n, q=wind; i--; )
			*dout++ = *q++ * *din++;
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Apply a window of type type to the short PCM sequence of length n
* in din.  Return the floating-point result sequence in dout.  If preemp
* is non-zero, apply preemphasis to tha data as it is windowed.
*/
int CF0::window( register float *din,
	register float *dout,
	register int n, register float preemp,
	int type)
{
	switch(type) {
	case 0:			/* rectangular */
		xrwindow(din, dout, n, preemp);
		break;
	case 1:			/* Hamming */
		xhwindow(din, dout, n, preemp);
		break;
	case 2:			/* cos^4 */
		xcwindow(din, dout, n, preemp);
		break;
	case 3:			/* Hanning */
		xhnwindow(din, dout, n, preemp);
		break;
	default:
		Fprintf(stderr,"Unknown window type (%d) requested in window()\n",type);
		return(FALSE);
	}
	return(TRUE);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Compute the pp+1 autocorrelation lags of the windowsize samples in s.
* Return the normalized autocorrelation coefficients in r.
* The rms is returned in e.
*/
void CF0::xautoc( 
	register int windowsize,
	register float *s, register int p,
	register float *r, register float *e)
{
	register int i, j;
	register float *q, *t, sum, sum0;

	for( i=windowsize, q=s, sum0=0.0; i--;) {
		sum = *q++;
		sum0 += sum*sum;
	}
	*r = 1.;			/* r[0] will always =1. */
	if(sum0 == 0.0) {		/* No energy: fake low-energy white noise. */
		*e = 1.;			/* Arbitrarily assign 1 to rms. */
		/* Now fake autocorrelation of white noise. */
		for ( i=1; i<=p; i++){
			r[i] = 0.;
		}
		return;
	}
	*e = (float) sqrt((double)(sum0/windowsize));
	sum0 = (float) (1.0/sum0);
	for( i=1; i <= p; i++){
		for( sum=0.0, j=windowsize-i, q=s, t=s+i; j--; )
			sum += (*q++) * (*t++);
		*(++r) = sum*sum0;
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Using Durbin's recursion, convert the autocorrelation sequence in r
* to reflection coefficients in k and predictor coefficients in a.
* The prediction error energy (gain) is left in *ex.
* Note: durbin returns the coefficients in normal sign format.
*	(i.e. a[0] is assumed to be = +1.)
*/
void CF0::xdurbin ( 
	/* analysis order */
	register float *r, register float *k, register float *a,register int p, register float *ex)
{
	float  bb[BIGSORD];
	register int i, j;
	register float e, s, *b = bb;

	e = *r;
	*k = -r[1]/e;
	*a = *k;
	e *= (float) (1. - (*k) * (*k));
	for ( i=1; i < p; i++){
		s = 0;
		for ( j=0; j<i; j++){
			s -= a[j] * r[i-j];
		}
		k[i] = ( s - r[i+1] )/e;
		a[i] = k[i];
		for ( j=0; j<=i; j++){
			b[j] = a[j];
		}
		for ( j=0; j<i; j++){
			a[j] += k[i] * b[i-j-1];
		}
		e *= (float) ( 1. - (k[i] * k[i]) );
	}
	*ex = e;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*  Compute the autocorrelations of the p LP coefficients in a. 
*  (a[0] is assumed to be = 1 and not explicitely accessed.)
*  The magnitude of a is returned in c.
*  2* the other autocorrelation coefficients are returned in b.
*/
void CF0::xa_to_aca ( 
	float *a, float*b, float*c,
	register int p)
{
	register float  s, *ap, *a0;
	register int  i, j;

	for ( s=1., ap=a, i = p; i--; ap++ )
		s += *ap * *ap;

	*c = s;
	for ( i = 1; i <= p; i++){
		s = a[i-1];
		for (a0 = a, ap = a+i, j = p-i; j--; )
			s += (*a0++ * *ap++);
		*b++ = (float) (2. * s);
	}

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Compute the Itakura LPC distance between the model represented
* by the signal autocorrelation (r) and its residual (gain) and
* the model represented by an LPC autocorrelation (c, b).
* Both models are of order p.
* r is assumed normalized and r[0]=1 is not explicitely accessed.
* Values returned by the function are >= 1.
*/
float CF0::xitakura (
	register int p,register float *b, register float *c,register float *r,register float *gain
	)
{
	register float s;

	for( s= *c; p--; )
		s += *r++ * *b++;

	return (s/ *gain);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Compute the time-weighted RMS of a size segment of data.  The data
* is weighted by a window of type w_type before RMS computation.  w_type
* is decoded above in window().
*/
float CF0::wind_energy(
	register float *data,	/* input PCM data */
	register int size,		/* size of window */
	register int   w_type)			/* window type */
{
	//static int nwind = 0;
	//static float *dwind = NULL;
	register float *dp, sum, f;
	register int i;

	if(nwind_ < size) {
		if(dwind_) dwind_ = (float*)ckrealloc((void *)dwind_,size*sizeof(float));
		else dwind_ = (float*)ckalloc(size*sizeof(float));
		if(!dwind_) {
			Fprintf(stderr,"Can't allocate scratch memory in wind_energy()\n");
			return(0.0);
		}
	}
	if(nwind_ != size) {
		xget_window(dwind_, size, w_type);
		nwind_ = size;
	}
	for(i=size, dp = dwind_, sum = 0.0; i-- > 0; ) {
		f = *dp++ * (float)(*data++);
		sum += f*f;
	}
	return((float)sqrt((double)(sum/size)));
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Generic autocorrelation LPC analysis of the short-integer data
* sequence in data.
*/
int CF0::xlpc(
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
	)	/* input data sequence; assumed to be wsize+1 long */
{
	//static float *dwind=NULL;
	//static int nwind=0;
	float rho[BIGSORD+1], k[BIGSORD], a[BIGSORD+1],*r,*kp,*ap,en,er,wfact=1.0;

	if((wsize <= 0) || (!data) || (lpc_ord > BIGSORD)) return(FALSE);

	if(nwind != wsize) {
		if(dwind) dwind = (float*)ckrealloc((void *)dwind,wsize*sizeof(float));
		else dwind = (float*)ckalloc(wsize*sizeof(float));
		if(!dwind) {
			Fprintf(stderr,"Can't allocate scratch memory in lpc()\n");
			return(FALSE);
		}
		nwind = wsize;
	}

	window(data, dwind, wsize, preemp, type);
	if(!(r = ar)) r = rho;	/* Permit optional return of the various */
	if(!(kp = lpck)) kp = k;	/* coefficients and intermediate results. */
	if(!(ap = lpca)) ap = a;
	xautoc( wsize, dwind, lpc_ord, r, &en );
	if(lpc_stabl > 1.0) {	/* add a little to the diagonal for stability */
		int i;
		float ffact;
		ffact = (float) (1.0/(1.0 + exp((-lpc_stabl/20.0) * log(10.0))));
		for(i=1; i <= lpc_ord; i++) rho[i] = ffact * r[i];
		*rho = *r;
		r = rho;
		if(ar)
			for(i=0;i<=lpc_ord; i++) ar[i] = r[i];
	}
	xdurbin ( r, kp, &ap[1], lpc_ord, &er);
	switch(type) {		/* rms correction for window */
	case 0:
		wfact = 1.0;		/* rectangular */
		break;
	case 1:
		wfact = .630397f;		/* Hamming */
		break;
	case 2:
		wfact = .443149f;		/* (.5 - .5*cos)^4 */
		break;
	case 3:
		wfact = .612372f;		/* Hanning */
		break;
	}
	*ap = 1.0;
	if(rms) *rms = en/wfact;
	if(normerr) *normerr = er;
	return(TRUE);
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Return a sequence based on the normalized crosscorrelation of the signal
in data.
*
data is the input speech array
size is the number of samples in each correlation
start is the first lag to compute (governed by the highest expected F0)
nlags is the number of cross correlations to compute (set by lowest F0)
engref is the energy computed at lag=0 (i.e. energy in ref. window)
maxloc is the lag at which the maximum in the correlation was found
maxval is the value of the maximum in the CCF over the requested lag interval
correl is the array of nlags cross-correlation coefficients (-1.0 to 1.0)
*
*/
void CF0::crossf( float *data, int size, int start, int nlags, float *engref, int *maxloc, float *maxval,float *correl )
{
	//static float *dbdata=NULL;
	//static int dbsize = 0;
	register float *dp, *ds, sum, st;
	register int j;
	register  float *dq, t, *p, engr, *dds, amax;
	register  double engc;
	int i, iloc, total;
	int sizei, sizeo, maxsize;

	/* Compute mean in reference window and subtract this from the
	entire sequence.  This doesn't do too much damage to the data
	sequenced for the purposes of F0 estimation and removes the need for
	more principled (and costly) low-cut filtering. */
	if((total = size+start+nlags) > dbsize_) {
		if(dbdata_)
			ckfree((void *)dbdata_);
		dbdata_ = NULL;
		dbsize_ = 0;
		if(!(dbdata_ = (float*)ckalloc(sizeof(float)*total))) {
			Fprintf(stderr,"Allocation failure in crossf()\n");
			return;/*exit(-1);*/
		}
		dbsize_ = total;
	}
	for(engr=0.0, j=size, p=data; j--; ) engr += *p++;
	engr /= size;
	for(j=size+nlags+start, dq = dbdata_, p=data; j--; )  *dq++ = *p++ - engr;

	maxsize = start + nlags;
	sizei = size + start + nlags + 1;
	sizeo = nlags + 1;

	/* Compute energy in reference window. */
	for(j=size, dp=dbdata_, sum=0.0; j--; ) {
		st = *dp++;
		sum += st * st;
	}

	*engref = engr = sum;
	if(engr > 0.0) {    /* If there is any signal energy to work with... */
		/* Compute energy at the first requested lag. */  
		for(j=size, dp=dbdata_+start, sum=0.0; j--; ) {
			st = *dp++;
			sum += st * st;
		}
		engc = sum;

		/* COMPUTE CORRELATIONS AT ALL OTHER REQUESTED LAGS. */
		for(i=0, dq=correl, amax=0.0, iloc = -1; i < nlags; i++) {
			for(j=size, sum=0.0, dp=dbdata_, dds = ds = dbdata_+i+start; j--; )
				sum += *dp++ * *ds++;
			*dq++ = t = (float) (sum/sqrt((double)(engc*engr))); /* output norm. CC */
			engc -= (double)(*dds * *dds); /* adjust norm. energy for next lag */
			if((engc += (double)(*ds * *ds)) < 1.0)
				engc = 1.0;		/* (hack: in case of roundoff error) */
			if(t > amax) {		/* Find abs. max. as we go. */
				amax = t;
				iloc = i+start;
			}
		}
		*maxloc = iloc;
		*maxval = amax;
	} else {	/* No energy in signal; fake reasonable return vals. */
		*maxloc = 0;
		*maxval = 0.0;
		for(p=correl,i=nlags; i-- > 0; )
			*p++ = 0.0;
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Return a sequence based on the normalized crosscorrelation of the
signal in data.  This is similar to crossf(), but is designed to
compute only small patches of the correlation sequence.  The length of
each patch is determined by nlags; the number of patches by nlocs, and
the locations of the patches is specified by the array locs.  Regions
of the CCF that are not computed are set to 0. 
*
data is the input speech array
size is the number of samples in each correlation
start0 is the first (virtual) lag to compute (governed by highest F0)
nlags0 is the number of lags (virtual+actual) in the correlation sequence
nlags is the number of cross correlations to compute at each location
engref is the energy computed at lag=0 (i.e. energy in ref. window)
maxloc is the lag at which the maximum in the correlation was found
maxval is the value of the maximum in the CCF over the requested lag interval
correl is the array of nlags cross-correlation coefficients (-1.0 to 1.0)
locs is an array of indices pointing to the center of a patches where the
cross correlation is to be computed.
nlocs is the number of correlation patches to compute.
*
*/
void CF0::crossfi( float *data, int size, int start0, int nlags0, int nlags, 
	float *engref, int *maxloc, float *maxval,float *correl,
	int *locs, int nlocs)
{
	//static float *dbdata=NULL;
	//static int dbsize = 0;
	register float *dp, *ds, sum, st;
	register int j;
	register  float *dq, t, *p, engr, *dds, amax;
	register  double engc;
	int i, iloc, start, total;

	/* Compute mean in reference window and subtract this from the
	entire sequence. */
	if((total = size+start0+nlags0) > dbsize) {
		if(dbdata)
			ckfree((void *)dbdata);
		dbdata = NULL;
		dbsize = 0;
		if(!(dbdata = (float*)ckalloc(sizeof(float)*total))) {
			Fprintf(stderr,"Allocation failure in crossfi()\n");
			return;/*exit(-1);*/
		}
		dbsize = total;
	}
	for(engr=0.0, j=size, p=data; j--; ) engr += *p++;
	engr /= size;
	/*  for(j=size+nlags0+start0, t = -2.1, amax = 2.1, dq = dbdata, p=data; j--; ) {
	if(((smax = *p++ - engr) > t) && (smax < amax))
	smax = 0.0;
	*dq++ = smax;
	} */
	for(j=size+nlags0+start0, dq = dbdata, p=data; j--; ) {
		*dq++ = *p++ - engr;
	}

	/* Zero the correlation output array to avoid confusing the peak
	picker (since all lags will not be computed). */
	for(p=correl,i=nlags0; i-- > 0; )
		*p++ = 0.0;

	/* compute energy in reference window */
	for(j=size, dp=dbdata, sum=0.0; j--; ) {
		st = *dp++;
		sum += st * st;
	}

	*engref = engr = sum;
	amax=0.0;
	iloc = -1;
	if(engr > 0.0) {
		for( ; nlocs > 0; nlocs--, locs++ ) {
			start = *locs - (nlags>>1);
			if(start < start0)
				start = start0;
			dq = correl + start - start0;
			/* compute energy at first requested lag */  
			for(j=size, dp=dbdata+start, sum=0.0; j--; ) {
				st = *dp++;
				sum += st * st;
			}
			engc = sum;

			/* COMPUTE CORRELATIONS AT ALL REQUESTED LAGS */
			for(i=0; i < nlags; i++) {
				for(j=size, sum=0.0, dp=dbdata, dds = ds = dbdata+i+start; j--; )
					sum += *dp++ * *ds++;
				if(engc < 1.0)
					engc = 1.0;		/* in case of roundoff error */
				*dq++ = t = (float) (sum/sqrt((double)(10000.0 + (engc*engr))));
				engc -= (double)(*dds * *dds);
				engc += (double)(*ds * *ds);
				if(t > amax) {
					amax = t;
					iloc = i+start;
				}
			}
		}
		*maxloc = iloc;
		*maxval = amax;
	} else {
		*maxloc = 0;
		*maxval = 0.0;
	}
}

///////////////////////////////////////////////////////////

/*	Round the argument to the nearest integer.			*/
int CF0::eround(register double flnum)
{
	return((flnum >= 0.0) ? (int)(flnum + 0.5) : (int)(flnum - 0.5));
}

/* IO buffer */
bufcell *CF0::bufcell_new(){
	bufcell *p;

	p = (bufcell *)malloc( sizeof( bufcell));
	if( p == NULL) return NULL;
	p->len=0;
	p->data = (short *) malloc( sizeof( short)*INBUF_LEN+1);
	if( p->data == NULL) return NULL;

	p->next = NULL;

	return p;
}

/* clear bufcell chain */
void CF0::bufcell_free( bufcell *p){
	bufcell *next;

	while( p != NULL) {
		next = p->next;
		free( p->data);
		p->data = NULL;
		free((float *) p);

		p = next;
	}
}

/* load wave data  */
int CF0::load_wavfile( wav_params *par){
	FILE *fp;
	int i, pos, total_len;
	bufcell *ibuf, *ibuf_head;

	ibuf_head = bufcell_new();
	ibuf = ibuf_head;

	if( par->file != NULL){
		fp = fopen( par->file, "r");

		if( !fp){
			fprintf( stderr, "can't open file (%s)\n", par->file);
			/* bufcell_free( ibuf); */
			return GETF0_ERROR;
		}
	}
	else{
		fp = stdin;
	}

	total_len = 0;
	for(;;){
		ibuf->len = fread( ibuf->data, sizeof(short), INBUF_LEN, fp);
		total_len += ibuf->len;

		if( ibuf->len < INBUF_LEN) break; /* end of file */
		ibuf->next = bufcell_new();
		if( ibuf->next == NULL) return GETF0_ERROR;
		ibuf = ibuf->next;
	} 

	par->length = total_len;

	pos = 0;
	if( par->padding){
		par->length += par->head_pad;
		par->length += par->tail_pad;
		pos = par->head_pad;
	}

	par->data = (float *) malloc( sizeof(float)*par->length);
	memset( par->data, 0, sizeof(float)*par->length);

	ibuf = ibuf_head;
	for(;;){
		for( i=0; i<ibuf->len; i++){
			par->data[pos++] = (float)ibuf->data[i];
		}
		if( ibuf->next == NULL) break;
		ibuf = ibuf->next;
	}

	bufcell_free(ibuf_head);

	if( par->nan == -1) par->nan = par->length;

	return GETF0_OK;
}

/* 2byte byte swap */
void CF0::swap2w( char *p){
	char p0,p1;

	p0 = p[0];
	p1 = p[1];

	p[0] = p1;
	p[1] = p0;
}

/*  */
int CF0::Get_SoundData( wav_params *par, int ndone , float *fdata, int actsize){
	int i, rs;
	rs = actsize;

	if( ndone + actsize > par->length)
		rs = par->length - ndone;

	for( i=0; i<rs; i++)
		fdata[i] = (float) par->data[i+ndone];

	return rs;  
}

/* init output data structure */
int CF0::init_out_params( out_params *par, int len){

	if( !(len >0)) return GETF0_ERROR;

	if( (par->f0p = (float *)malloc( sizeof(float) * len)) == NULL)
		return GETF0_ERROR;
	if( (par->vuvp = (float *)malloc( sizeof(float) * len)) == NULL)
		return GETF0_ERROR;
	if( (par->rms_speech = (float *)malloc( sizeof(float) * len)) == NULL)
		return GETF0_ERROR;
	if( (par->acpkp = (float *)malloc( sizeof(float) * len)) == NULL)
		return GETF0_ERROR;

	return GETF0_OK;
}

void CF0::free_out_params( out_params *par){
	free( par->f0p);
	free( par->vuvp);
	free( par->rms_speech);
	free( par->acpkp);

	free( par);
	par = NULL;
}


/* show usage and exit */
//void usage(){
//	fprintf( stderr, "\n");
//	fprintf( stderr, "get_f0s ... Estimates F0 using normalized cross correlation and dynamic\n            programming (functionaly the same as the ESPS get_f0)\n\n");
//	fprintf( stderr, "usage : get_f0s [options] input-file\n");
//	fprintf( stderr, "\n");  
//	fprintf( stderr, "[input-file]\n");
//	fprintf( stderr, "  raw audio file : sampling rate=16000, mono\n");
//	fprintf( stderr, "  (if it is not assigned then stdin)\n");
//	fprintf( stderr, "\n");
//	fprintf( stderr, "[options]\n");
//	fprintf( stderr, "  -b       : output binary                            [off]\n");
//	fprintf( stderr, "  -v       : verbose mode                             [off]\n");
//	fprintf( stderr, "  -x       : byte swap                                [off]\n");
//	fprintf( stderr, "  -z       : SPTK like framing                        [off]\n");
//	fprintf( stderr, "  -h       : show this message                             \n");
//	fprintf( stderr, "  -o str   : output filename                         [NULL]\n");
//	fprintf( stderr, "  -s float : frame step (sec)                        [0.01]\n");
//	fprintf( stderr, "  -l float : frame length (sec)                    [0.0075]\n");
//	fprintf( stderr, "  -r int   : sampling rate (Hz)                     [16000]\n");
//	fprintf( stderr, "  -U int   : max F0 (Hz)                              [550]\n");
//	fprintf( stderr, "  -L int   : min F0 (Hz)                               [50]\n");
//	fprintf( stderr, "  -Z int   : N frame tail-padding                     [off]\n");
//	fprintf( stderr, "\n");
//	//    fprintf( stderr, " %s ver.%s\n", PACKAGE_NAME, PACKAGE_VERSION);
//	//    fprintf( stderr, " Report bugs to <%s>\n", PACKAGE_BUGREPORT);
//	exit(1);
//}

/* output result */
int CF0::dump_output( char *filename, out_params *par, int full, int binary){
	FILE *fp;
	int i;

	if( filename == NULL){
		fp = stdout;
	}
	else{
		fp = fopen( filename, "w");
		if( fp == NULL) return GETF0_ERROR;
	}

	for( i=0; i<par->nframe; i++){
		if( full == 1){
			fprintf( fp, "%f\t%f\t%f\t%f\n",
				par->f0p[i], 
				par->vuvp[i],
				par->rms_speech[i],
				par->acpkp[i]);
		}
		else{
			if( binary == 1){
				fwrite( &(par->f0p[i]), sizeof(float), 1, fp);
			}
			else{
				fprintf( fp, "%f\n", par->f0p[i]);
			}
		}
	}

	if( fp != NULL) fclose( fp);

	return GETF0_OK;
}



/* functionaly the same as the ESPS get_f0 */
int
	CF0::Get_f0(wav_params *wpar, F0_params *par, out_params *opar){
		float *fdata;
		int length;
		int done;
		long buff_size, actsize;
		double sf;
		//F0_params *read_f0_params();
		float *f0p, *vuvp, *rms_speech, *acpkp;
		int i, vecsize;
		//int init_dp_f0(double,  F0_params*,  long*, long*), 
		//	dp_f0(float*, int, int,double, F0_params*, float**,float**,float**,float**, int*, int );
		long sdstep = 0, total_samps;
		int ndone = 0;

		double framestep2 = 0.0;

		int  startpos = 0, endpos = -1;

		length = wpar->length;
		startpos = wpar->startpos;
		endpos = wpar->nan - wpar->startpos;

		if (startpos < 0) startpos = 0;
		if (endpos >= (length - 1) || endpos == -1)
			endpos = length - 1;
		if (startpos > endpos) return GETF0_OK;

		sf = (double) wpar->rate;

		if (framestep > 0)  /* If a value was specified with -S, use it. */
			par->frame_step = (float) (framestep / sf);

		if(check_f0_params(par, sf)){
			return GETF0_ERROR;
		}

		total_samps = endpos - startpos + 1;
		if(total_samps < ((par->frame_step * 2.0) + par->wind_dur) * sf) {
			return GETF0_ERROR;
		}
		/* Initialize variables in get_f0.c; allocate data structures;
		* determine length and overlap of input frames to read.
		*/
		if (init_dp_f0(sf, par, &buff_size, &sdstep)
			|| buff_size > INT_MAX || sdstep > INT_MAX)
		{
			return GETF0_ERROR;
		}

		if (buff_size > total_samps)
			buff_size = total_samps;

		actsize = min(buff_size, length);
		//fdata = (float *) malloc(sizeof(float) * max(buff_size, sdstep));
		fdata = new float [ max(buff_size, sdstep) ];
		memset( fdata, 0, sizeof(float)* max(buff_size, sdstep) );

		ndone = startpos;

		while (TRUE) {
			done = (actsize < buff_size) || (total_samps == buff_size);
			/* Snack_GetSoundData(sound, ndone, fdata, actsize); */
			Get_SoundData( wpar, ndone, fdata, actsize);

			/*if (sound->debug > 0) Snack_WriteLog("dp_f0...\n");*/
			if (dp_f0(fdata, (int) actsize, (int) sdstep, sf, par,
				&f0p, &vuvp, &rms_speech, &acpkp, &vecsize, done)) {

					return GETF0_ERROR;
			}
			for( i=vecsize-1; i>=0; i--){
				opar->f0p[ opar->nframe] = f0p[i];
				opar->vuvp[ opar->nframe] = vuvp[i];
				opar->rms_speech[ opar->nframe] = rms_speech[i];
				opar->acpkp[ opar->nframe] = acpkp[i];
				opar->nframe++;
			}

			if (done) break;

			ndone += sdstep; 
			actsize = min(buff_size, length - ndone);
			total_samps -= sdstep;
			if (actsize > total_samps)
				actsize = total_samps;

		}

		//		printf("free_fdata begin\n");
		//free((void *)fdata);
		delete [] fdata;
		// free((void *)par);

		//		printf("free_dp_f0 begin\n");
		free_dp_f0();
		//		printf("free_dp_f0 end\n");

		return GETF0_OK;
}


/*
* Some consistency checks on parameter values.
* Return a positive integer if any errors detected, 0 if none.
*/

int CF0::check_f0_params( F0_params *par, double sample_freq)
{
	int	  error = 0;
	double  dstep;

	if((par->cand_thresh < 0.01) || (par->cand_thresh > 0.99)) {
		error++;
	}
	if((par->wind_dur > .1) || (par->wind_dur < .0001)) {
		error++;
	}
	if((par->n_cands > 100) || (par->n_cands < 3)){
		error++;
	}
	if((par->max_f0 <= par->min_f0) || (par->max_f0 >= (sample_freq/2.0)) ||
		(par->min_f0 < (sample_freq/10000.0))){
			error++;
	}
	dstep = ((double)((int)(0.5 + (sample_freq * par->frame_step))))/sample_freq;
	if(dstep != par->frame_step) {
		if(debug_level)
			par->frame_step = (float) dstep;
	}
	if((par->frame_step > 0.1) || (par->frame_step < (1.0/sample_freq))){
		error++;
	}

	return(error);
}
//static void get_cand(Cross *cross,float *peak,int *loc,int nlags,int *ncand,float cand_thresh), peak(float *y, float *xp, float *yp), do_ffir(register float* buf,register int in_samps,register float *bufo,register int *out_samps, int idx, register int ncoef,float *fc, register int invert,register int skip, register int init
//	);
//
//static int lc_lin_fir(register float fc,int *nf,float *coef), downsamp(float *in, float *out, int samples, int *outsamps, int state_idx, int decimate, int ncoef, float fc[], int init) ;
//
/* ----------------------------------------------------------------------- */
void CF0::get_fast_cands( float *fdata, float *fdsdata,  int ind, int step,  int size, int dec, int start, int nlags,
	float *engref, int *maxloc,float *maxval, Cross *cp, float *peaks,  int *locs, int *ncand,  F0_params *par)
{
	int decind, decstart, decnlags, decsize, i, j, *lp;
	float *corp, xp, yp, lag_wt;
	register float *pe;

	lag_wt = par->lag_weight/nlags;
	decnlags = 1 + (nlags/dec);
	if((decstart = start/dec) < 1) decstart = 1;
	decind = (ind * step)/dec;
	decsize = 1 + (size/dec);
	corp = cp->correl;

	crossf(fdsdata + decind, decsize, decstart, decnlags, engref, maxloc,
		maxval, corp);
	cp->maxloc = *maxloc;	/* location of maximum in correlation */
	cp->maxval = *maxval;	/* max. correlation value (found at maxloc) */
	cp->rms = (float) sqrt(*engref/size); /* rms in reference window */
	cp->firstlag = decstart;

	get_cand(cp,peaks,locs,decnlags,ncand,par->cand_thresh); /* return high peaks in xcorr */

	/* Interpolate to estimate peak locations and values at high sample rate. */
	for(i = *ncand, lp = locs, pe = peaks; i--; pe++, lp++) {
		j = *lp - decstart - 1;
		peak(&corp[j],&xp,&yp);
		*lp = (*lp * dec) + (int)(0.5+(xp*dec)); /* refined lag */
		*pe = yp*(1.0f - (lag_wt* *lp)); /* refined amplitude */
	}

	if(*ncand >= par->n_cands) {	/* need to prune candidates? */
		register int *loc, *locm, lt;
		register float smaxval, *pem;
		register int outer, inner, lim;
		for(outer=0, lim = par->n_cands-1; outer < lim; outer++)
			for(inner = *ncand - 1 - outer,
				pe = peaks + (*ncand) -1, pem = pe-1,
				loc = locs + (*ncand) - 1, locm = loc-1;
		inner--;
		pe--,pem--,loc--,locm--)
			if((smaxval = *pe) > *pem) {
				*pe = *pem;
				*pem = smaxval;
				lt = *loc;
				*loc = *locm;
				*locm = lt;
			}
			*ncand = par->n_cands-1;  /* leave room for the unvoiced hypothesis */
	}
	crossfi(fdata + (ind * step), size, start, nlags, 7, engref, maxloc,
		maxval, corp, locs, *ncand);

	cp->maxloc = *maxloc;	/* location of maximum in correlation */
	cp->maxval = *maxval;	/* max. correlation value (found at maxloc) */
	cp->rms = (float) sqrt(*engref/size); /* rms in reference window */
	cp->firstlag = start;
	get_cand(cp,peaks,locs,nlags,ncand,par->cand_thresh); /* return high peaks in xcorr */
	if(*ncand >= par->n_cands) {	/* need to prune candidates again? */
		register int *loc, *locm, lt;
		register float smaxval, *pe, *pem;
		register int outer, inner, lim;
		for(outer=0, lim = par->n_cands-1; outer < lim; outer++)
			for(inner = *ncand - 1 - outer,
				pe = peaks + (*ncand) -1, pem = pe-1,
				loc = locs + (*ncand) - 1, locm = loc-1;
		inner--;
		pe--,pem--,loc--,locm--)
			if((smaxval = *pe) > *pem) {
				*pe = *pem;
				*pem = smaxval;
				lt = *loc;
				*loc = *locm;
				*locm = lt;
			}
			*ncand = par->n_cands - 1;  /* leave room for the unvoiced hypothesis */
	}
}

/* ----------------------------------------------------------------------- */
float *CF0::downsample(float *input,int samsin,int state_idx, double freq, int *samsout, int decimate, int first_time, int last_time)
{
	//static float	b[2048];
	//static float *foutput = NULL;
	float	beta = 0.0f;
	//static int	ncoeff = 127, ncoefft = 0;
	int init;

	if(input && (samsin > 0) && (decimate > 0) && *samsout) {
		if(decimate == 1) {
			return(input);
		}

		if(first_time){
			int nbuff = (samsin/decimate) + (2*ncoeff);

			ncoeff = ((int)(freq * .005)) | 1;
			beta = .5f/decimate;
			foutput = (float*)realloc((void *)foutput, sizeof(float) * nbuff);
			/*      spsassert(foutput, "Can't allocate foutput in downsample");*/
			for( ; nbuff > 0 ;)
				foutput[--nbuff] = 0.0;

			if( !lc_lin_fir(beta,&ncoeff,b)) {
				fprintf(stderr,"\nProblems computing interpolation filter\n");
				free((void *)foutput);
				return(NULL);
			}
			ncoefft = (ncoeff/2) + 1;
		}		    /*  endif new coefficients need to be computed */

		if(first_time) init = 1;
		else if (last_time) init = 2;
		else init = 0;

		if(downsamp(input,foutput,samsin,samsout,state_idx,decimate,ncoefft,b,init)) {
			return(foutput);
		} else
			Fprintf(stderr,"Problems in downsamp() in downsample()\n");
	} else
		Fprintf(stderr,"Bad parameters passed to downsample()\n");

	return(NULL);
}

/* ----------------------------------------------------------------------- */
/* Get likely candidates for F0 peaks. */
void CF0::get_cand(Cross *cross,float *peak,int *loc,int nlags,int *ncand,float cand_thresh)
{
	register int i, lastl, *t;
	register float o, p, q, *r, *s, clip;
	int start, ncan, maxl;

	clip = (float) (cand_thresh * cross->maxval);
	maxl = cross->maxloc;
	lastl = nlags - 2;
	start = cross->firstlag;

	r = cross->correl;
	o= *r++;			/* first point */
	q = *r++;	                /* middle point */
	p = *r++;
	s = peak;
	t = loc;
	ncan=0;
	for(i=1; i < lastl; i++, o=q, q=p, p= *r++){
		if((q > clip) &&		/* is this a high enough value? */
			(q >= p) && (q >= o)){ /* NOTE: this finds SHOLDERS and PLATEAUS
								   as well as peaks (is this a good idea?) */
				*s++ = q;		/* record the peak value */
				*t++ = i + start;	/* and its location */
				ncan++;			/* count number of peaks found */
		}
	}
	/*
	o = q;
	q = p;
	if( (q > clip) && (q >=0)){
	*s++ = q;
	*t++ = i+start;
	ncan++;
	}
	*/
	*ncand = ncan;
}

/* ----------------------------------------------------------------------- */
/* buffer-to-buffer downsample operation */
/* This is STRICTLY a decimator! (no upsample) */
int CF0::downsamp(float *in, float *out, int samples, int *outsamps, int state_idx, int decimate, int ncoef, float fc[], int init)
{
	if(in && out) {
		do_ffir(in, samples, out, outsamps, state_idx, ncoef, fc, 0, decimate, init);
		return(TRUE);
	} else
		printf("Bad signal(s) passed to downsamp()\n");
	return(FALSE);
}

/*      ----------------------------------------------------------      */
void CF0::do_ffir(register float *buf,register int in_samps,register float *bufo, 
	register int *out_samps,int idx, register int ncoef,float *fc,
	register int invert,register int skip,register int init)
	/* fc contains 1/2 the coefficients of a symmetric FIR filter with unity
	passband gain.  This filter is convolved with the signal in buf.
	The output is placed in buf2.  If(float *in, float *out, int samples, int *outsamps, int state_idx, int decimate, int ncoef, float fc[], int init)
	(invert), the filter magnitude
	response will be inverted.  If(init&1), beginning of signal is in buf;
	if(init&2), end of signal is in buf.  out_samps is set to the number of
	output points placed in bufo. */
{
	register float *dp1, *dp2, *dp3, sum, integral;
	//static float *co=NULL, *mem=NULL;
	//static float state[1000];
	//static int fsize=0, resid=0;
	register int i, j, k, l;
	register float *sp;
	register float *buf1;

	buf1 = buf;
	if(ncoef > fsize) {/*allocate memory for full coeff. array and filter memory */    fsize = 0;
	i = (ncoef+1)*2;
	if(!((co = (float *)realloc((void *)co, sizeof(float)*i)) &&
		(mem_ = (float *)realloc((void *)mem_, sizeof(float)*i)))) {
			fprintf(stderr,"allocation problems in do_fir()\n");
			return;
	}
	fsize = ncoef;
	}

	/* fill 2nd half with data */
	for(i=ncoef, dp1=mem_+ncoef-1; i-- > 0; )  *dp1++ = *buf++;  

	if(init & 1) {	/* Is the beginning of the signal in buf? */
		/* Copy the half-filter and its mirror image into the coefficient array. */
		for(i=ncoef-1, dp3=fc+ncoef-1, dp2=co, dp1 = co+((ncoef-1)*2),
			integral = 0.0; i-- > 0; )
			if(!invert) *dp1-- = *dp2++ = *dp3--;
			else {
				integral += (sum = *dp3--);
				*dp1-- = *dp2++ = -sum;
			}
			if(!invert)  *dp1 = *dp3;	/* point of symmetry */
			else {
				integral *= 2;
				integral += *dp3;
				*dp1 = integral - *dp3;
			}

			for(i=ncoef-1, dp1=mem_; i-- > 0; ) *dp1++ = 0;
	}
	else
		for(i=ncoef-1, dp1=mem_, sp=state; i-- > 0; ) *dp1++ = *sp++;

	i = in_samps;
	resid = 0;

	k = (ncoef << 1) -1;	/* inner-product loop limit */

	if(skip <= 1) {       /* never used */
		/*    *out_samps = i;	
		for( ; i-- > 0; ) {	
		for(j=k, dp1=mem_, dp2=co, dp3=mem_+1, sum = 0.0; j-- > 0;
		*dp1++ = *dp3++ )
		sum += *dp2++ * *dp1;

		*--dp1 = *buf++;	
		*bufo++ = (sum < 0.0)? sum -0.5 : sum +0.5; 
		}
		if(init & 2) {	
		for(i=ncoef; i-- > 0; ) {
		for(j=k, dp1=mem_, dp2=co, dp3=mem_+1, sum = 0.0; j-- > 0;
		*dp1++ = *dp3++ )
		sum += *dp2++ * *dp1;
		*--dp1 = 0.0;
		*bufo++ = (sum < 0)? sum -0.5 : sum +0.5; 
		}
		*out_samps += ncoef;
		}
		return;
		*/
	} 
	else {			/* skip points (e.g. for downsampling) */
		/* the buffer end is padded with (ncoef-1) data points */
		for( l=0 ; l < *out_samps; l++ ) {
			for(j=k-skip, dp1=mem_, dp2=co, dp3=mem_+skip, sum=0.0; j-- >0;
				*dp1++ = *dp3++)
				sum += *dp2++ * *dp1;
			for(j=skip; j-- >0; *dp1++ = *buf++) /* new data to memory */
				sum += *dp2++ * *dp1;
			*bufo++ = (sum<0.0) ? sum -0.5f : sum +0.5f;
		}
		if(init & 2){
			resid = in_samps - *out_samps * skip;
			for(l=resid/skip; l-- >0; ){
				for(j=k-skip, dp1=mem_, dp2=co, dp3=mem_+skip, sum=0.0; j-- >0;
					*dp1++ = *dp3++)
					sum += *dp2++ * *dp1;
				for(j=skip; j-- >0; *dp1++ = 0.0)
					sum += *dp2++ * *dp1;
				*bufo++ = (sum<0.0) ? sum -0.5f : sum +0.5f;
				(*out_samps)++;
			}
		}
		else
			for(dp3=buf1+idx-ncoef+1, l=ncoef-1, sp=state; l-- >0; ) *sp++ = *dp3++;
	}
}

/*      ----------------------------------------------------------      */
int CF0::lc_lin_fir(register float fc,int *nf,float *coef)
	/* create the coefficients for a symmetric FIR lowpass filter using the
	window technique with a Hanning window. */
{
	register int	i, n;
	register double	twopi, fn, c;

	if(((*nf % 2) != 1))
		*nf = *nf + 1;
	n = (*nf + 1)/2;

	/*  Compute part of the ideal impulse response (the sin(x)/x kernel). */
	twopi = M_PI * 2.0;
	coef[0] = (float) (2.0 * fc);
	c = M_PI;
	fn = twopi * fc;
	for(i=1;i < n; i++) coef[i] = (float)(sin(i * fn)/(c * i));

	/* Now apply a Hanning window to the (infinite) impulse response. */
	/* (Probably should use a better window, like Kaiser...) */
	fn = twopi/(double)(*nf);
	for(i=0;i<n;i++) 
		coef[n-i-1] *= (float)((.5 - (.5 * cos(fn * ((double)i + 0.5)))));

	return(TRUE);
}


/* ----------------------------------------------------------------------- */
/* Use parabolic interpolation over the three points defining the peak
* vicinity to estimate the "true" peak. */
void CF0::peak(float *y, float *xp, float *yp)
{
	register float a, c;

	a = (float)((y[2]-y[1])+(.5*(y[0]-y[2])));
	if(fabs(a) > .000001) {
		*xp = c = (float)((y[0]-y[2])/(4.0*a));
		*yp = y[1] - (a*c*c);
	} else {
		*xp = 0.0;
		*yp = y[1];
	}
}

/* A fundamental frequency estimation algorithm using the normalized
cross correlation function and dynamic programming.  The algorithm
implemented here is similar to that presented by B. Secrest and
G. Doddington, "An integrated pitch tracking algorithm for speech
systems", Proc. ICASSP-83, pp.1352-1355.  It is fully described
by D. Talkin, "A robust algorithm for ptich tracking (RAPT)", in
W. B. Kleijn & K. K. Paliwal (eds.) Speech Coding and Synthesis,
(New York: Elsevier, 1995). */

/* For each frame, up to par->n_cands cross correlation peaks are
considered as F0 intervals.  Each is scored according to its within-
frame properties (relative amplitude, relative location), and
according to its connectivity with each of the candidates in the
previous frame.  An unvoiced hypothesis is also generated at each
frame and is considered in the light of voicing state change cost,
the quality of the cross correlation peak, and frequency continuity. */

/* At each frame, each candidate has associated with it the following
items:
its peak value
its peak value modified by its within-frame properties
its location
the candidate # in the previous frame yielding the min. err.
(this is the optimum path pointer!)
its cumulative cost: (local cost + connectivity cost +
cumulative cost of its best-previous-frame-match). */

/* Dynamic programming is then used to pick the best F0 trajectory and voicing
state given the local and transition costs for the entire utterance. */

/* To avoid the necessity of computing the full crosscorrelation at
the input sample rate, the signal is downsampled; a full ccf is
computed at the lower frequency; interpolation is used to estimate the
location of the peaks at the higher sample rate; and the fine-grained
ccf is computed only in the vicinity of these estimated peak
locations. */


/*
* READ_SIZE: length of input data frame in sec to read
* DP_CIRCULAR: determines the initial size of DP circular buffer in sec
* DP_HIST: stored frame history in second before checking for common path 
*      DP_CIRCULAR > READ_SIZE, DP_CIRCULAR at least 2 times of DP_HIST 
* DP_LIMIT: in case no convergence is found, DP frames of DP_LIMIT secs
*      are kept before output is forced by simply picking the lowest cost
*      path
*/

#define READ_SIZE 0.2
#define DP_CIRCULAR 1.5
#define DP_HIST 0.5
#define DP_LIMIT 1.0

/* 
* stationarity parameters -
* STAT_WSIZE: window size in sec used in measuring frame energy/stationarity
* STAT_AINT: analysis interval in sec in measuring frame energy/stationarity
*/
#define STAT_WSIZE 0.030
#define STAT_AINT 0.020


/*--------------------------------------------------------------------*/
int CF0::get_Nframes(long buffsize, int pad,int  step)
{
	if (buffsize < pad)
		return (0);
	else
		return ((buffsize - pad)/step);
}


/*--------------------------------------------------------------------*/
int CF0::init_dp_f0(double freq,F0_params* par, long *buffsize, long *sdstep)
{
	int nframes;
	int i;
	int stat_wsize, agap, ind, downpatch;

	/*
	* reassigning some constants 
	*/

	tcost = par->trans_cost;
	tfact_a = par->trans_amp;
	tfact_s = par->trans_spec;
	vbias = par->voice_bias;
	fdouble = par->double_cost;
	frame_int = par->frame_step;

	step = eround(frame_int * freq);
	size = eround(par->wind_dur * freq);
	frame_int = (float)(((float)step)/freq);
	wdur = (float)(((float)size)/freq);
	start = eround(freq / par->max_f0);
	stop = eround(freq / par->min_f0);
	nlags = stop - start + 1;
	ncomp = size + stop + 1; /* # of samples required by xcorr
							 comp. per fr. */
	maxpeaks = 2 + (nlags/2);	/* maximum number of "peaks" findable in ccf */
	ln2 = (float)log(2.0);
	size_frame_hist = (int) (DP_HIST / frame_int);
	size_frame_out = (int) (DP_LIMIT / frame_int);

	/*
	* SET UP THE D.P. WEIGHTING FACTORS:
	*      The intent is to make the effectiveness of the various fudge factors
	*      independent of frame rate or sampling frequency.                
	*/

	/* Lag-dependent weighting factor to emphasize early peaks (higher freqs)*/
	lagwt = par->lag_weight/stop;

	/* Penalty for a frequency skip in F0 per frame */
	freqwt = par->freq_weight/frame_int;

	i = (int) (READ_SIZE *freq);
	if(ncomp >= step) nframes = ((i-ncomp)/step ) + 1;
	else nframes = i / step;

	/* *buffsize is the number of samples needed to make F0 computation
	of nframes DP frames possible.  The last DP frame is patched with
	enough points so that F0 computation on it can be carried.  F0
	computaion on each frame needs enough points to do

	1) xcross or cross correlation measure:
	enough points to do xcross - ncomp

	2) stationarity measure:
	enough to make 30 msec windowing possible - ind

	3) downsampling:
	enough to make filtering possible -- downpatch

	So there are nframes whole DP frames, padded with pad points
	to make the last frame F0 computation ok.

	*/

	/* last point in data frame needs points of 1/2 downsampler filter length 
	long, 0.005 is the filter length used in downsampler */
	downpatch = (((int) (freq * 0.005))+1) / 2;

	stat_wsize = (int) (STAT_WSIZE * freq);
	agap = (int) (STAT_AINT * freq);
	ind = ( agap - stat_wsize ) / 2;
	i = stat_wsize + ind;
	pad = downpatch + ((i>ncomp) ? i:ncomp);
	*buffsize = nframes * step + pad;
	*sdstep = nframes * step;

	/* Allocate space for the DP storage circularly linked data structure */

	size_cir_buffer = (int) (DP_CIRCULAR / frame_int);

	/* creating circularly linked data structures */
	tailF = alloc_frame(nlags, par->n_cands);
	headF = tailF;

	/* link them up */
	for(i=1; i<size_cir_buffer; i++){
		headF->next = alloc_frame(nlags, par->n_cands);
		headF->next->prev = headF;
		headF = headF->next;
	}
	headF->next = tailF;
	tailF->prev = headF;

	headF = tailF;

	/* Allocate sscratch array to use during backtrack convergence test. */
	if( ! pcands ) {
		pcands = (int *) malloc( par->n_cands * sizeof(int));
		/*    spsassert(pcands,"can't allocate pathcands");*/
	}

	/* Allocate arrays to return F0 and related signals. */

	/* Note: remember to compare *vecsize with size_frame_out, because
	size_cir_buffer is not constant */
	output_buf_size = size_cir_buffer;
	rms_speech = (float*)malloc(sizeof(float) * output_buf_size);
	/*  spsassert(rms_speech,"rms_speech ckalloc failed");*/
	f0p = (float*)malloc(sizeof(float) * output_buf_size);
	/*  spsassert(f0p,"f0p ckalloc failed");*/
	vuvp = (float*)malloc(sizeof(float)* output_buf_size);
	/*  spsassert(vuvp,"vuvp ckalloc failed");*/
	acpkp = (float*)malloc(sizeof(float) * output_buf_size);
	/*  spsassert(acpkp,"acpkp ckalloc failed");*/

	/* Allocate space for peak location and amplitude scratch arrays. */
	g_peaks = (float*)malloc(sizeof(float) * maxpeaks * 2);
	//	printf("maxpeaks = %d\n", maxpeaks); 

	locs = (int*)malloc(sizeof(int) * maxpeaks * 2);
	/*  spsassert(locs, "locs ckalloc failed");*/

	/* Initialise the retrieval/saving scheme of window statistic measures */
	wReuse = agap / step;
	if (wReuse){
		windstat = (Windstat *) malloc( wReuse * sizeof(Windstat));
		/*      spsassert(windstat, "windstat ckalloc failed");*/
		for(i=0; i<wReuse; i++){
			windstat[i].err = 0;
			windstat[i].rms = 0;
		}
	}

	if(debug_level){
		Fprintf(stderr, "done with initialization:\n");
		Fprintf(stderr,
			" size_cir_buffer:%d  xcorr frame size:%d start lag:%d nlags:%d\n",
			size_cir_buffer, size, start, nlags);
	}

	num_active_frames = 0;
	first_time = 1;

	return(0);
}

//static Stat *get_stationarity(float *fdata, double freq, int buff_size, int nframes, int frame_step,int first_time);

/*--------------------------------------------------------------------*/
int CF0::dp_f0(float *fdata, int buff_size, int sdstep,double freq,
	F0_params*	par, float **f0p_pt, float **vuvp_pt, float **rms_speech_pt, float **acpkp_pt, int *vecsize, int last_time)
{
	float  maxval, engref, *sta, *rms_ratio, *dsdata;
		//, *downsample(float *input,int samsin,int state_idx, double freq, int *samsout, int decimate, int first_time, int last_time);
	register float ttemp, ftemp, ft1, ferr, err, errmin;
	register int  i, j, k, loc1, loc2;
	int   nframes, maxloc, ncand, ncandp, minloc,
		decimate, samsds;

	Stat *stat = NULL;

	nframes = get_Nframes((long) buff_size, pad, step); /* # of whole frames */

	if(debug_level)
		Fprintf(stderr,
		"******* Computing %d dp frames ******** from %d points\n", nframes, buff_size);

	/* Now downsample the signal for coarse peak estimates. */

	decimate = (int)(freq/2000.0);    /* downsample to about 2kHz */
	if (decimate <= 1)
		dsdata = fdata;
	else {
		samsds = ((nframes-1) * step + ncomp) / decimate;
		dsdata = downsample(fdata, buff_size, sdstep, freq, &samsds, decimate, 
			first_time, last_time);
		if (!dsdata) {
			Fprintf(stderr, "can't get downsampled data.\n");
			return 1;
		}
	}

	/* Get a function of the "stationarity" of the speech signal. */

	stat = get_stationarity(fdata, freq, buff_size, nframes, step, first_time);
	if (!stat) { 
		Fprintf(stderr, "can't get stationarity\n");
		return(1);
	}
	sta = stat->stat;
	rms_ratio = stat->rms_ratio;

	/***********************************************************************/
	/* MAIN FUNDAMENTAL FREQUENCY ESTIMATION LOOP */
	/***********************************************************************/
	if(!first_time && nframes > 0) headF = headF->next;

	for(i = 0; i < nframes; i++) {

		/* NOTE: This buffer growth provision is probably not necessary.
		It was put in (with errors) by Derek Lin and apparently never
		tested.  My tests and analysis suggest it is completely
		superfluous. DT 9/5/96 */
		/* Dynamically allocating more space for the circular buffer */
		if(headF == tailF->prev){
			Frame *frm;

			if(cir_buff_growth_count > 5){
				Fprintf(stderr,
					"too many requests (%d) for dynamically allocating space.\n   There may be a problem in finding converged path.\n",cir_buff_growth_count);
				return(1);
			}
			if(debug_level) 
				Fprintf(stderr, "allocating %d more frames for DP circ. buffer.\n", size_cir_buffer);
			frm = alloc_frame(nlags, par->n_cands);
			headF->next = frm;
			frm->prev = headF;
			for(k=1; k<size_cir_buffer; k++){
				frm->next = alloc_frame(nlags, par->n_cands);
				frm->next->prev = frm;
				frm = frm->next;
			}
			frm->next = tailF;
			tailF->prev = frm;
			cir_buff_growth_count++;
		}

		headF->rms = stat->rms[i];
		get_fast_cands(fdata, dsdata, i, step, size, decimate, start,
			nlags, &engref, &maxloc,
			&maxval, headF->cp, g_peaks, locs, &ncand, par);

		/*    Move the peak value and location arrays into the dp structure */
		{
			register float *ftp1, *ftp2;
			register short *sp1;
			register int *sp2;

			for(ftp1 = headF->dp->pvals, ftp2 = g_peaks,
				sp1 = headF->dp->locs, sp2 = locs, j=ncand; j--; ) {
					*ftp1++ = *ftp2++;
					*sp1++ = *sp2++;
			}
			*sp1 = -1;		/* distinguish the UNVOICED candidate */
			*ftp1 = maxval;
			headF->dp->mpvals[ncand] = vbias+maxval; /* (high cost if cor. is high)*/
		}

		/* Apply a lag-dependent weight to the peaks to encourage the selection
		of the first major peak.  Translate the modified peak values into
		costs (high peak ==> low cost). */
		for(j=0; j < ncand; j++){
			ftemp = 1.0f - ((float)locs[j] * lagwt);
			headF->dp->mpvals[j] = 1.0f - (g_peaks[j] * ftemp);
		}
		ncand++;			/* include the unvoiced candidate */
		headF->dp->ncands = ncand;

		/*********************************************************************/
		/*    COMPUTE THE DISTANCE MEASURES AND ACCUMULATE THE COSTS.       */
		/*********************************************************************/

		ncandp = headF->prev->dp->ncands;
		for(k=0; k<ncand; k++){	/* for each of the current candidates... */
			minloc = 0;
			errmin = FLT_MAX;
			if((loc2 = headF->dp->locs[k]) > 0) { /* current cand. is voiced */
				for(j=0; j<ncandp; j++){ /* for each PREVIOUS candidate... */
					/*    Get cost due to inter-frame period change. */
					loc1 = headF->prev->dp->locs[j];
					if (loc1 > 0) { /* prev. was voiced */
						ftemp = (float) log(((double) loc2) / loc1);
						ttemp = (float) fabs(ftemp);
						ft1 = (float) (fdouble + fabs(ftemp + ln2));
						if (ttemp > ft1)
							ttemp = ft1;
						ft1 = (float) (fdouble + fabs(ftemp - ln2));
						if (ttemp > ft1)
							ttemp = ft1;
						ferr = ttemp * freqwt;
					} else {		/* prev. was unvoiced */
						ferr = tcost + (tfact_s * sta[i]) + (tfact_a / rms_ratio[i]);
					}
					/*    Add in cumulative cost associated with previous peak. */
					err = ferr + headF->prev->dp->dpvals[j];
					if(err < errmin){	/* find min. cost */
						errmin = err;
						minloc = j;
					}
				}
			} else {			/* this is the unvoiced candidate */
				for(j=0; j<ncandp; j++){ /* for each PREVIOUS candidate... */

					/*    Get voicing transition cost. */
					if (headF->prev->dp->locs[j] > 0) { /* previous was voiced */
						ferr = tcost + (tfact_s * sta[i]) + (tfact_a * rms_ratio[i]);
					}
					else
						ferr = 0.0;
					/*    Add in cumulative cost associated with previous peak. */
					err = ferr + headF->prev->dp->dpvals[j];
					if(err < errmin){	/* find min. cost */
						errmin = err;
						minloc = j;
					}
				}
			}
			/* Now have found the best path from this cand. to prev. frame */
			if (first_time && i==0) {		/* this is the first frame */
				headF->dp->dpvals[k] = headF->dp->mpvals[k];
				headF->dp->prept[k] = 0;
			} else {
				headF->dp->dpvals[k] = errmin + headF->dp->mpvals[k];
				headF->dp->prept[k] = minloc;
			}
		} /*    END OF THIS DP FRAME */

		if (i < nframes - 1)
			headF = headF->next;

		if (debug_level >= 2) {
			Fprintf(stderr,"%d engref:%10.0f max:%7.5f loc:%4d\n",
				i,engref,maxval,maxloc);
		}

	} /* end for (i ...) */

	/***************************************************************/
	/* DONE WITH FILLING DP STRUCTURES FOR THE SET OF SAMPLED DATA */
	/*    NOW FIND A CONVERGED DP PATH                             */
	/***************************************************************/

	*vecsize = 0;			/* # of output frames returned */

	num_active_frames += nframes;

	if( num_active_frames >= size_frame_hist  || last_time ){
		Frame *frm;
		int  num_paths, best_cand, frmcnt, checkpath_done = 1;
		float patherrmin;

		if(debug_level)
			Fprintf(stderr, "available frames for backtracking: %d\n",
			num_active_frames);

		patherrmin = FLT_MAX;
		best_cand = 0;
		num_paths = headF->dp->ncands;

		/* Get the best candidate for the final frame and initialize the
		paths' backpointers. */
		frm = headF;
		for(k=0; k < num_paths; k++) {
			if (patherrmin > headF->dp->dpvals[k]){
				patherrmin = headF->dp->dpvals[k];
				best_cand = k;	/* index indicating the best candidate at a path */
			}
			pcands[k] = frm->dp->prept[k];
		}

		if(last_time){     /* Input data was exhausted. force final outputs. */
			cmpthF = headF;		/* Use the current frame as starting point. */
		} else {
			/* Starting from the most recent frame, trace back each candidate's
			best path until reaching a common candidate at some past frame. */
			frmcnt = 0;
			while (1) {
				frm = frm->prev;
				frmcnt++;
				checkpath_done = 1;
				for(k=1; k < num_paths; k++){ /* Check for convergence. */
					if(pcands[0] != pcands[k])
						checkpath_done = 0;
				}
				if( ! checkpath_done) { /* Prepare for checking at prev. frame. */
					for(k=0; k < num_paths; k++){
						pcands[k] = frm->dp->prept[pcands[k]];
					}
				} else {	/* All paths have converged. */
					cmpthF = frm;
					best_cand = pcands[0];
					if(debug_level)
						Fprintf(stderr,
						"paths went back %d frames before converging\n",frmcnt);
					break;
				}
				if(frm == tailF){	/* Used all available data? */
					if( num_active_frames < size_frame_out) { /* Delay some more? */
						checkpath_done = 0; /* Yes, don't backtrack at this time. */
						cmpthF = NULL;
					} else {		/* No more delay! Force best-guess output. */
						checkpath_done = 1;
						cmpthF = headF;
						/*	    Fprintf(stderr,
						"WARNING: no converging path found after going back %d frames, will use the lowest cost path\n",num_active_frames);*/
					}
					break;
				} /* end if (frm ...) */
			}	/* end while (1) */
		} /* end if (last_time) ... else */

		/*************************************************************/
		/* BACKTRACKING FROM cmpthF (best_cand) ALL THE WAY TO tailF    */
		/*************************************************************/
		i = 0;
		frm = cmpthF;	/* Start where convergence was found (or faked). */
		while( frm != tailF->prev && checkpath_done){
			if( i == output_buf_size ){ /* Need more room for outputs? */
				output_buf_size *= 2;
				if(debug_level)
					Fprintf(stderr,
					"reallocating space for output frames: %d\n",
					output_buf_size);
				rms_speech = (float *) realloc((void *) rms_speech,
					sizeof(float) * output_buf_size);
				/*	spsassert(rms_speech, "rms_speech realloc failed in dp_f0()");*/
				f0p = (float *) realloc((void *) f0p,
					sizeof(float) * output_buf_size);
				/*	spsassert(f0p, "f0p realloc failed in dp_f0()");*/
				vuvp = (float *) realloc((void *) vuvp, sizeof(float) * output_buf_size);
				/*	spsassert(vuvp, "vuvp realloc failed in dp_f0()");*/
				acpkp = (float *) realloc((void *) acpkp, sizeof(float) * output_buf_size);
				/*	spsassert(acpkp, "acpkp realloc failed in dp_f0()");*/
			}
			rms_speech[i] = frm->rms;
			acpkp[i] =  frm->dp->pvals[best_cand];
			loc1 = frm->dp->locs[best_cand];
			vuvp[i] = 1.0;
			best_cand = frm->dp->prept[best_cand];
			ftemp = (float) loc1;
			if(loc1 > 0) {		/* Was f0 actually estimated for this frame? */
				if (loc1 > start && loc1 < stop) { /* loc1 must be a local maximum. */
					float cormax, cprev, cnext, den;

					j = loc1 - start;
					cormax = frm->cp->correl[j];
					cprev = frm->cp->correl[j+1];
					cnext = frm->cp->correl[j-1];
					den = (float) (2.0 * ( cprev + cnext - (2.0 * cormax) ));
					/*
					* Only parabolic interpolate if cormax is indeed a local 
					* turning point. Find peak of curve that goes though the 3 points
					*/

					if (fabs(den) > 0.000001)
						ftemp += 2.0f - ((((5.0f*cprev)+(3.0f*cnext)-(8.0f*cormax))/den));
				}
				f0p[i] = (float) (freq/ftemp);
			} else {		/* No valid estimate; just fake some arbitrary F0. */
				f0p[i] = 0;
				vuvp[i] = 0.0;
			}
			frm = frm->prev;

			if (debug_level >= 2)
				Fprintf(stderr," i:%4d%8.1f%8.1f\n",i,f0p[i],vuvp[i]);
			/* f0p[i] starts from the most recent one */ 
			/* Need to reverse the order in the calling function */
			i++;
		} /* end while() */
		if (checkpath_done){
			*vecsize = i;
			tailF = cmpthF->next;
			num_active_frames -= *vecsize;
		}
	} /* end if() */

	if (debug_level)
		Fprintf(stderr, "writing out %d frames.\n", *vecsize);

	*f0p_pt = f0p;
	*vuvp_pt = vuvp;
	*acpkp_pt = acpkp;
	*rms_speech_pt = rms_speech;
	/*  *acpkp_pt = acpkp;*/

	if(first_time) first_time = 0;
	return(0);
}


/*--------------------------------------------------------------------*/
Frame * CF0::alloc_frame(int nlags, int ncands)
{
	Frame *frm;
	int j;

	frm = (Frame*)malloc(sizeof(Frame));
	frm->dp = (Dprec *) malloc(sizeof(Dprec));
	/*  spsassert(frm->dp,"frm->dp malloc failed in alloc_frame");*/
	frm->dp->ncands = 0;
	frm->cp = (Cross *) malloc(sizeof(Cross));
	/*  spsassert(frm->cp,"frm->cp malloc failed in alloc_frame");*/
	frm->cp->correl = (float *) malloc(sizeof(float) * nlags);
	/*  spsassert(frm->cp->correl, "frm->cp->correl malloc failed");*/
	/* Allocate space for candidates and working arrays. */
	frm->dp->locs = (short*)malloc(sizeof(short) * ncands);
	/*  spsassert(frm->dp->locs,"frm->dp->locs malloc failed in alloc_frame()");*/
	frm->dp->pvals = (float*)malloc(sizeof(float) * ncands);
	/*  spsassert(frm->dp->pvals,"frm->dp->pvals malloc failed in alloc_frame()");*/
	frm->dp->mpvals = (float*)malloc(sizeof(float) * ncands);
	/*  spsassert(frm->dp->mpvals,"frm->dp->mpvals malloc failed in alloc_frame()");*/
	frm->dp->prept = (short*)malloc(sizeof(short) * ncands);
	/*  spsassert(frm->dp->prept,"frm->dp->prept malloc failed in alloc_frame()");*/
	frm->dp->dpvals = (float*)malloc(sizeof(float) * ncands);
	/*  spsassert(frm->dp->dpvals,"frm->dp->dpvals malloc failed in alloc_frame()");*/

	/*  Initialize the cumulative DP costs to zero */
	for(j = ncands-1; j >= 0; j--)
		frm->dp->dpvals[j] = 0.0;

	return(frm);
}


/*--------------------------------------------------------------------*/
/* push window stat to stack, and pop the oldest one */

int CF0::save_windstat(float *rho,int  order,float  err, float rms)
{
	int i,j;

	if(wReuse > 1){               /* push down the stack */
		for(j=1; j<wReuse; j++){
			for(i=0;i<=order; i++) windstat[j-1].rho[i] = windstat[j].rho[i];
			windstat[j-1].err = windstat[j].err;
			windstat[j-1].rms = windstat[j].rms;
		}
		for(i=0;i<=order; i++) windstat[wReuse-1].rho[i] = rho[i]; /*save*/
		windstat[wReuse-1].err = (float) err;
		windstat[wReuse-1].rms = (float) rms;
		return 1;
	} else if (wReuse == 1) {
		for(i=0;i<=order; i++) windstat[0].rho[i] = rho[i];  /* save */
		windstat[0].err = (float) err;
		windstat[0].rms = (float) rms;
		return 1;
	} else 
		return 0;
}


/*--------------------------------------------------------------------*/
int CF0::retrieve_windstat(float *rho, int order,float * err, float *rms)
{
	Windstat wstat;
	int i;

	if(wReuse){
		wstat = windstat[0];
		for(i=0; i<=order; i++) rho[i] = wstat.rho[i];
		*err = wstat.err;
		*rms = wstat.rms;
		return 1;
	}
	else return 0;
}


/*--------------------------------------------------------------------*/
float CF0::get_similarity(int order, int size,float * pdata,float * cdata,
	float *rmsa, float *rms_ratio,float pre, float stab, int w_type, int init)
{
	float rho3[BIGSORD+1], err3, rms3, rmsd3, b0, t, a2[BIGSORD+1], 
		rho1[BIGSORD+1], a1[BIGSORD+1], b[BIGSORD+1], err1, rms1, rmsd1;
	//float xitakura (
	//	register int p,register float *b, register float *c,register float *r,register float *gain
	//	) ;

	//float wind_energy(
	//	register float *data,	/* input PCM data */
	//	register int size,		/* size of window */
	//	register int   w_type);			/* window type */
	//void xa_to_aca ( 
	//	float *a, float*b, float*c,
	//	register int p) ;

	//int xlpc(
	//	int lpc_ord,		/* Analysis order */
	//	float lpc_stabl,	/* Stability factor to prevent numerical problems. */
	//	int  wsize,			/* window size in points */
	//	float *data,
	//	float  *lpca,		/* if non-NULL, return vvector for predictors */
	//	float  *ar,		/* if non-NULL, return vector for normalized autoc. */
	//	float  *lpck,		/* if non-NULL, return vector for PARCOR's */
	//	float  *normerr,		/* return scaler for normalized error */
	//	float  *rms,		/* return scaler for energy in preemphasized window */
	//	float  preemp,
	//	int  type		/* window type (decoded in window() above) */
	//	) ;
	/* (In the lpc() calls below, size-1 is used, since the windowing and
	preemphasis function assumes an extra point is available in the
	input data array.  This condition is apparently no longer met after
	Derek's modifications.) */

	/* get current window stat */
	xlpc(order, stab, size-1, cdata,
		a2, rho3, (float *) NULL, &err3, &rmsd3, pre, w_type);
	rms3 = wind_energy(cdata, size, w_type);

	if(!init) {
		/* get previous window stat */
		if( !retrieve_windstat(rho1, order, &err1, &rms1)){
			xlpc(order, stab, size-1, pdata,
				a1, rho1, (float *) NULL, &err1, &rmsd1, pre, w_type);
			rms1 = wind_energy(pdata, size, w_type);
		}
		xa_to_aca(a2+1,b,&b0,order);
		t = xitakura(order,b,&b0,rho1+1,&err1) - .8f;
		if(rms1 > 0.0)
			*rms_ratio = (0.001f + rms3)/rms1;
		else
			if(rms3 > 0.0)
				*rms_ratio = 2.0;	/* indicate some energy increase */
			else
				*rms_ratio = 1.0;	/* no change */
	} else {
		*rms_ratio = 1.0;
		t = 10.0;
	}
	*rmsa = rms3;
	save_windstat( rho3, order, err3, rms3);
	return((float)(0.2/t));
}


/* -------------------------------------------------------------------- */
/* This is an ad hoc signal stationarity function based on Itakura
* distance and relative amplitudes.
*/
/* 
This illustrates the window locations when the very first frame is read.
It shows an example where each frame step |  .  | is 10 msec.  The
frame step size is variable.  The window size is always 30 msec.
The window centers '*' is always 20 msec apart.
The windows cross each other right at the center of the DP frame, or
where the '.' is.

---------*---------   current window

---------*---------  previous window

|  .  |  .  |  .  |  .  |  .  |  .  |  .  |  .  |  .  |
^           ^  ^
^           ^  ^
^           ^  fdata
^           ^
^           q
p

---
ind

fdata, q, p, ind, are variables used below.

*/

//static Stat *stat = NULL;
//static float *mem = NULL;

Stat*
	CF0::get_stationarity(float *fdata, double freq, int buff_size, int nframes, int frame_step,int first_time)
{
	//static int nframes_old = 0, memsize;
	float preemp = 0.4f, stab = 30.0f;
	float *p, *q, *r, *datend;
	int ind, i, j, m, size, order, agap, w_type = 3;

	agap = (int) (STAT_AINT *freq);
	size = (int) (STAT_WSIZE * freq);
	ind = (agap - size) / 2;

	if( nframes_old < nframes || !stat || first_time){
		/* move this to init_dp_f0() later */
		nframes_old = nframes;
		if(stat){
			free((char *) stat->stat);
			free((char *) stat->rms);
			free((char *) stat->rms_ratio);
			free((char *) stat);
		}
		if (mem) free((void *)mem); 
		stat = (Stat *) malloc(sizeof(Stat));
		/*    spsassert(stat,"stat malloc failed in get_stationarity");*/
		stat->stat = (float*)malloc(sizeof(float)*nframes);
		/*    spsassert(stat->stat,"stat->stat malloc failed in get_stationarity");*/
		stat->rms = (float*)malloc(sizeof(float)*nframes);
		/*    spsassert(stat->rms,"stat->rms malloc failed in get_stationarity");*/
		stat->rms_ratio = (float*)malloc(sizeof(float)*nframes);
		/*    spsassert(stat->rms_ratio,"stat->ratio malloc failed in get_stationarity");*/
		memsize = (int) (STAT_WSIZE * freq) + (int) (STAT_AINT * freq);
		mem = (float *) malloc( sizeof(float) * memsize);
		/*    spsassert(mem, "mem malloc failed in get_stationarity()");*/
		for(j=0; j<memsize; j++) mem[j] = 0;
	}

	if(nframes == 0) return(stat);

	q = fdata + ind;
	datend = fdata + buff_size;

	if((order = (int) (2.0 + (freq/1000.0))) > BIGSORD) {
		Fprintf(stderr,
			"Optimim order (%d) exceeds that allowable (%d); reduce Fs\n",order, BIGSORD);
		order = BIGSORD;
	}

	/* prepare for the first frame */
	for(j=memsize/2, i=0; j<memsize; j++, i++) mem[j] = fdata[i];

	/* never run over end of frame, should already taken care of when read */

	for(j=0, p = q - agap; j < nframes; j++, p += frame_step, q += frame_step){
		if( (p >= fdata) && (q >= fdata) && ( q + size <= datend) )
			stat->stat[j] = get_similarity(order,size, p, q, 
			&(stat->rms[j]),
			&(stat->rms_ratio[j]),preemp,
			stab,w_type, 0);
		else {
			if(first_time) {
				if( (p < fdata) && (q >= fdata) && (q+size <=datend) )
					stat->stat[j] = get_similarity(order,size, NULL, q,
					&(stat->rms[j]),
					&(stat->rms_ratio[j]),
					preemp,stab,w_type, 1);
				else{
					stat->rms[j] = 0.0;
					stat->stat[j] = 0.01f * 0.2f;   /* a big transition */
					stat->rms_ratio[j] = 1.0;   /* no amplitude change */
				}
			} else {
				if( (p<fdata) && (q+size <=datend) ){
					stat->stat[j] = get_similarity(order,size, mem, 
						mem + (memsize/2) + ind,
						&(stat->rms[j]),
						&(stat->rms_ratio[j]),
						preemp, stab,w_type, 0);
					/* prepare for the next frame_step if needed */
					if(p + frame_step < fdata ){
						for( m=0; m<(memsize-frame_step); m++) 
							mem[m] = mem[m+frame_step];
						r = q + size;
						for( m=0; m<frame_step; m++) 
							mem[memsize-frame_step+m] = *r++;
					}
				}
			}
		}
	}

	/* last frame, prepare for next call */
	for(j=(memsize/2)-1, p=fdata + (nframes * frame_step)-1; j>=0 && p>=fdata; j--)
		mem[j] = *p--;
	return(stat);
}


/* -------------------------------------------------------------------- */
/*	Round the argument to the nearest integer.			*/
/*
int
eround(flnum)
double  flnum;
{
return((flnum >= 0.0) ? (int)(flnum + 0.5) : (int)(flnum - 0.5));
}

*/
void CF0::free_dp_f0()
{
	int i;
	Frame *frm, *next;

	//	printf("free pcands\n");
	free((void *)pcands);
	pcands = NULL;

	//	printf("free rms_speech\n");
	free((void *)rms_speech);
	rms_speech = NULL;

	//	printf("free f0p\n");
	free((void *)f0p);
	f0p = NULL;

	//	printf("free vuvp\n");
	free((void *)vuvp);
	vuvp = NULL;

	//	printf("free acpkp\n");
	free((void *)acpkp);
	acpkp = NULL;

	//	printf("free peaks, %d\n", g_peaks);
	free((void *)g_peaks);
	g_peaks = NULL;

	//	printf("free locs\n");
	free((void *)locs);
	locs = NULL;

	if (wReuse) {
		free((void *)windstat);
		windstat = NULL;
	}

	frm = headF;

	for(i = 0; i < size_cir_buffer; i++) {
		next = frm->next;
		free((void *)frm->cp->correl);
		free((void *)frm->dp->locs);
		free((void *)frm->dp->pvals);
		free((void *)frm->dp->mpvals);
		free((void *)frm->dp->prept);
		free((void *)frm->dp->dpvals);
		free((void *)frm->cp);
		free((void *)frm->dp);
		free((void *)frm);
		frm = next;
	}
	headF = NULL;
	tailF = NULL;

	free((void *)stat->stat);
	free((void *)stat->rms);
	free((void *)stat->rms_ratio);

	free((void *)stat);
	stat = NULL;

	free((void *)mem);
	mem = NULL;
}
int CF0::sign(float x)
{
	if (x > 0)
		return 1;
	if (x < 0)
		return -1;
	return 0;
}

float CF0::pchipend(int h1, int h2, float del1, float del2)
{
	float d = ((2*h1+h2)*del1 - h1*del2)/(h1+h2);
	if (sign(d) != sign(del1))
		return 0.0;
	if ((sign(del1) != sign(del2)) && (abs(d) > abs(3*del1)))
		return 3*del1;
	return 0.0;
}


int CF0::Smooth_f0(out_params *opar, float* out)
{
	int i;

	std::vector<float> pchip_f0p, smooth_log_f0p;
	float frame_step = 0.01f;

	pchip_f0p.assign(opar->f0p, opar->f0p + opar->nframe);
	smooth_log_f0p.assign(opar->nframe, 0.0);

	if (opar->f0p[0] == 0) {
		opar->f0p[0] = 50;
		pchip_f0p[0] = 50;
	}

	if (opar->f0p[opar->nframe - 1] == 0) {
		opar->f0p[opar->nframe - 1] = 50;
		pchip_f0p[opar->nframe - 1] = 50;
	}

	//memcpy(opar->pchip_f0p, opar->f0p, sizeof(float) * opar->nframe);

	std::vector<int> x;
	std::vector<float> y;
	for (i = 0; i < opar->nframe; i++) {
		if (opar->f0p[i] > 0) {
			x.push_back(i);
			y.push_back(opar->f0p[i]);
		}
	}
	if (x.size() < 3)
		return GETF0_ERROR;

	//  First derivatives
	std::vector<int> h;
	std::vector<float> delta;
	for (i = 0; i < x.size() - 1; i++) {
		h.push_back(x[i+1] - x[i]);
		delta.push_back((y[i+1] - y[i]) / (x[i+1] - x[i]));
	}

	int length = h.size() + 1;
	std::vector<float> d;
	d.assign(length, 0.0);
	for (i = 1; i < delta.size(); i++) {
		if (delta[i] * delta[i-1] > 0) {
			int w1 = 2 * h[i] + h[i-1];
			int w2 = h[i] + 2 * h[i-1];
			d[i] = (w1 + w2) / (w1 / delta[i-1] + w2 / delta[i]);
		}
	}
	//  Slopes at endpoints
	d[0] = pchipend(h[0], h[1], delta[0], delta[1]);
	d[length - 1] = pchipend(h[length - 2], h[length - 3], delta[length - 2], delta[length - 3]);

	//  Piecewise polynomial coefficients
	std::vector<float> c(length - 1);
	std::vector<float> b(length - 1);
	for (i = 0; i < length - 1; i++) {
		c[i] = (3*delta[i] - 2*d[i] - d[i+1]) / h[i];
		b[i] = (d[i] - 2*delta[i] + d[i+1]) / (h[i] * h[i]);
	}

	// Get interpolated values
	for (i = 0; i < x.size() - 1; i++) {
		for (int j = x[i] + 1; j < x[i+1]; j++) {
			int s = j - x[i];
			float v = y[i] + s * (d[i] + s * (c[i] + s * b[i]));
			pchip_f0p[j] = v;
		}
	}

	// log(f0)
	std::vector<float> logf0(opar->nframe);
	for (i = 0; i < opar->nframe; i++) {
		logf0[i] = pchip_f0p[i] < FLT_MIN ? log(FLT_MIN) : log(pchip_f0p[i]);
		smooth_log_f0p[i] = logf0[i];
	}

	// Moving window normalization, 1 seconds
	int half_win_len = int(1.0 / frame_step) / 2;
	int win_len = half_win_len * 2 + 1;
	if (opar->nframe <= win_len) {
		float average = 0.0;
		for (int i = 0; i < opar->nframe; i++)
			average += logf0[i];
		average /= opar->nframe;
		for (i = 0; i < opar->nframe; i++)
			logf0[i] -= average;
	} else {
		std::vector<float> logf0_ma(opar->nframe); // moving windows average
		float cursum = 0.0;
		for (int i = 0; i < win_len; i++)
			cursum += logf0[i];
		logf0_ma[half_win_len] = cursum / win_len;
		for (i = half_win_len + 1; i < opar->nframe - half_win_len; i++) {
			cursum = cursum - logf0[i - 1 - half_win_len] + logf0[i + half_win_len];
			logf0_ma[i] = cursum / win_len;
		}
		for (i = 0; i < half_win_len; i++)
			logf0_ma[i] = logf0_ma[half_win_len];
		for (i = opar->nframe - half_win_len; i < opar->nframe; i++)
			logf0_ma[i] = logf0_ma[opar->nframe - half_win_len - 1];
		for (i = 0; i < opar->nframe; i++)
			logf0[i] -= logf0_ma[i];
	}
	for (i = 0; i < opar->nframe; i++)
		smooth_log_f0p[i] = logf0[i];

	float cursum = 0.0;
	for (i = 0; i < 5; i++)
		cursum += logf0[i];

	smooth_log_f0p[0] = smooth_log_f0p[1] = smooth_log_f0p[2] = cursum / 5;

	for (i = 3; i < opar->nframe - 2; i++) {
		cursum = cursum - logf0[i - 3] + logf0[i + 2];
		smooth_log_f0p[i] = cursum / 5.0;
	}

	smooth_log_f0p[opar->nframe - 1] = smooth_log_f0p[opar->nframe - 2] = smooth_log_f0p[opar->nframe - 3];

	// output result
	for(i = 0; i< opar->nframe; ++i) {
		out[i] = smooth_log_f0p[i];
	}

	return 0;
}

/// [in] ptr: pointer to wave data, 8K 16bit 
/// [in] num_samples: length of wave
/// [out] out: pointer to pitch contour
/// [in]  buf_size: size of out buffer
int CF0::RunPitch(short* ptr, int num_samples, float* out, int buf_size, bool is_smooth, int null_pitch_delay) {

	int  i,binary_mode=0, verbose_mode=0, tail_zerofill=0;
	char *outputfile = NULL;
	F0_params *par; 
	wav_params *wpar;
	out_params *opar; 
	wpar = (wav_params *) malloc(sizeof(wav_params));
	opar = (out_params *) malloc(sizeof(out_params));
	par = (F0_params *) malloc(sizeof(F0_params));

	wpar->file = NULL;
	wpar->rate = 16000; // 8000;
	wpar->size = 2;
	wpar->length = 0;
	wpar->data = NULL;
	wpar->swap = 0;
	wpar->head_pad = 0;
	wpar->tail_pad = 0;
	wpar->startpos = 0;
	wpar->nan = -1;
	wpar->padding = 0;

	opar->nframe = 0;
	opar->f0p = NULL;
	opar->vuvp = NULL;
	opar->rms_speech = NULL;
	opar->acpkp = NULL;

	par->cand_thresh = 0.3f;
	par->lag_weight = 0.3f;
	par->freq_weight = 0.02f;
	par->trans_cost = 0.005f;
	par->trans_amp = 0.5f;
	par->trans_spec = 0.5f;
	par->voice_bias = 0.0f;
	par->double_cost = 0.35f;
	par->min_f0 = 50;
	par->max_f0 = 550;
	par->frame_step = 0.01f;
	par->wind_dur = 0.01f;
	par->n_cands = 20;
	par->mean_f0 = 200;     /* unused */
	par->mean_f0_weight = 0.0f;  /* unused */
	par->conditioning = 0;    /*unused */

	/* last item of argv[] must be filename or NULL */

	if( wpar->padding){
		wpar->head_pad = eround( par->wind_dur * wpar->rate / 2.0);
		wpar->tail_pad = 0;

		if( tail_zerofill > 0){
			wpar->tail_pad = eround(par->frame_step * tail_zerofill * wpar->rate);
		}
	}

	//  /* load wavedata */
	wpar->length = num_samples;

	int pos = 0;
	if( wpar->padding) {
		wpar->length += wpar->head_pad;
		wpar->length += wpar->tail_pad;
		pos = wpar->head_pad;
	}

	wpar->data = (float *) malloc( sizeof(float)*wpar->length);
	if ( NULL == wpar->data ) {
		fprintf(stderr, "Allocate memory for wpar->data failed\n");
		free( wpar );
		free( par );
		free_out_params( opar );
		return 0;
	}
	memset( wpar->data, 0, sizeof(float)*wpar->length);

	for (i = 0; i < num_samples; ++i) {
		wpar->data[pos++] = (float)ptr[i];
	}

	if( wpar->nan == -1) wpar->nan = wpar->length;

	//	printf("init_out\n");
	/* init output data */
	if( init_out_params( opar, 
		(int) ((wpar->length / (wpar->rate * par->frame_step))+0.5)) == GETF0_ERROR){
			fprintf( stderr, "error: init_out_params()\n");
			free( wpar->data );
			free( wpar );
			free( par );
			free_out_params( opar );

			return 0;
	}

	//	printf("Get_f0\n");
	/* estimate F0 */
	if( Get_f0( wpar, par, opar) == GETF0_ERROR){
		fprintf( stderr, "error: get_f0()\n");
		free( wpar->data );
		free( wpar );
		free( par );
		free_out_params( opar );
		return 0;
	}

	if ( opar->nframe > buf_size ) {
		fprintf( stderr, "warning: number of frames = %d, buffer size = %d\n", opar->nframe, buf_size);
		opar->nframe = buf_size;
	}

	/* output result */
	int nframes = opar->nframe;

	if ( true == is_smooth ) {
		//Smooth_f0(opar, out);
		if (Smooth_f0(opar, out) == GETF0_ERROR) {
	//		nframes = 0;	
			if (nframes > null_pitch_delay) {
				nframes -= null_pitch_delay;
			} else {
				nframes = 0;
			}
                	for (i = 0; i < nframes; ++i) {
                      		out[i] = 0.f;
                	}
		}
	}
	else {
		for (i = 0; i < nframes; ++i) {
			// out[i] = opar->f0p[i] < 1e-6 ? 0 : log(opar->f0p[i]);
			out[i] = opar->f0p[i];
		}
	}
	//	printf("clear\n");
	free( wpar->data );
	free( wpar );
	free( par );
	free_out_params( opar );
	return nframes;
}

int CF0::Smooth_f0(float* out, int nframe)
{
	int i;
	std::vector<float> pchip_f0p, smooth_log_f0p;
	float frame_step = 0.01f;

	pchip_f0p.assign(out, out + nframe);
	smooth_log_f0p.assign(nframe, 0.0);

	if (pchip_f0p[0] == 0) {
		pchip_f0p[0] = 50;
	}

	if (pchip_f0p[nframe - 1] == 0) {
		pchip_f0p[nframe - 1] = 50;
	}

	std::vector<int> x;
	std::vector<float> y;
	for (i = 0; i < nframe; i++) {
		if (pchip_f0p[i] > 0) {
			x.push_back(i);
			y.push_back(pchip_f0p[i]);
		}
	}
	if (x.size() < 3)
		return GETF0_ERROR;

	//  First derivatives
	std::vector<int> h;
	std::vector<float> delta;
	for (i = 0; i < x.size() - 1; i++) {
		h.push_back(x[i+1] - x[i]);
		delta.push_back((y[i+1] - y[i]) / (x[i+1] - x[i]));
	}

	int length = h.size() + 1;
	std::vector<float> d;
	d.assign(length, 0.0);
	for (i = 1; i < delta.size(); i++) {
		if (delta[i] * delta[i-1] > 0) {
			int w1 = 2 * h[i] + h[i-1];
			int w2 = h[i] + 2 * h[i-1];
			d[i] = (w1 + w2) / (w1 / delta[i-1] + w2 / delta[i]);
		}
	}
	//  Slopes at endpoints
	d[0] = pchipend(h[0], h[1], delta[0], delta[1]);
	d[length - 1] = pchipend(h[length - 2], h[length - 3], delta[length - 2], delta[length - 3]);

	//  Piecewise polynomial coefficients
	std::vector<float> c(length - 1);
	std::vector<float> b(length - 1);
	for (i = 0; i < length - 1; i++) {
		c[i] = (3*delta[i] - 2*d[i] - d[i+1]) / h[i];
		b[i] = (d[i] - 2*delta[i] + d[i+1]) / (h[i] * h[i]);
	}

	// Get interpolated values
	for (i = 0; i < x.size() - 1; i++) {
		for (int j = x[i] + 1; j < x[i+1]; j++) {
			int s = j - x[i];
			float v = y[i] + s * (d[i] + s * (c[i] + s * b[i]));
			pchip_f0p[j] = v;
		}
	}

	// log(f0)
	std::vector<float> logf0(nframe);
	for (i = 0; i < nframe; i++) {
		logf0[i] = pchip_f0p[i] < FLT_MIN ? log(FLT_MIN) : log(pchip_f0p[i]);
		smooth_log_f0p[i] = logf0[i];
	}

	// Moving window normalization, 1 seconds
	int half_win_len = int(1.0 / frame_step) / 2;
	int win_len = half_win_len * 2 + 1;
	if (nframe <= win_len) {
		float average = 0.0;
		for (i = 0; i < nframe; i++)
			average += logf0[i];
		average /= nframe;
		for (i = 0; i < nframe; i++)
			logf0[i] -= average;
	} else {
		std::vector<float> logf0_ma(nframe); // moving windows average
		float cursum = 0.0;
		for (i = 0; i < win_len; i++)
			cursum += logf0[i];
		logf0_ma[half_win_len] = cursum / win_len;
		for (i = half_win_len + 1; i < nframe - half_win_len; i++) {
			cursum = cursum - logf0[i - 1 - half_win_len] + logf0[i + half_win_len];
			logf0_ma[i] = cursum / win_len;
		}
		for (i = 0; i < half_win_len; i++)
			logf0_ma[i] = logf0_ma[half_win_len];
		for (i = nframe - half_win_len; i < nframe; i++)
			logf0_ma[i] = logf0_ma[nframe - half_win_len - 1];
		for (i = 0; i < nframe; i++)
			logf0[i] -= logf0_ma[i];
	}
	for (i = 0; i < nframe; i++)
		smooth_log_f0p[i] = logf0[i];

	float cursum = 0.0;
	for (i = 0; i < 5; i++)
		cursum += logf0[i];

	smooth_log_f0p[0] = smooth_log_f0p[1] = smooth_log_f0p[2] = cursum / 5;

	for (i = 3; i < nframe - 2; i++) {
		cursum = cursum - logf0[i - 3] + logf0[i + 2];
		smooth_log_f0p[i] = cursum / 5.0;
	}

	smooth_log_f0p[nframe - 1] = smooth_log_f0p[nframe - 2] = smooth_log_f0p[nframe - 3];

	// output result
	for(i = 0; i< nframe; ++i) {
		out[i] = smooth_log_f0p[i];
	}

	return 0;
}


