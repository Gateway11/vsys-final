//
//  zmacro.h
//  r2vt4
//
//  Created by hadoop on 10/26/17.
//  Copyright © 2017 hadoop. All rights reserved.
//

#ifndef __r2vt4__zmacro_h
#define __r2vt4__zmacro_h

//QuickLogAdd--------------------------------------------------------------------------------------------------------------------------------
#define QuickLogAdd(x,y) (x>y)?((x-y)>QUICK_EXP_SHIELD?x:x+ZMat::m_plogtab1[(int)((x-y)*QUICK_EXP_CONSTANT_3 + QUICK_EXP_CONSTANT_1)]) \
: ((y-x)> QUICK_EXP_SHIELD?y:y+ZMat::m_plogtab1[(int)((y-x)*QUICK_EXP_CONSTANT_3+QUICK_EXP_CONSTANT_1)])


//NEON_Matrix4Tran2--------------------------------------------------------------------------------------------------------------------------------
#if defined(__arm__)
#define NEON_Matrix4Tran2(m,n) \
__asm__ volatile(\
"vldmia %0, { q0-q3 }  \n\t"\
"vtrn.f32     q0, q1	\n\t"\
"vtrn.f32     q2, q3	\n\t"\
"vswp        d1, d4	\n\t"\
"vswp        d3, d6	\n\t"\
"vstmia %1, { q0-q3 }" \
:\
: "r" (m), "r" (n)\
: "memory", "q0", "q1", "q2", "q3"\
);

#elif defined(__aarch64__)

#define NEON_Matrix4Tran2(m,n) \
__asm__ volatile(\
"ld1 { v0.4s-v3.4s }, [%0] \n\t"\
"st4 { v0.4s-v3.4s }, [%1]"\
:\
: "r" (m), "r" (n)\
: "memory", "v0", "v1", "v2", "v3"\
);

#else

#define NEON_Matrix4Tran2(m,n) \
{\
float* pData_Tmp = Z_SAFE_NEW_AR1(pData_Tmp, float, 16);\
pData_Tmp[0] = m[0];pData_Tmp[1] = m[4];pData_Tmp[2] = m[8];pData_Tmp[3] = m[12];\
pData_Tmp[4] = m[1];pData_Tmp[5] = m[5];pData_Tmp[6] = m[9];pData_Tmp[7] = m[13];\
pData_Tmp[8] = m[2];pData_Tmp[9] = m[6];pData_Tmp[10] = m[10];pData_Tmp[11] = m[14];\
pData_Tmp[12] = m[3];pData_Tmp[13] = m[7];pData_Tmp[14] = m[11];pData_Tmp[15] = m[15];\
memcpy(n, pData_Tmp,sizeof(float) * 16);\
Z_SAFE_DEL_AR1(pData_Tmp)\
}
#endif


//qmempy4------------------------------------------------------------------------------------------------------------------------
#if defined(__arm__)
#define qmempy4(a,b) \
__asm__ volatile(\
"vld1.32 {q0}, [%1]  \n\t"\
"vst1.32 {q0}, [%0] " \
:\
: "r" (a), "r" (b)\
: "memory", "q0"\
);
#else
#define qmempy4(a,b) memcpy(a,b,16)
#endif

#endif /* zmacro_h */
