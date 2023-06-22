#ifndef _DFKE_CBLAS_H_
#define _DFKE_CBLAS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "f2c.h"

void transpose(float * x, int row, int col);
void scopy(const float * x,int n, float * y);

int sgemm_(const char *transa, const char *transb, 
	   const integer *m, const integer *n, const integer *k, 
	   const real *alpha, const real *a, const integer *lda,const real *b, const integer *ldb, 
	   const real *beta, real *c, const integer *ldc);

#ifdef __cplusplus
}
#endif

#endif

