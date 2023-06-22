
#ifdef __INTEL_COMPILER
#include <mathimf.h>
#else
#include <math.h>
#endif
#include <algorithm>
//#include <time.h>
#include <stdlib.h>

#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

#ifdef	_USE_SSE_
#include <emmintrin.h>
#endif

//#include "utils.h"
#include <blis/cblas.h>
#include "dnn.h"

//#include "mdl64noF0.c"
#include "mdl32fbank.c"

//extern const float mean[429];
//extern const float sdev[429];

// optimization methods
#define USE_MKL
//#define USE_SSE2

#ifdef USE_MKL
#ifdef WIN32
#pragma comment(lib,"mkl_solver_sequential.lib")
#pragma comment(lib,"mkl_intel_c.lib")
#pragma comment(lib,"mkl_sequential.lib")
#pragma comment(lib,"mkl_core.lib")
#else
#if defined(__arm__) || defined(__AARCH64EL__)
#include "blis/blis.h"
#else
#ifdef __APPLE__
#include <Accelerate/Accelerate.h>
#endif
#endif
#endif
#else
//#include "cblas/cblas.h"
extern "C" {
#include "cblas/lsame.c"
#include "cblas/saxpy.c"
#include "cblas/sgemm.c"
#include "cblas/strans.c"
#include "cblas/xerbla.c"
}
#endif

#define USE_QUICK_EXP
//#define FIXED_POINT

// Here we have macros for really hacky but quick exponential function.
// This was taken from Chris Oei's version and tidied up a little.
// I don't know anything about the origin, but it appears to be relying
// on the fact that storing an int in the exponent of a double gives
// you an exponential in the higher bits with the lower bits being
// used to add a straight line fit.
// Note that, due to the use of the QN_WORDS_BIGENDIAN macro, only works on
// machines where the words-in-a-double ordering  is the same as the
// bytes-in-a-word ordering.  There are exceptions, e.g. the Vax!
// It also relies on the static qn_d2i being initialized to zero.
// ARLO: This is from the paper "A Fast, Compact Approximation of the
//       Exponential Function" by Schraudolph (1999).  It is only valid
//       for the range -700 to 700, so I added this check.  It can be
//       about 4x faster for integer inputs, but the conversion of float
//       to int just about cancels out the speedup.

//For integers, set QN_EXP_A = 1512775

//// FAST EXP APPROXIMATION
#ifdef USE_QUICK_EXP
#define QN_EXPQ_WORKSPACE union { double d; struct { int j,i;} n; } qn_d2i; qn_d2i.d = 0.0;

#ifndef M_LN2
#define M_LN2      0.693147180559945309417
#endif

#define EXP_A (1048576/M_LN2)
#define EXP_C 60801
#define FAST_EXP(y) (qn_d2i.n.i = EXP_A*(y)+(1072693248-EXP_C),qn_d2i.d)
#define QN_EXPQ(y) (qn_d2i.n.i = (int) (EXP_A*(y)) + (1072693248 - EXP_C), (y > -700.0f && y < 700.0f) ? qn_d2i.d : exp(y) )
////
#endif

// make orgdim multiply of 4 for 16-byte zero-padding
static int getstride(int orgdim)
{
	int stride = (orgdim+3)/4*4;
	return stride;
}
/*
	allocate 16-byte aligned memory, 
	i.e., the address of first byte if multiply of 16, 
*/
static char* new_aligned( int size, int dim)
{

#ifdef _USE_SSE_
	char *ptr = (char *)_mm_malloc(size*dim, 16);
#else
	char * ptr = new char[size*dim];
#endif
	if (!ptr) return NULL;
	memset(ptr, 0, size*dim);
	return ptr;
}

static void del_aligned( char*& ptr )
{
	if( ptr ) {
#ifdef _USE_SSE_
		_mm_free( ptr );
#else
		delete [] ptr;
#endif
		ptr = NULL;
	}
}

#ifdef USE_SSE2 // use float-point SIMD intrinsics
#include <emmintrin.h>

// inner product of two vectors and plus additive
static float 
multipy_add_sse2(const float* src1, int dim1, const float* src2, int dim2, float additive)
{
	assert(dim1 == dim2 && dim1!=0);
	assert(src1!=NULL && src2!=NULL);
	// ensure memory is 16-byte aligned and zero-padding
	assert((int)src1%16==0 && (int)src2%16==0 && dim1%4==0);

	__m128 tmp, sum = _mm_setzero_ps();
	__m128* pA = (__m128*)src1;
	__m128* pB = (__m128*)src2;
	float* ptr = (float*) &sum;
	for(int k=0; k<dim1; k+=4){
		tmp = _mm_mul_ps(*pA, *pB);
		sum = _mm_add_ps(sum, tmp);
		pA++;
		pB++;
	}
	float ret = (ptr[0]+ptr[1]+ptr[2]+ptr[3]);

	return ret+additive;
}

static void 
vector_add_sse2(float* tgt, int dim1, const float* src, int dim2)
{
	assert(tgt!=NULL && src!=NULL);
	assert(dim1 == dim2 && dim1!=0);
	// ensure memory is 16-byte aligned and zero-padding
	assert((int)tgt%16==0 && (int)src%16==0 && dim1%4==0);

	__m128 tmpval;
	float* tmparray = new float [dim1];
	if (tmparray==NULL)
	{
		printf("Error, fail to allocate memory for vector adding");
		return;
	}
	__m128* pA = (__m128*)tgt;
	__m128* pB = (__m128*)src;
	float* ptr = (float*) &tmpval;
	for(int k=0; k<dim1; k+=4){
		tmpval = _mm_setzero_ps();
		tmpval = _mm_add_ps(*pA, *pB);
		tmparray[k] = ptr[0]; 
		tmparray[k+1] = ptr[1]; 
		tmparray[k+2] = ptr[2]; 
		tmparray[k+3] = ptr[3];
		pA++;
		pB++;
	}
	
	// copy value back to target
	memcpy(tgt, tmparray, sizeof(float) * dim1 );
	if (tmparray!=NULL)
		delete [] tmparray;
}

#endif	// end of USE_SSE2

#ifdef FIXED_POINT
#include <tmmintrin.h>  // SSSE3
#include <smmintrin.h>	// SSE4.1

/* utilities for fixed-point and floating-point conversion */

// quantization from float to unsigned char (8-bit) for activation
inline unsigned char 
quan_ftoi_acvt(float val)
{
	// ensure activation val is >=0 and <=1
	assert(val>=0.0f && val<=1.0f);
	if (val==1.0f)
		return (unsigned char)255;
	else
		return (unsigned char)(val*256);
}

void
quan_ftoi_acvt_vector(const float* src, unsigned char* tgt, int dim)
{
	// float scale = 1.0f / maxval;
	assert(src!=NULL && tgt!=NULL);
	for(int i=0; i<dim; i++)
		*tgt = quan_ftoi_acvt( *src );
}

// necessary convert unsigned char activation back to float before take sigmoid operation
//void
//quan_itof_vector(const signed int* src, float* tgt, int dim, float maxval)
//{
//	//float scale = 1.0f / maxval;
//	assert(src!=NULL && tgt!=NULL);
//	//for(int i=0; i<dim; i++)
//	//	*tgt = quan_ftoi_acvt( (*src)*scale );
//}

// quantization from float to signed char (8-bit) for weight
inline signed char
quan_ftoi_weight(float val)
{

	assert(val>=-1.0f && val<=1.0f);
	if(val==-1.0f)
		return (signed char)(-128);
	else if(val==1.0f)
		return (signed char)127;
	else
		return (signed char)(val*128);
}

void
quan_ftoi_acvt_vector(const float* src, signed char* tgt, int dim, float maxval)
{
	float scale = 1.0f / maxval;
	assert(src!=NULL && tgt!=NULL);
	for(int i=0; i<dim; i++)
		*tgt = quan_ftoi_weight( (*src)*scale );
}

// quantization from float to signed int (32-bit) for bias
inline signed int
quan_ftoi_bias(float val)
{
	assert(val>=-1.0f && val<=1.0f);
	if(val==-1.0f)
		return (signed int)(-2147483648);
	else if(val==1.0f)
		return (signed int)2147483647;
	else
		return (signed int)(val*2147483648);
}

inline float
quan_itof_bias(signed int val)
{
	return (float)val*4.6566128730773926E-10;
}

void
quan_ftoi_bias_vector(const float* src, signed int* tgt, int dim, float maxval)
{
	float scale = 1.0f / maxval;
	assert(src!=NULL && tgt!=NULL);
	for(int i=0; i<dim; i++)
		*tgt = quan_ftoi_bias( (*src)*scale );
}

void
quan_itof_vector(const signed int* src, float* tgt, int dim)
{
	assert(src!=NULL && tgt!=NULL);
	for(int i=0; i<dim; i++)
		*tgt = quan_itof_bias( *src );
}

#endif // end of FIXED_POINT

static void logvec(int dim, float* vec)
{
	for(int i=0; i<dim; i++)
	{
		vec[i] = log(vec[i]);
	}
}

static void sigmoid(int dim, float* vec)
{
	for(int i=0; i<dim; i++)
	{
#ifdef USE_QUICK_EXP
		QN_EXPQ_WORKSPACE
		vec[i] = 1.0f / (1+QN_EXPQ(-vec[i]));
#else
		vec[i] = 1.0f / (1+exp(-vec[i]));
#endif
	}
}

static void softmax(int dim, float *vec)
{
	float sum = 0.0f;
	//float maxout;
	//maxout=findmax(n,out);

	//for(int i=0;i<n;i++)
	//{
	//	sum=sum+exp(out[i]-maxout);
	//}
	//for(int i=0;i<n;i++)
	//{
	//	out[i]=exp(out[i]-maxout)/sum;
	//}
	//std::vector<float>  temp(vec, vec+dim);
	//std::sort(temp.begin(), temp.end());
	//float maxval = temp[dim-1];
	float maxval = *(std::max_element(vec, vec+dim));
	for (int i=0; i<dim; i++)
	{
#ifdef USE_QUICK_EXP
		QN_EXPQ_WORKSPACE
		vec[i] = QN_EXPQ(vec[i]-maxval);
#else
		vec[i] = exp(vec[i]-maxval);
#endif
		sum += vec[i];
	}
	//printf("\n sum value: %f\n", sum);
	float scale = 1.0f / sum;
	for (int i=0; i<dim; i++)
		vec[i] *= scale;
}

Layer2::~Layer2()
{
	if(weights && !m_bLoadFromCFile)
	{
		del_aligned((char*&)weights);
	}
	if(bias && !m_bLoadFromCFile)
	{
		del_aligned((char*&)bias);
	}
	if(activations)
	{
		del_aligned((char*&)activations);
	}

	curr_layer_num = 0;
	prev_layer_num = 0;
	curr_layer_num_stride = 0;
	prev_layer_num_stride = 0;
}

CDNN2::CDNN2(): total_layer_num(0), target_layer_num(0), model(NULL), status(false)
{
	//mkl_set_num_threads(12);

	prior = NULL;
	m_bLoadFromCFile = false;
}

CDNN2::~CDNN2()
{
	//if ( target_layer_num==0 )
	//	return;
	// init to index of last layer
	//int lidx = target_layer_num - 1;
	//while (lidx>=0)
	//{
	//	// first release each layer
	//	if (model[lidx])
	//		delete *(model+lidx);
	//	lidx -= 1;
	//}
	// reset val. and release whole model
	if (model)
	{
		delete [] model;
		model = NULL;
	}
	//// release temp_activations
	//if(temp_activations!=NULL)
	//{
	//	for(int i=0; i<target_layer_num-1; i++)
	//	{
	//		if(temp_activations[i]!=NULL)
	//			del_aligned((char*&)temp_activations[i]);
	//	}
	//	delete [] temp_activations;
	//	temp_activations = NULL;
	//}

	if (prior && !m_bLoadFromCFile) {
		delete[] prior;
		prior = NULL;
	}

	total_layer_num = target_layer_num = 0;
	status = false;
}

/*
	Load DNN model (orginal version)
*/
bool CDNN2::load_model_org(const char* modelfile, int tgtid)
{
	FILE* fp = fopen(modelfile, "rb");
	if (fp==NULL)
	{
		printf("Cannot open network file: %s", modelfile);
		return false;
	}
	
	target_layer_num = tgtid;
	fread(&total_layer_num, sizeof(int), 1, fp);	// get layer number
	if (total_layer_num < 2)
	{
		printf("Error, target layer is at least 2, \
			   but [%d] is provided!", \
			   target_layer_num);
		return false;
	}
	if (total_layer_num < target_layer_num)
	{
		printf("Error, specified target layer [%d] \
			   is larger than total layers [%d]!", \
			   target_layer_num, total_layer_num);
		return false;
	}

	// only load those needed layers
	int *num_per_layer = new int[total_layer_num];
	if (num_per_layer==NULL)
	{
		printf("Error, failed to allocate memory during model loading");
		return false;
	}
	model = new Layer2[target_layer_num-1];
	if (model==NULL)
	{
		printf("Error, failed to allocate memory for model during model loading");
		return false;
	}
	/*
		suppose network layers are 1, 2, 3, ...
		model[0] stores weight12 and bias2,
		model[1] stores weight23 and bias3,
		model[2] stores weight34 and bias4,
		...
		model[i] stores weight (i+1)*(i+2) and bias i+2
	*/ 
	int i;
	for (i=0; i<total_layer_num; i++)
		fread( (num_per_layer+i), sizeof(int), 1, fp);
	
	//
	for (i=0; i<target_layer_num-1; i++)
	{
		model[i].prev_layer_num = num_per_layer[i];
		model[i].curr_layer_num = num_per_layer[i+1];
		
		model[i].prev_layer_num_stride = getstride(model[i].prev_layer_num);
		model[i].curr_layer_num_stride = getstride(model[i].curr_layer_num);
		
		// allocate aligned memory and init to zero
		model[i].bias = (float*)new_aligned(sizeof(float), model[i].curr_layer_num_stride);
		model[i].weights = (float*)new_aligned(sizeof(float), \
			model[i].curr_layer_num * model[i].prev_layer_num_stride);
		model[i].activations = (float*)new_aligned(sizeof(float), model[i].curr_layer_num_stride);
		if (model[i].weights==NULL || model[i].bias==NULL || model[i].activations==NULL)
		{
			printf("Error, failed to allocate memory during model loading.");
			return false;
		}

		// read weight matrix
		for (int j=0; j<model[i].curr_layer_num; j++)
		{
			fread(model[i].weights+j*model[i].prev_layer_num_stride, \
				sizeof(float), model[i].prev_layer_num, fp);
		}

		// read layer bias
		fread(model[i].bias, sizeof(float), model[i].curr_layer_num, fp);
		//if (i==target_layer_num-1) // read last needed layer
		//{
		//	model[i].curr_layer_num = num_per_layer[i];
		//	model[i].prev_layer_num = 0;
		//	model[i].weights = NULL;
		//	model[i].bias = new float [num_per_layer[i]];
		//	model[i].activations = new float [num_per_layer[i]];
		//	if (model[i].bias==NULL || model[i].activations==NULL)
		//	{
		//		printf("Error, failed to allocate memory during model loading.");
		//		return false;
		//	}
		//	// skip weight part to get bias
		//	fseek(fp, sizeof(float)*num_per_layer[i]*num_per_layer[i+1], SEEK_CUR);
		//	fread(model[i].bias, sizeof(float), num_per_layer[i], fp);
		//}
		//else
		//{
		//	model[i].curr_layer_num = num_per_layer[i];
		//	model[i].prev_layer_num = num_per_layer[i+1];
		//	model[i].weights = new float [ num_per_layer[i]*num_per_layer[i+1] ];
		//	model[i].bias = new float [num_per_layer[i]];
		//	model[i].activations = new float [num_per_layer[i]];
		//	if (model[i].weights==NULL || model[i].bias==NULL || model[i].activations==NULL)
		//	{
		//		printf("Error, failed to allocate memory during model loading.");
		//		return false;
		//	}
		//	// read both weight and bias
		//	fread(model[i].weights, sizeof(float), num_per_layer[i]*num_per_layer[i+1], fp);
		//	fread(model[i].bias, sizeof(float), num_per_layer[i], fp);
		//}
	}
	
	printf("Loading model done!\n");

	if (num_per_layer!=NULL)
	{
		delete [] num_per_layer;
		num_per_layer = NULL;
	}
	fclose(fp);
	status = true;
	return true;
}

/*
	Load DNN model (batch version)
*/
bool CDNN2::load_model_batch(const char* modelfile, int tgtid)
{
	FILE* fp = fopen(modelfile, "rb");
	if (fp==NULL)
	{
		printf("Cannot open network file: %s", modelfile);
		return false;
	}
	
	target_layer_num = tgtid;
	fread(&total_layer_num, sizeof(int), 1, fp);	// get layer number
	if (total_layer_num < 2)
	{
		printf("Error, target layer is at least 2, \
			   but [%d] is provided!", \
			   target_layer_num);
		return false;
	}
	if (total_layer_num < target_layer_num)
	{
		printf("Error, specified target layer [%d] \
			   is larger than total layers [%d]!", \
			   target_layer_num, total_layer_num);
		return false;
	}

	// only load those needed layers
	int *num_per_layer = new int[total_layer_num];
	if (num_per_layer==NULL)
	{
		printf("Error, failed to allocate memory during model loading");
		return false;
	}
	model = new Layer2[target_layer_num-1];
	if (model==NULL)
	{
		printf("Error, failed to allocate memory for model during model loading");
		return false;
	}
	/*
		suppose network layers are 1, 2, 3, ...
		model[0] stores weight12 and bias2,
		model[1] stores weight23 and bias3,
		model[2] stores weight34 and bias4,
		...
		model[i] stores weight (i+1)*(i+2) and bias i+2
	*/ 
	int i;
	for (i=0; i<total_layer_num; i++)
		fread( (num_per_layer+i), sizeof(int), 1, fp);
	
	//
	for (i=0; i<target_layer_num-1; i++)
	{
		model[i].prev_layer_num = num_per_layer[i];
		model[i].curr_layer_num = num_per_layer[i+1];
		
		model[i].prev_layer_num_stride = getstride(model[i].prev_layer_num);
		model[i].curr_layer_num_stride = getstride(model[i].curr_layer_num);
		
		// allocate aligned memory and init to zero
		model[i].bias = (float*)new_aligned(sizeof(float), model[i].curr_layer_num_stride);
		model[i].weights = (float*)new_aligned(sizeof(float), \
			model[i].curr_layer_num_stride * model[i].prev_layer_num_stride);
		//model[i].activations = (float*)new_aligned(sizeof(float), model[i].curr_layer_num_stride);
		if (model[i].weights==NULL || model[i].bias==NULL/* || model[i].activations==NULL*/)
		{
			printf("Error, failed to allocate memory during model loading.");
			return false;
		}

		// read weight matrix
		for (int j=0; j<model[i].curr_layer_num; j++)
		{
			fread(model[i].weights+j*model[i].prev_layer_num_stride, \
				sizeof(float), model[i].prev_layer_num, fp);
		}

		// read layer bias
		fread(model[i].bias, sizeof(float), model[i].curr_layer_num, fp);
		//if (i==target_layer_num-1) // read last needed layer
		//{
		//	model[i].curr_layer_num = num_per_layer[i];
		//	model[i].prev_layer_num = 0;
		//	model[i].weights = NULL;
		//	model[i].bias = new float [num_per_layer[i]];
		//	model[i].activations = new float [num_per_layer[i]];
		//	if (model[i].bias==NULL || model[i].activations==NULL)
		//	{
		//		printf("Error, failed to allocate memory during model loading.");
		//		return false;
		//	}
		//	// skip weight part to get bias
		//	fseek(fp, sizeof(float)*num_per_layer[i]*num_per_layer[i+1], SEEK_CUR);
		//	fread(model[i].bias, sizeof(float), num_per_layer[i], fp);
		//}
		//else
		//{
		//	model[i].curr_layer_num = num_per_layer[i];
		//	model[i].prev_layer_num = num_per_layer[i+1];
		//	model[i].weights = new float [ num_per_layer[i]*num_per_layer[i+1] ];
		//	model[i].bias = new float [num_per_layer[i]];
		//	model[i].activations = new float [num_per_layer[i]];
		//	if (model[i].weights==NULL || model[i].bias==NULL || model[i].activations==NULL)
		//	{
		//		printf("Error, failed to allocate memory during model loading.");
		//		return false;
		//	}
		//	// read both weight and bias
		//	fread(model[i].weights, sizeof(float), num_per_layer[i]*num_per_layer[i+1], fp);
		//	fread(model[i].bias, sizeof(float), num_per_layer[i], fp);
		//}
	}
	
	printf("Loading model done!\n");

	if (num_per_layer!=NULL)
	{
		delete [] num_per_layer;
		num_per_layer = NULL;
	}
	//fclose(fp);

	// tmp method for prior
	//ifstream fin("models/triphone-pro.txt");
	//prior = new float[9712];
	//int idx = 0;
	//float tmpp;
	//while (fin >> tmpp) {
	//	prior[idx] = log(tmpp);
	//	idx++;
	//}
	//fin.close();

	int n = model[total_layer_num - 2].curr_layer_num;
	prior = new float[n];
	fread(prior, sizeof(float), n, fp);
	fclose(fp);

	//maxbatchsize = 256;
	//temp_activations = new float* [target_layer_num-1];
	//if(temp_activations==NULL)
	//{
	//	printf("Cannot allocate memory for temporary activations");
	//	return false;
	//}
	//for(int i=0; i<target_layer_num-1; i++)
	//{
	//	temp_activations[i] = (float*)new_aligned(sizeof(float), maxbatchsize*model[i].curr_layer_num_stride);
	//	if(temp_activations[i]==NULL)
	//	{
	//		printf("Cannot allocate memory for temporary activations");
	//		return false;
	//	}
	//}

	status = true;
	return true;
}

bool CDNN2::load_model_cfile(int tgtid)
{
	target_layer_num = tgtid;
	m_bLoadFromCFile = true;
	total_layer_num = mdlfile_total_layer_num;
	if (total_layer_num < 2)
	{
		printf("Error, target layer is at least 2, \
			   but [%d] is provided!", \
			   target_layer_num);
		return false;
	}
	if (total_layer_num < target_layer_num)
	{
		printf("Error, specified target layer [%d] \
			   is larger than total layers [%d]!", \
			   target_layer_num, total_layer_num);
		return false;
	}

	int *num_per_layer = mdlfile_num_per_layer;
	model = new Layer2[target_layer_num-1];
	if (model==NULL)
	{
		printf("Error, failed to allocate memory for model during model loading");
		return false;
	}

	for (int i=0; i<target_layer_num-1; i++)
	{
		model[i].prev_layer_num = num_per_layer[i];
		model[i].curr_layer_num = num_per_layer[i+1];

		model[i].prev_layer_num_stride = getstride(model[i].prev_layer_num);
		model[i].curr_layer_num_stride = getstride(model[i].curr_layer_num);

		model[i].m_bLoadFromCFile = true;
		if (i==0) {
			model[i].bias = mdlfile_bias1;
			model[i].weights = (float*)mdlfile_weights01;
		}
		else if (i==1) {
			model[i].bias = mdlfile_bias2;
			model[i].weights = (float*)mdlfile_weights12;
		}
		/*else if (i==2) {
			model[i].bias = mdlfile_bias3;
			model[i].weights = mdlfile_weights23;
		}*/
	}

	prior = mdlfile_priors;

	status = true;
	return true;
}

bool 
CDNN2::get_batch_output_org(const float* cepbuffer, int num, int dim, \
									   int& outdim, float* outfea[], NetOutType type)
{
	/*
		cepbuffer	: [IN]	input feature buffer
		num			: [IN]	frame numbers
		dim			: [IN]	frame dimension
		outdim		: [OUT]	bottleneck feature dimension
		outfea		: [OUT]	bottleneck feature buffer
		NetOutType	: [IN]	target layer output type
	*/
	if (!check_model())
	{
		printf("Error, empty model, load first!");
		return false;
	}
	// clear activations to zero before process current feature
	//resetNetAct();
	if (dim != model[0].prev_layer_num)
	{
		printf("Inconsistent between feature dimension \
			   [%d] and network input [%d]", dim, model[0].prev_layer_num);
		return false;
	}
	// allocate memory for output feature
	outdim = model[target_layer_num-2].curr_layer_num;
	assert(outdim >=0 );
	//outfea = new float [ num * outdim ];
	if (outfea==NULL)
	{
		printf("Error, input probability buffer does not exist");
		return false;
	}
	//memset(outfea, 0, num * outdim * sizeof(float) );

	// realigned input feature
	int feadim_stride = getstride(dim);
	if (feadim_stride != model[0].prev_layer_num_stride)
	{
		printf("Inconsistent between aligned feature dimension \
			   [%d] and network input [%d]", feadim_stride, model[0].prev_layer_num_stride);
		return false;
	}
	float* ali_cepbuffer = (float*)new_aligned(sizeof(float), feadim_stride*num);
	if(ali_cepbuffer==NULL)
	{
		printf("Error, fail to allocate memory for feature realign");
		return false;
	}
	int i, j; 
	// copy feature to aligned temporary buffer
	for (i=0; i<num; i++)
	{
		for(j=0; j<dim; j++)
		{
			// feature without normalization
			*(ali_cepbuffer+i*feadim_stride+j) = *(cepbuffer+i*dim+j);
			
			// feature with explicit normalization
			//*(ali_cepbuffer+i*feadim_stride+j) = (*(cepbuffer+i*dim+j) - mean[j])*sdev[j];
		}
	}

	// over all frames
	for(i=0; i<num; i++)
	{
		// go through network until target layer
		for(j=0; j<target_layer_num-1; j++)
		{
			const float* input = (j==0) ? (ali_cepbuffer + i * feadim_stride) : (model[j-1].activations);

#ifdef USE_MKL

			cblas_sgemv(CblasRowMajor, CblasNoTrans, model[j].curr_layer_num, model[j].prev_layer_num_stride, \
				1.0, model[j].weights, model[j].prev_layer_num_stride, input, \
				1,0, model[j].activations, 1);

			cblas_saxpy(model[j].curr_layer_num_stride, 1.0, model[j].bias, 1, model[j].activations, 1);


#endif

#ifdef USE_SSE2
			for(int k=0; k<model[j].curr_layer_num; k++)
			{
				*(model[j].activations + k) = \
					multipy_add_sse2(model[j].weights+k*model[j].prev_layer_num_stride, \
					model[j].prev_layer_num_stride, input, model[j].prev_layer_num_stride, *(model[j].bias+k));
			}
			//vector_add_sse2(model[j].activations, model[j].curr_layer_num_stride, model[j].bias, model[j].curr_layer_num_stride);
#endif
			// arrive at target layer, output type is determined by type flag
			if (j==target_layer_num-2)
			{
				switch(type)
				{
				case LINEAR:
					// do nothing
					break;
				case SIGMOID:
					sigmoid(model[j].curr_layer_num, model[j].activations);
					break;
				case SOFTMAX:
					softmax(model[j].curr_layer_num, model[j].activations);
					break;
				default:
					printf("Unknown output type: ");
					return false;
				}
			}
			else
				sigmoid(model[j].curr_layer_num, model[j].activations);
		}
		// copy activation to output feature buffer
		//memcpy( outfea+i*outdim, model[target_layer_num-2].activations, outdim*sizeof(float) );
		memcpy( outfea[i], model[target_layer_num-2].activations, outdim*sizeof(float) );
	}

	return true;
}

// forward computation in batch mode
bool 
CDNN2::get_batch_output_batch(const float* cepbuffer, int num, int dim, \
									   int& outdim, float* outfea, NetOutType type)
{
	/*
		cepbuffer	: [IN]	input feature buffer
		num			: [IN]	frame numbers (batch number)
		dim			: [IN]	frame dimension
		outdim		: [OUT]	bottleneck feature dimension
		outfea		: [OUT]	bottleneck feature buffer
		NetOutType	: [IN]	target layer output type
	*/

	//Time record.
	//DWORD start_time = GetTickCount();
	if (!check_model())
	{
		printf("Error, empty model, load first!");
		return false;
	}

	if (dim != model[0].prev_layer_num_stride)
	{
		printf("Inconsistent between feature dimension \
			   [%d] and network input [%d]", dim, model[0].prev_layer_num_stride);
		return false;
	}
	outdim = model[target_layer_num-2].curr_layer_num;
	assert( outdim >=0 );
	if (outfea==NULL)
	{
		printf("Error, input probability buffer does not exist");
		return false;
	}

	// allocate temporary memory for intermediate layer's activations
	float** temp_activations = new float* [target_layer_num-1];
	if(temp_activations==NULL)
	{
		printf("Cannot allocate memory for temporary activations");
		return false;
	}
	for(int i=0; i<target_layer_num-1; i++)
	{
		temp_activations[i] = (float*)new_aligned(sizeof(float), num*model[i].curr_layer_num_stride);
		if(temp_activations[i]==NULL)
		{
			printf("Cannot allocate memory for temporary activations");
			return false;
		}
		// init to batch * bias of current layer
		for (int j=0; j<num; j++)
		{
			for(int k=0; k<model[i].curr_layer_num; k++)
				temp_activations[i][j*model[i].curr_layer_num_stride+k] = *(model[i].bias+k);
		}
	}

#ifdef USE_MKL
	// linear operation for first layer
	cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans, num, model[0].curr_layer_num_stride, \
		model[0].prev_layer_num_stride, 1.0, cepbuffer, model[0].prev_layer_num_stride, \
		model[0].weights, model[0].prev_layer_num_stride, 1.0, temp_activations[0], \
		model[0].curr_layer_num_stride);
#else
	integer M = num;
	integer N = model[0].curr_layer_num_stride;
	integer K = model[0].prev_layer_num_stride;
	integer lda = M;
	integer ldb = K;
	integer ldc = M;
	real alpha = 1.0;
	real beta = 1.0;
	real * a = new float[M*K]; scopy(cepbuffer,M*K,a);
	real * b = model[0].weights;
	real * c = temp_activations[0];
	transpose(a,M,K);
	transpose(c,M,N);
	sgemm_("n","n",&M,&N,&K,&alpha,a,&lda,b,&ldb,&beta,c,&ldc);
	transpose(c,N,M);
	delete [] a;
#endif // end USE_MKL

	// sigmoid operation
	for(int i=0; i<num; i++)
		sigmoid(model[0].curr_layer_num, temp_activations[0]+i*model[0].curr_layer_num_stride);

	// loop over intermediate layers
	for(int i=1; i<target_layer_num-1; i++)
	{
#ifdef USE_MKL
		// linear operation for layer
		cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasTrans, num, model[i].curr_layer_num_stride, \
			model[i].prev_layer_num_stride, 1.0, temp_activations[i-1], model[i].prev_layer_num_stride, \
			model[i].weights, model[i].prev_layer_num_stride, 1.0, temp_activations[i], \
			model[i].curr_layer_num_stride);
#else
	integer M = num;
	integer N = model[i].curr_layer_num_stride;
	integer K = model[i].prev_layer_num_stride;
	integer lda = M;
	integer ldb = K;
	integer ldc = M;
	real alpha = 1.0;
	real beta = 1.0;
	real * a = temp_activations[i-1];
	real * b = model[i].weights;
	real * c = temp_activations[i];
	transpose(a,M,K);
	transpose(c,M,N);
	sgemm_("n","n",&M,&N,&K,&alpha,a,&lda,b,&ldb,&beta,c,&ldc);
	transpose(c,N,M);
	transpose(a,K,M);
#endif

		// arrive at target layer
		if (i==target_layer_num-2)
		{
			switch(type)
			{
			case LINEAR:
				// do nothing
				break;
			case SIGMOID:
				for(int j=0; j<num; j++) {sigmoid(model[i].curr_layer_num, temp_activations[i]+j*model[i].curr_layer_num_stride);}
				break;
			case SOFTMAX:
				for(int j=0; j<num; j++) {
					softmax(model[i].curr_layer_num, temp_activations[i]+j*model[i].curr_layer_num_stride);
					logvec(model[i].curr_layer_num, temp_activations[i]+j*model[i].curr_layer_num_stride);
				}
				break;
			default:
				printf("Unknown output type: ");
				return false;
			}
		}
		// intermediate layer
		else
			for(int j=0; j<num; j++) {sigmoid(model[i].curr_layer_num, temp_activations[i]+j*model[i].curr_layer_num_stride);}
	}

	
	// now copy temp_activations to output buffer
	switch(type)
	{
	case LINEAR:
		// do nothing
		for(int i=0; i<num; i++)
		{
			for(int j=0; j<model[target_layer_num-2].curr_layer_num; j++)
				//outfea[i][j+1] = 0.8f*(temp_activations[target_layer_num-2][i*model[target_layer_num-2].curr_layer_num_stride+j] - prior[j]);
				outfea[i*outdim+j] = 0.8f*(temp_activations[target_layer_num-2][i*model[target_layer_num-2].curr_layer_num_stride+j] - prior[j]);
		}
		break;
	case SIGMOID:
		break;
	case SOFTMAX:
		for(int i=0; i<num; i++)
		{
			for(int j=0; j<model[target_layer_num-2].curr_layer_num; j++)
				//outfea[i][j+1] = temp_activations[target_layer_num-2][i*model[target_layer_num-2].curr_layer_num_stride+j] - prior[j];
				//outfea[i*outdim+j] = temp_activations[target_layer_num-2][i*model[target_layer_num-2].curr_layer_num_stride+j] - prior[j];
                outfea[i*outdim+j] = temp_activations[target_layer_num-2][i*model[target_layer_num-2].curr_layer_num_stride+j];
		}
		break;
	default:
		printf("Unknown output type: ");
		return false;
	}

	// release temp_activations
	for(int i=0; i<target_layer_num-1; i++)
	{
		if(temp_activations[i]!=NULL)
			del_aligned((char*&)temp_activations[i]);
	}
	if(temp_activations!=NULL)
	{
		delete [] temp_activations;
		temp_activations = NULL;
	}

	//DWORD stop_time = GetTickCount();
	//printf("[INFOR]: Time consumed : %d ms\n", stop_time - start_time);

	return true;
}

void CDNN2::resetNetAct()
{
	if(model==NULL)
	{
		printf("Empty model, load first!\n");
		exit(-1);
	}
	for(int i=0; i<target_layer_num; i++)
		memset(model[i].activations, 0, sizeof(float)*model[i].curr_layer_num_stride);
}

void
CDNN2::print_model_org(const char* outmod) const
{
	if (model == NULL)
	{
		printf("Error, empty model, load first!");
		return;
	}
	
	FILE* fp = fopen(outmod, "wt");
	int i,j,k;
	for (i=0; i<target_layer_num-1; i++)
	{
		// output matrix, row major
		fprintf(fp, "Weight between layer %d and %d\n", i+1, i+2);
		for (j=0; j<model[i].curr_layer_num; j++)
		{
			for (k=0; k<model[i].prev_layer_num; k++)
				fprintf(fp, "%f ", *(model[i].weights+j*model[i].prev_layer_num_stride+k));
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n\n");
		// output bias
		fprintf(fp, "Bias of layer %d\n", i+2);
		for (j=0; j<model[i].curr_layer_num; j++)
			fprintf(fp, "%f ", *(model[i].bias+j));
		fprintf(fp, "\n");
	}
	fclose(fp);
}

void
CDNN2::print_model_batch(const char* outmod) const
{
	if (model == NULL)
	{
		printf("Error, empty model, load first!");
		return;
	}

	FILE* fp = fopen(outmod, "wt");
	int i,j,k;
	for (i=0; i<target_layer_num-1; i++)
	{
		// output matrix, row major
		fprintf(fp, "Weight between layer %d and %d\n", i+1, i+2);
		for (j=0; j<model[i].prev_layer_num; j++)
		{
			for (k=0; k<model[i].curr_layer_num; k++)
				fprintf(fp, "%f ", *(model[i].weights+j*model[i].curr_layer_num_stride+k));
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n\n");
		// output bias
		fprintf(fp, "Bias of layer %d\n", i+2);
		for (j=0; j<model[i].curr_layer_num; j++)
			fprintf(fp, "%f ", *(model[i].bias+j));
		fprintf(fp, "\n");
	}
	fclose(fp);
}
