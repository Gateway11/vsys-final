#include <stdint.h>
#include <string.h>
#include "HED_SM3_Soft.h"
#define HASH_U32                  SM3_U32
#define rev(x)                    (((x)>>24)|((x)<<24)|(((x)>>8)&0xFF00)|(((x)<<8)&0xFF0000))
#define IS_WD_PTR(ptr)            ((((HASH_U32)(ptr))&0x3)==0x0)
static HASH_U32 is_big_endian ( void )
{
    const HASH_U32 val[1] = {0x1};
    return *((const unsigned char *)val) == 0x0;
}
static void trans_data(HASH_U32 *dst, const HASH_U32 *src, HASH_U32 len )
{
    HASH_U32 i;

    if ( !is_big_endian() )
    {
        for ( i = 0; i < len; i++ )
        {
            dst[i] = rev(src[i]);
        }
    }
    else  if (dst != src)
    {
        for ( i = 0; i < len; i++ )
        {
            dst[i] = src[i];
        }

    }

}
#define  NOT_TRS
#ifdef  NOT_TRS
#define M_TRS_IV(dst, src, len)     if((dst)!=(src))memcpy(dst, src, (len)*4)            /*  */
#else      /* -----  not NOT_TRS  ----- */
#define M_TRS_IV(dst, src, len)     trans_data(dst, src, len)            /*  */
#endif     /* -----  not NOT_TRS  ----- */
#define HASH_IV_LEN               (SM3_IV_WLEN*4)
#define HASH_IV_WLEN              SM3_IV_WLEN
#define HASH_BLK_WLEN             SM3_BLK_WLEN
#define HASH_BLK_LEN              (SM3_BLK_WLEN*4)
#define IV_CONST                  SM3IvConst
#define HASH_INFO                 SM3Info
#define H_INIT                    HED_SM3_Init_Soft
#define H_UPDATE                  HED_SM3_Update_Soft
#define H_FINAL                   HED_SM3_Final_Soft
#define HASH_CALC                 HED_SM3_Calc_Soft
static const HASH_U32 SM3IvConst[] =
{
    0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600,
    0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e, 0x00000000
};
static void LOCAL_PK(HASH_INFO *pIvs, const void * Mblock)
{
    int i,j;
    HASH_U32 A,B,C,D,E,F,G,H,tmp;
    HASH_U32 SS1,SS2,TT1,TT2,T;
    HASH_U32 W0[68];

#if  0
#else
    const HASH_U32 *ptrMess;
#endif
    A=pIvs->Iv[0];
    B=pIvs->Iv[1];
    C=pIvs->Iv[2];
    D=pIvs->Iv[3];
    E=pIvs->Iv[4];
    F=pIvs->Iv[5];
    G=pIvs->Iv[6];
    H=pIvs->Iv[7];

#if  0
    memcpy(W0, Mblock, HASH_BLK_LEN);
#else

    if ( IS_WD_PTR(Mblock) )
    {
        ptrMess = (const HASH_U32 *)Mblock;
    }
    else
    {
        memcpy(W0, Mblock, HASH_BLK_LEN);
        ptrMess = W0;
    }
    trans_data(W0, ptrMess, HASH_BLK_WLEN);

#endif

    for (i=16; i<68; i++)
    {
        tmp=(((W0[i-3])<<(15))|((W0[i-3])>>(32-15)))^W0[i-9]^W0[i-16];
        W0[i]=(tmp ^ (((tmp)<<(15))|((tmp)>>(32-15))) ^ (((tmp)<<(23))|((tmp)>>(32-23))))^(((W0[i-13])<<(7))|((W0[i-13])>>(32-7)))^W0[i-6];
    }
    T=0x79CC4519;
    for (i=0; i<16; i++)
    {
        SS2=(((A)<<(12))|((A)>>(32-12)));
        SS1=SS2+E+(((T)<<(i))|((T)>>(32-i)));
        SS1=(((SS1)<<(7))|((SS1)>>(32-7)));
        SS2^=SS1;
        TT1=(A^B^C)+D+SS2+(W0[i]^W0[i+4]);
        TT2=(E^F^G)+H+SS1+W0[i];
        D=C;
        C= (((B)<<(9))|((B)>>(32-9)));
        B=A;
        A=TT1;
        H=G;
        G=(((F)<<(19))|((F)>>(32-19)));
        F=E;
        E=(TT2 ^ (((TT2)<<(9))|((TT2)>>(32-9))) ^ (((TT2)<<(17))|((TT2)>>(32-17))));
    }
    T=0x7A879D8A;
    for(i=16; i<64; i++)
    {
        SS2=(((A)<<(12))|((A)>>(32-12)));
        j=i%32;
        SS1=SS2+E+(((T)<<(j))|((T)>>(32-j)));
        SS1=(((SS1)<<(7))|((SS1)>>(32-7)));
        SS2^=SS1;
        TT1=((A&B)|(A&C)|(B&C))+D+SS2+(W0[i]^W0[i+4]);
        TT2=((E&F)|((~E)&G))+H+SS1+W0[i];
        D=C;
        C=(((B)<<(9))|((B)>>(32-9)));
        B=A;
        A=TT1;
        H=G;
        G=(((F)<<(19))|((F)>>(32-19)));
        F=E;
        E=(TT2 ^ (((TT2)<<(9))|((TT2)>>(32-9))) ^ (((TT2)<<(17))|((TT2)>>(32-17))));
    }
    pIvs->Iv[0]^=A;
    pIvs->Iv[1]^=B;
    pIvs->Iv[2]^=C;
    pIvs->Iv[3]^=D;
    pIvs->Iv[4]^=E;
    pIvs->Iv[5]^=F;
    pIvs->Iv[6]^=G;
    pIvs->Iv[7]^=H;
}



unsigned int H_INIT(HASH_INFO *c)
{
    M_TRS_IV(c->Iv, IV_CONST, HASH_IV_WLEN);
    c->total = 0x0;
    c->num = 0;
    return 0;
}
unsigned int H_UPDATE(HASH_INFO *c, const void *data, unsigned int ilen)
{
    unsigned char *ptrBuf, *pMess;
    HASH_U32 currLen;

    if (ilen == 0)
        return 0x0;


    ptrBuf = (unsigned char *)c->buff;

    if ((c->num+ilen) <  HASH_BLK_LEN)
    {
        memcpy(ptrBuf+c->num, data, ilen);
        c->num += ilen;
        return 0x0;
    }                               /* -----  end if  ----- */
    pMess = (unsigned char *)data;

    M_TRS_IV(c->Iv, c->Iv, HASH_IV_WLEN);
    currLen = c->num +ilen;

    if ( c->num != 0 )
    {
        memcpy(ptrBuf+c->num, pMess, HASH_BLK_LEN-c->num);
        LOCAL_PK(c, c->buff);
        c->total += HASH_BLK_LEN;
        currLen -= HASH_BLK_LEN;
        pMess += HASH_BLK_LEN-c->num;
    }                               /* -----  end if  ----- */

    while ( currLen >= HASH_BLK_LEN)
    {
        LOCAL_PK(c, pMess);
        c->total += HASH_BLK_LEN;
        currLen -= HASH_BLK_LEN;
        pMess += HASH_BLK_LEN;
    }
    c->num = currLen;

    if ( currLen )
    {
        memcpy(ptrBuf, pMess, currLen);
    }                               /* -----  end if  ----- */
    M_TRS_IV(c->Iv, c->Iv, HASH_IV_WLEN);
    return 0x0;

}
unsigned int H_FINAL(unsigned char *md, HASH_INFO *c)
{
    unsigned char *p = (unsigned char *)c->buff;
    HASH_U32 n = c->num;
    c->total += n;
    c->num = 0;
    p[n] = 0x80;                /* there is always room for one */
    n++;

    M_TRS_IV(c->Iv, c->Iv, HASH_IV_WLEN);
    if (n > (HASH_BLK_LEN - 8))
    {
        memset(p + n, 0, HASH_BLK_LEN - n);
        n = 0;
        LOCAL_PK(c, c->buff);
        p = (unsigned char *)c->buff;
    }
    memset(p + n, 0, HASH_BLK_LEN-n);

    p += HASH_BLK_LEN - 8;

    n = c->total<<3;
    trans_data(c->buff+HASH_BLK_WLEN-1, &n, 1);

    LOCAL_PK(c, c->buff);

    trans_data(c->Iv, c->Iv, HASH_IV_WLEN);
    memcpy(md, c->Iv, HASH_IV_LEN);
    return 0;

}


void HASH_CALC(const void *d, unsigned int ilen, unsigned char *md)
{
    unsigned int st;

    HASH_INFO ctx;

    st = H_INIT( &ctx );
    st |= H_UPDATE( &ctx, d, ilen );
    st |= H_FINAL(md, &ctx);

}

