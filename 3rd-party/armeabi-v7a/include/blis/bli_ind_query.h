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

#ifndef BLIS_IND_QUERY_H
#define BLIS_IND_QUERY_H


typedef enum
{
	BLIS_3MH = 0,
	BLIS_3M1,
	BLIS_4MH,
	BLIS_4M1B,
	BLIS_4M1A,
	BLIS_NAT,
} ind_t;

#define BLIS_NUM_IND_METHODS 6

// -----------------------------------------------------------------------------

/*
bool_t bli_gemmind_has_avail( num_t dt );
bool_t bli_hemmind_has_avail( num_t dt );
bool_t bli_herkind_has_avail( num_t dt );
bool_t bli_her2kind_has_avail( num_t dt );
bool_t bli_symmind_has_avail( num_t dt );
bool_t bli_syrkind_has_avail( num_t dt );
bool_t bli_syr2kind_has_avail( num_t dt );
bool_t bli_trmm3ind_has_avail( num_t dt );
bool_t bli_trmmind_has_avail( num_t dt );
bool_t bli_trsmind_has_avail( num_t dt );

void*  bli_gemmind_get_avail( num_t dt );
void*  bli_hemmind_get_avail( num_t dt );
void*  bli_herkind_get_avail( num_t dt );
void*  bli_her2kind_get_avail( num_t dt );
void*  bli_symmind_get_avail( num_t dt );
void*  bli_syrkind_get_avail( num_t dt );
void*  bli_syr2kind_get_avail( num_t dt );
void*  bli_trmm3ind_get_avail( num_t dt );
void*  bli_trmmind_get_avail( num_t dt );
void*  bli_trsmind_get_avail( num_t dt );
*/

#undef  GENPROT
#define GENPROT( opname ) \
\
bool_t PASTEMAC(opname,ind_has_avail)( num_t dt ); \
void*  PASTEMAC(opname,ind_get_avail)( num_t dt );

GENPROT( gemm )
GENPROT( hemm )
GENPROT( herk )
GENPROT( her2k )
GENPROT( symm )
GENPROT( syrk )
GENPROT( syr2k )
GENPROT( trmm3 )
GENPROT( trmm )
GENPROT( trsm )

// -----------------------------------------------------------------------------

bool_t bli_ind_oper_is_impl( opid_t oper, ind_t method );

//bool_t bli_ind_oper_is_avail( opid_t oper, ind_t method, num_t dt );

bool_t bli_ind_oper_has_avail( opid_t oper, num_t dt );
void*  bli_ind_oper_get_avail( opid_t oper, num_t dt );

ind_t  bli_ind_oper_find_avail( opid_t oper, num_t dt );

char*  bli_ind_oper_get_avail_impl_string( opid_t oper, num_t dt );

void   bli_ind_init( void );

void   bli_ind_enable( ind_t method );
void   bli_ind_disable( ind_t method );
void   bli_ind_disable_all( void );

void   bli_ind_enable_dt( ind_t method, num_t dt );
void   bli_ind_disable_dt( ind_t method, num_t dt );
void   bli_ind_disable_all_dt( num_t dt );

void   bli_ind_set_enable_dt( ind_t method, num_t dt, bool_t status );

void   bli_ind_oper_enable_only( opid_t oper, ind_t method, num_t dt );
void   bli_ind_oper_set_enable_all( opid_t oper, num_t dt, bool_t status );

void   bli_ind_oper_set_enable( opid_t oper, ind_t method, num_t dt, bool_t status );
bool_t bli_ind_oper_get_enable( opid_t oper, ind_t method, num_t dt );

void*  bli_ind_oper_get_func( opid_t oper, ind_t method );

char*  bli_ind_get_impl_string( ind_t method );

num_t  bli_ind_map_cdt_to_index( num_t dt );


#endif

