/*

   BLIS    
   An object-based framework for developing high-performance BLAS-like
   libraries.

   Copyright (C) 2014, The University of Texas at Austin

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    - Neither the name of The University of Texas at Austin nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef BLIS_KERNEL_POST_MACRO_DEFS_H
#define BLIS_KERNEL_POST_MACRO_DEFS_H

/*
// -- Define PASTEMAC-friendly kernel function name macros ---------------------

//
// Level-3
//

// gemm micro-kernels

#define bli_sGEMM_UKERNEL BLIS_SGEMM_UKERNEL
#define bli_dGEMM_UKERNEL BLIS_DGEMM_UKERNEL
#define bli_cGEMM_UKERNEL BLIS_CGEMM_UKERNEL
#define bli_zGEMM_UKERNEL BLIS_ZGEMM_UKERNEL

// gemmtrsm_l micro-kernels

#define bli_sGEMMTRSM_L_UKERNEL BLIS_SGEMMTRSM_L_UKERNEL
#define bli_dGEMMTRSM_L_UKERNEL BLIS_DGEMMTRSM_L_UKERNEL
#define bli_cGEMMTRSM_L_UKERNEL BLIS_CGEMMTRSM_L_UKERNEL
#define bli_zGEMMTRSM_L_UKERNEL BLIS_ZGEMMTRSM_L_UKERNEL

// gemmtrsm_u micro-kernels

#define bli_sGEMMTRSM_U_UKERNEL BLIS_SGEMMTRSM_U_UKERNEL
#define bli_dGEMMTRSM_U_UKERNEL BLIS_DGEMMTRSM_U_UKERNEL
#define bli_cGEMMTRSM_U_UKERNEL BLIS_CGEMMTRSM_U_UKERNEL
#define bli_zGEMMTRSM_U_UKERNEL BLIS_ZGEMMTRSM_U_UKERNEL

// trsm_l micro-kernels

#define bli_sTRSM_L_UKERNEL BLIS_STRSM_L_UKERNEL
#define bli_dTRSM_L_UKERNEL BLIS_DTRSM_L_UKERNEL
#define bli_cTRSM_L_UKERNEL BLIS_CTRSM_L_UKERNEL
#define bli_zTRSM_L_UKERNEL BLIS_ZTRSM_L_UKERNEL

// trsm_u micro-kernels

#define bli_sTRSM_U_UKERNEL BLIS_STRSM_U_UKERNEL
#define bli_dTRSM_U_UKERNEL BLIS_DTRSM_U_UKERNEL
#define bli_cTRSM_U_UKERNEL BLIS_CTRSM_U_UKERNEL
#define bli_zTRSM_U_UKERNEL BLIS_ZTRSM_U_UKERNEL

//
// Level-3 4m
//

// gemm4m micro-kernels

#define bli_cGEMM4M1_UKERNEL BLIS_CGEMM4M1_UKERNEL
#define bli_zGEMM4M1_UKERNEL BLIS_ZGEMM4M1_UKERNEL

// gemmtrsm4m_l micro-kernels

#define bli_cGEMMTRSM4M1_L_UKERNEL BLIS_CGEMMTRSM4M1_L_UKERNEL
#define bli_zGEMMTRSM4M1_L_UKERNEL BLIS_ZGEMMTRSM4M1_L_UKERNEL

// gemmtrsm4m_u micro-kernels

#define bli_cGEMMTRSM4M1_U_UKERNEL BLIS_CGEMMTRSM4M1_U_UKERNEL
#define bli_zGEMMTRSM4M1_U_UKERNEL BLIS_ZGEMMTRSM4M1_U_UKERNEL

// trsm4m_l micro-kernels

#define bli_cTRSM4M1_L_UKERNEL BLIS_CTRSM4M1_L_UKERNEL
#define bli_zTRSM4M1_L_UKERNEL BLIS_ZTRSM4M1_L_UKERNEL

// trsm4m_u micro-kernels

#define bli_cTRSM4M1_U_UKERNEL BLIS_CTRSM4M1_U_UKERNEL
#define bli_zTRSM4M1_U_UKERNEL BLIS_ZTRSM4M1_U_UKERNEL

//
// Level-3 3m
//

// gemm3m micro-kernels

#define bli_cGEMM3M1_UKERNEL BLIS_CGEMM3M1_UKERNEL
#define bli_zGEMM3M1_UKERNEL BLIS_ZGEMM3M1_UKERNEL

// gemmtrsm3m_l micro-kernels

#define bli_cGEMMTRSM3M1_L_UKERNEL BLIS_CGEMMTRSM3M1_L_UKERNEL
#define bli_zGEMMTRSM3M1_L_UKERNEL BLIS_ZGEMMTRSM3M1_L_UKERNEL

// gemmtrsm3m_u micro-kernels

#define bli_cGEMMTRSM3M1_U_UKERNEL BLIS_CGEMMTRSM3M1_U_UKERNEL
#define bli_zGEMMTRSM3M1_U_UKERNEL BLIS_ZGEMMTRSM3M1_U_UKERNEL

// trsm3m_l micro-kernels

#define bli_cTRSM3M1_L_UKERNEL BLIS_CTRSM3M1_L_UKERNEL
#define bli_zTRSM3M1_L_UKERNEL BLIS_ZTRSM3M1_L_UKERNEL

// trsm3m_u micro-kernels

#define bli_cTRSM3M1_U_UKERNEL BLIS_CTRSM3M1_U_UKERNEL
#define bli_zTRSM3M1_U_UKERNEL BLIS_ZTRSM3M1_U_UKERNEL

//
// Level-1m
//

// NOTE: We don't need any PASTEMAC-friendly aliases to packm kernel
// macros because they are used directly in the initialization of the
// function pointer array, rather than via a templatizing wrapper macro.


//
// Level-1f
//

// axpy2v kernels

#define bli_sssAXPY2V_KERNEL BLIS_SAXPY2V_KERNEL
#define bli_dddAXPY2V_KERNEL BLIS_DAXPY2V_KERNEL
#define bli_cccAXPY2V_KERNEL BLIS_CAXPY2V_KERNEL
#define bli_zzzAXPY2V_KERNEL BLIS_ZAXPY2V_KERNEL

// dotaxpyv kernels

#define bli_sssDOTAXPYV_KERNEL BLIS_SDOTAXPYV_KERNEL
#define bli_dddDOTAXPYV_KERNEL BLIS_DDOTAXPYV_KERNEL
#define bli_cccDOTAXPYV_KERNEL BLIS_CDOTAXPYV_KERNEL
#define bli_zzzDOTAXPYV_KERNEL BLIS_ZDOTAXPYV_KERNEL

// axpyf kernels

#define bli_sssAXPYF_KERNEL BLIS_SAXPYF_KERNEL
#define bli_dddAXPYF_KERNEL BLIS_DAXPYF_KERNEL
#define bli_cccAXPYF_KERNEL BLIS_CAXPYF_KERNEL
#define bli_zzzAXPYF_KERNEL BLIS_ZAXPYF_KERNEL

// dotxf kernels

#define bli_sssDOTXF_KERNEL BLIS_SDOTXF_KERNEL
#define bli_dddDOTXF_KERNEL BLIS_DDOTXF_KERNEL
#define bli_cccDOTXF_KERNEL BLIS_CDOTXF_KERNEL
#define bli_zzzDOTXF_KERNEL BLIS_ZDOTXF_KERNEL

// dotxaxpyf kernels

#define bli_sssDOTXAXPYF_KERNEL BLIS_SDOTXAXPYF_KERNEL
#define bli_dddDOTXAXPYF_KERNEL BLIS_DDOTXAXPYF_KERNEL
#define bli_cccDOTXAXPYF_KERNEL BLIS_CDOTXAXPYF_KERNEL
#define bli_zzzDOTXAXPYF_KERNEL BLIS_ZDOTXAXPYF_KERNEL


//
// Level-1v
//

// addv kernels

#define bli_ssADDV_KERNEL BLIS_SADDV_KERNEL
#define bli_ddADDV_KERNEL BLIS_DADDV_KERNEL
#define bli_ccADDV_KERNEL BLIS_CADDV_KERNEL
#define bli_zzADDV_KERNEL BLIS_ZADDV_KERNEL

// axpyv kernels

#define bli_sssAXPYV_KERNEL BLIS_SAXPYV_KERNEL
#define bli_dddAXPYV_KERNEL BLIS_DAXPYV_KERNEL
#define bli_cccAXPYV_KERNEL BLIS_CAXPYV_KERNEL
#define bli_zzzAXPYV_KERNEL BLIS_ZAXPYV_KERNEL

// copyv kernels

#define bli_ssCOPYV_KERNEL BLIS_SCOPYV_KERNEL
#define bli_ddCOPYV_KERNEL BLIS_DCOPYV_KERNEL
#define bli_ccCOPYV_KERNEL BLIS_CCOPYV_KERNEL
#define bli_zzCOPYV_KERNEL BLIS_ZCOPYV_KERNEL

// dotv kernels

#define bli_sssDOTV_KERNEL BLIS_SDOTV_KERNEL
#define bli_dddDOTV_KERNEL BLIS_DDOTV_KERNEL
#define bli_cccDOTV_KERNEL BLIS_CDOTV_KERNEL
#define bli_zzzDOTV_KERNEL BLIS_ZDOTV_KERNEL

// dotxv kernels

#define bli_sssDOTXV_KERNEL BLIS_SDOTXV_KERNEL
#define bli_dddDOTXV_KERNEL BLIS_DDOTXV_KERNEL
#define bli_cccDOTXV_KERNEL BLIS_CDOTXV_KERNEL
#define bli_zzzDOTXV_KERNEL BLIS_ZDOTXV_KERNEL

// invertv kernels

#define bli_sINVERTV_KERNEL BLIS_SINVERTV_KERNEL
#define bli_dINVERTV_KERNEL BLIS_DINVERTV_KERNEL
#define bli_cINVERTV_KERNEL BLIS_CINVERTV_KERNEL
#define bli_zINVERTV_KERNEL BLIS_ZINVERTV_KERNEL

// scal2v kernels

#define bli_sssSCAL2V_KERNEL BLIS_SSCAL2V_KERNEL
#define bli_dddSCAL2V_KERNEL BLIS_DSCAL2V_KERNEL
#define bli_cccSCAL2V_KERNEL BLIS_CSCAL2V_KERNEL
#define bli_zzzSCAL2V_KERNEL BLIS_ZSCAL2V_KERNEL

// scalv kernels

#define bli_ssSCALV_KERNEL BLIS_SSCALV_KERNEL
#define bli_ddSCALV_KERNEL BLIS_DSCALV_KERNEL
#define bli_ccSCALV_KERNEL BLIS_CSCALV_KERNEL
#define bli_zzSCALV_KERNEL BLIS_ZSCALV_KERNEL

// setv kernels

#define bli_ssSETV_KERNEL BLIS_SSETV_KERNEL
#define bli_ddSETV_KERNEL BLIS_DSETV_KERNEL
#define bli_ccSETV_KERNEL BLIS_CSETV_KERNEL
#define bli_zzSETV_KERNEL BLIS_ZSETV_KERNEL

// subv kernels

#define bli_ssSUBV_KERNEL BLIS_SSUBV_KERNEL
#define bli_ddSUBV_KERNEL BLIS_DSUBV_KERNEL
#define bli_ccSUBV_KERNEL BLIS_CSUBV_KERNEL
#define bli_zzSUBV_KERNEL BLIS_ZSUBV_KERNEL

// swapv kernels

#define bli_ssSWAPV_KERNEL BLIS_SSWAPV_KERNEL
#define bli_ddSWAPV_KERNEL BLIS_DSWAPV_KERNEL
#define bli_ccSWAPV_KERNEL BLIS_CSWAPV_KERNEL
#define bli_zzSWAPV_KERNEL BLIS_ZSWAPV_KERNEL
*/


// -- Maximum register blocksize search ----------------------------------------

// The macro-kernels oftentimes need to statically allocate a temporary
// MR x NR micro-tile of C. This micro-tile must be sized such that it will
// work for both native and 4m/3m implementations, since the user can switch
// between them at runtime. In order to facilitate the sizing of those
// micro-tiles, we must determine the largest the register blocksizes would
// need to be to accommodate both native and 4m/3m-based complex
// micro-kernels. For real datatypes, the maximum is never larger than the
// actual s and d register blocksizes. However, for complex datatypes, the
// "native" register blocksizes may differ from the "virtual" register
// blocksizes used by the 4m/3m implementations. Usually, it is the register
// blocksizes used for 4m/3m-based complex micro-kernels that would be
// larger, and thus determine the maximum for c and z datatypes. But, we
// prefer not to assume this, therefore, we always take the larger of the
// two values.

#define BLIS_DEFAULT_4M_MR_C BLIS_DEFAULT_MR_S
#define BLIS_DEFAULT_4M_NR_C BLIS_DEFAULT_NR_S
#define BLIS_DEFAULT_4M_MR_Z BLIS_DEFAULT_MR_D
#define BLIS_DEFAULT_4M_NR_Z BLIS_DEFAULT_NR_D

//
// Find the largest register blocksize MR.
//

#define BLIS_MAX_DEFAULT_MR_S BLIS_DEFAULT_MR_S
#define BLIS_MAX_DEFAULT_MR_D BLIS_DEFAULT_MR_D

// NOTE: 4m and 3m register blocksizes are assumed to be equal. Thus,
// we only inspect the 4m values.

// c: Choose between the regular and 4m/3m blocksize.
#define BLIS_MAX_DEFAULT_MR_C BLIS_DEFAULT_MR_C
#if     BLIS_DEFAULT_4M_MR_C > BLIS_MAX_DEFAULT_MR_C
#undef  BLIS_MAX_DEFAULT_MR_C
#define BLIS_MAX_DEFAULT_MR_C BLIS_DEFAULT_4M_MR_C
#endif

// z: Choose between the regular and 4m/3m blocksize.
#define BLIS_MAX_DEFAULT_MR_Z BLIS_DEFAULT_MR_Z
#if     BLIS_DEFAULT_4M_MR_Z > BLIS_MAX_DEFAULT_MR_Z
#undef  BLIS_MAX_DEFAULT_MR_Z
#define BLIS_MAX_DEFAULT_MR_Z BLIS_DEFAULT_4M_MR_Z
#endif

//
// Find the largest register blocksize NR.
//

#define BLIS_MAX_DEFAULT_NR_S BLIS_DEFAULT_NR_S
#define BLIS_MAX_DEFAULT_NR_D BLIS_DEFAULT_NR_D

// NOTE: 4m and 3m register blocksizes are assumed to be equal. Thus,
// we only inspect the 4m values.

// c: Choose between the regular and 4m/3m blocksize.
#define BLIS_MAX_DEFAULT_NR_C BLIS_DEFAULT_NR_C
#if     BLIS_DEFAULT_4M_NR_C > BLIS_MAX_DEFAULT_NR_C
#undef  BLIS_MAX_DEFAULT_NR_C
#define BLIS_MAX_DEFAULT_NR_C BLIS_DEFAULT_4M_NR_C
#endif

// z: Choose between the regular and 4m/3m blocksize.
#define BLIS_MAX_DEFAULT_NR_Z BLIS_DEFAULT_NR_Z
#if     BLIS_DEFAULT_4M_NR_Z > BLIS_MAX_DEFAULT_NR_Z
#undef  BLIS_MAX_DEFAULT_NR_Z
#define BLIS_MAX_DEFAULT_NR_Z BLIS_DEFAULT_4M_NR_Z
#endif


// -- Abbreiviated macros ------------------------------------------------------

// Here, we shorten the maximum blocksizes found above so that they can be
// derived via the PASTEMAC macro.

// Maximum MR blocksizes

#define bli_smaxmr   BLIS_MAX_DEFAULT_MR_S
#define bli_dmaxmr   BLIS_MAX_DEFAULT_MR_D
#define bli_cmaxmr   BLIS_MAX_DEFAULT_MR_C
#define bli_zmaxmr   BLIS_MAX_DEFAULT_MR_Z

// Maximum NR blocksizes

#define bli_smaxnr   BLIS_MAX_DEFAULT_NR_S
#define bli_dmaxnr   BLIS_MAX_DEFAULT_NR_D
#define bli_cmaxnr   BLIS_MAX_DEFAULT_NR_C
#define bli_zmaxnr   BLIS_MAX_DEFAULT_NR_Z


#endif 

