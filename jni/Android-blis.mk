LOCAL_PATH:= $(call my-dir)

#https://github.com/flame/blis.git
#configure auto
CBLAS_RELATIVE_PATH := ../3rd-party/blis/frame/compat/cblas

include $(CLEAR_VARS)
LOCAL_MODULE    := blas
LOCAL_SRC_FILES := $(shell find -L ../3rd-party/BLAS2/SRC -name "*.c")

#the inclusion of BLASWRAP caused unresolved symbols for TooN
LOCAL_CFLAGS    := -O3 -fPIC -DNO_BLAS_WRAP -Wno-logical-op-parentheses
include $(BUILD_STATIC_LIBRARY)

LOCAL_CFLAGS_COMMON := -O2 -Wall -Wno-unused-function -Wfatal-errors -Wno-tautological-compare -Wno-pass-failed -fPIC -std=c99 -D_POSIX_C_SOURCE=200112L -I../3rd-party/blis/include/arm64 -I../3rd-party/blis/frame/include -DBLIS_IS_BUILDING_LIBRARY -fvisibility=hidden
LOCAL_CFLAGS_REF_KERNELS := $(LOCAL_CFLAGS_COMMON) -O3 -funsafe-math-optimizations -ffp-contract=fast -DBLIS_IN_REF_KERNEL=1

LOCAL_SRC_FILES_REF_KERNELS := $(shell find -L ../3rd-party/blis/ref_kernels ! -path "*old*" ! -path *other* -name "*.c")

include $(CLEAR_VARS)
LOCAL_MODULE    := ref_kernels_firestorm
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES_REF_KERNELS)
LOCAL_CFLAGS    := $(LOCAL_CFLAGS_REF_KERNELS) -march=armv8-a -ftree-vectorize -D_GNU_SOURCE -DBLIS_CNAME=firestorm
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ref_kernels_thunderx2
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES_REF_KERNELS)
LOCAL_CFLAGS    := $(LOCAL_CFLAGS_REF_KERNELS) -mcpu=thunderx2t99 -ftree-vectorize -D_GNU_SOURCE -DBLIS_CNAME=thunderx2
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ref_kernels_cortexa57
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES_REF_KERNELS)
LOCAL_CFLAGS    := $(LOCAL_CFLAGS_REF_KERNELS) -mcpu=cortex-a57 -ftree-vectorize -D_GNU_SOURCE -DBLIS_CNAME=cortexa57
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ref_kernels_cortexa53
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES_REF_KERNELS)
LOCAL_CFLAGS := $(LOCAL_CFLAGS_REF_KERNELS) -mcpu=cortex-a53 -ftree-vectorize -D_GNU_SOURCE -DBLIS_CNAME=cortexa53
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ref_kernels_generic
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES_REF_KERNELS)
LOCAL_CFLAGS    := $(LOCAL_CFLAGS_REF_KERNELS) -DBLIS_CNAME=generic
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := blis
#find -L ../3rd-party/blis/kernels/armv8a ! -path "*old*" -name "*.c"
#find -L ../3rd-party/blis/frame ! -path "*old*" ! -path "*other*" -name "*.c"
LOCAL_SRC_FILES := \
    ../3rd-party/blis/config/cortexa53/bli_cntx_init_cortexa53.c \
    ../3rd-party/blis/config/cortexa57/bli_cntx_init_cortexa57.c \
    ../3rd-party/blis/config/firestorm/bli_cntx_init_firestorm.c \
    ../3rd-party/blis/config/generic/bli_cntx_init_generic.c \
    ../3rd-party/blis/config/thunderx2/bli_cntx_init_thunderx2.c \
    ../3rd-party/blis/kernels/armv8a/1m/bli_packm_armv8a_int_d6xk.c \
    ../3rd-party/blis/kernels/armv8a/1m/bli_packm_armv8a_int_d8xk.c \
    ../3rd-party/blis/kernels/armv8a/1m/bli_packm_armv8a_int_s12xk.c \
    ../3rd-party/blis/kernels/armv8a/1m/bli_packm_armv8a_int_s8xk.c \
    ../3rd-party/blis/kernels/armv8a/3/bli_gemm_armv8a_asm_d6x8.c \
    ../3rd-party/blis/kernels/armv8a/3/bli_gemm_armv8a_asm_d8x6r.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/bli_gemmsup_rd_armv8a_asm_d6x8m.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/bli_gemmsup_rd_armv8a_asm_d6x8n.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/bli_gemmsup_rv_armv8a_asm_d4x8n.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/bli_gemmsup_rv_armv8a_asm_d5x8n.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/bli_gemmsup_rv_armv8a_asm_d6x5m.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/bli_gemmsup_rv_armv8a_asm_d6x6m.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/bli_gemmsup_rv_armv8a_asm_d6x7m.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/bli_gemmsup_rv_armv8a_asm_d6x8m.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/bli_gemmsup_rv_armv8a_asm_d6x8n.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/bli_gemmsup_rv_armv8a_asm_d8x4m.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/d3x4/bli_gemmsup_rd_armv8a_asm_d3x4.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/d3x4/bli_gemmsup_rd_armv8a_asm_d6x3.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/d3x4/bli_gemmsup_rd_armv8a_int_d2x8.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/d3x4/bli_gemmsup_rd_armv8a_int_d3x4.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/d6x4/bli_gemmsup_rv_armv8a_int_d3x8mn.c \
    ../3rd-party/blis/kernels/armv8a/3/sup/d6x4/bli_gemmsup_rv_armv8a_int_d6x4mn.c \
    ../3rd-party/blis/frame/0/bli_l0_check.c \
    ../3rd-party/blis/frame/0/bli_l0_fpa.c \
    ../3rd-party/blis/frame/0/bli_l0_oapi.c \
    ../3rd-party/blis/frame/0/bli_l0_tapi.c \
    ../3rd-party/blis/frame/0/copysc/bli_copysc.c \
    ../3rd-party/blis/frame/1/bli_l1v_check.c \
    ../3rd-party/blis/frame/1/bli_l1v_fpa.c \
    ../3rd-party/blis/frame/1/bli_l1v_oapi.c \
    ../3rd-party/blis/frame/1/bli_l1v_oapi_ba.c \
    ../3rd-party/blis/frame/1/bli_l1v_oapi_ex.c \
    ../3rd-party/blis/frame/1/bli_l1v_tapi.c \
    ../3rd-party/blis/frame/1/bli_l1v_tapi_ba.c \
    ../3rd-party/blis/frame/1/bli_l1v_tapi_ex.c \
    ../3rd-party/blis/frame/1d/bli_l1d_check.c \
    ../3rd-party/blis/frame/1d/bli_l1d_fpa.c \
    ../3rd-party/blis/frame/1d/bli_l1d_oapi.c \
    ../3rd-party/blis/frame/1d/bli_l1d_oapi_ba.c \
    ../3rd-party/blis/frame/1d/bli_l1d_oapi_ex.c \
    ../3rd-party/blis/frame/1d/bli_l1d_tapi.c \
    ../3rd-party/blis/frame/1d/bli_l1d_tapi_ba.c \
    ../3rd-party/blis/frame/1d/bli_l1d_tapi_ex.c \
    ../3rd-party/blis/frame/1f/bli_l1f_check.c \
    ../3rd-party/blis/frame/1f/bli_l1f_fpa.c \
    ../3rd-party/blis/frame/1f/bli_l1f_oapi.c \
    ../3rd-party/blis/frame/1f/bli_l1f_oapi_ba.c \
    ../3rd-party/blis/frame/1f/bli_l1f_oapi_ex.c \
    ../3rd-party/blis/frame/1f/bli_l1f_tapi.c \
    ../3rd-party/blis/frame/1f/bli_l1f_tapi_ba.c \
    ../3rd-party/blis/frame/1f/bli_l1f_tapi_ex.c \
    ../3rd-party/blis/frame/1m/bli_l1m_check.c \
    ../3rd-party/blis/frame/1m/bli_l1m_fpa.c \
    ../3rd-party/blis/frame/1m/bli_l1m_oapi.c \
    ../3rd-party/blis/frame/1m/bli_l1m_oapi_ba.c \
    ../3rd-party/blis/frame/1m/bli_l1m_oapi_ex.c \
    ../3rd-party/blis/frame/1m/bli_l1m_tapi.c \
    ../3rd-party/blis/frame/1m/bli_l1m_tapi_ba.c \
    ../3rd-party/blis/frame/1m/bli_l1m_tapi_ex.c \
    ../3rd-party/blis/frame/1m/bli_l1m_unb_var1.c \
    ../3rd-party/blis/frame/1m/packm/bli_packm_alloc.c \
    ../3rd-party/blis/frame/1m/packm/bli_packm_blk_var1.c \
    ../3rd-party/blis/frame/1m/packm/bli_packm_check.c \
    ../3rd-party/blis/frame/1m/packm/bli_packm_cntl.c \
    ../3rd-party/blis/frame/1m/packm/bli_packm_init.c \
    ../3rd-party/blis/frame/1m/packm/bli_packm_int.c \
    ../3rd-party/blis/frame/1m/packm/bli_packm_part.c \
    ../3rd-party/blis/frame/1m/packm/bli_packm_scalar.c \
    ../3rd-party/blis/frame/1m/packm/bli_packm_struc_cxk.c \
    ../3rd-party/blis/frame/1m/packm/bli_packm_struc_cxk_md.c \
    ../3rd-party/blis/frame/1m/unpackm/bli_unpackm_blk_var1.c \
    ../3rd-party/blis/frame/1m/unpackm/bli_unpackm_check.c \
    ../3rd-party/blis/frame/1m/unpackm/bli_unpackm_cntl.c \
    ../3rd-party/blis/frame/1m/unpackm/bli_unpackm_int.c \
    ../3rd-party/blis/frame/2/bli_l2_check.c \
    ../3rd-party/blis/frame/2/bli_l2_fpa.c \
    ../3rd-party/blis/frame/2/bli_l2_oapi.c \
    ../3rd-party/blis/frame/2/bli_l2_oapi_ba.c \
    ../3rd-party/blis/frame/2/bli_l2_oapi_ex.c \
    ../3rd-party/blis/frame/2/bli_l2_tapi.c \
    ../3rd-party/blis/frame/2/bli_l2_tapi_ba.c \
    ../3rd-party/blis/frame/2/bli_l2_tapi_ex.c \
    ../3rd-party/blis/frame/2/gemv/bli_gemv_unb_var1.c \
    ../3rd-party/blis/frame/2/gemv/bli_gemv_unb_var2.c \
    ../3rd-party/blis/frame/2/gemv/bli_gemv_unf_var1.c \
    ../3rd-party/blis/frame/2/gemv/bli_gemv_unf_var2.c \
    ../3rd-party/blis/frame/2/gemv/bli_gemv_var_oapi.c \
    ../3rd-party/blis/frame/2/ger/bli_ger_unb_var1.c \
    ../3rd-party/blis/frame/2/ger/bli_ger_unb_var2.c \
    ../3rd-party/blis/frame/2/ger/bli_ger_var_oapi.c \
    ../3rd-party/blis/frame/2/hemv/bli_hemv_unb_var1.c \
    ../3rd-party/blis/frame/2/hemv/bli_hemv_unb_var2.c \
    ../3rd-party/blis/frame/2/hemv/bli_hemv_unb_var3.c \
    ../3rd-party/blis/frame/2/hemv/bli_hemv_unb_var4.c \
    ../3rd-party/blis/frame/2/hemv/bli_hemv_unf_var1.c \
    ../3rd-party/blis/frame/2/hemv/bli_hemv_unf_var1a.c \
    ../3rd-party/blis/frame/2/hemv/bli_hemv_unf_var3.c \
    ../3rd-party/blis/frame/2/hemv/bli_hemv_unf_var3a.c \
    ../3rd-party/blis/frame/2/hemv/bli_hemv_var_oapi.c \
    ../3rd-party/blis/frame/2/her/bli_her_unb_var1.c \
    ../3rd-party/blis/frame/2/her/bli_her_unb_var2.c \
    ../3rd-party/blis/frame/2/her/bli_her_var_oapi.c \
    ../3rd-party/blis/frame/2/her2/bli_her2_unb_var1.c \
    ../3rd-party/blis/frame/2/her2/bli_her2_unb_var2.c \
    ../3rd-party/blis/frame/2/her2/bli_her2_unb_var3.c \
    ../3rd-party/blis/frame/2/her2/bli_her2_unb_var4.c \
    ../3rd-party/blis/frame/2/her2/bli_her2_unf_var1.c \
    ../3rd-party/blis/frame/2/her2/bli_her2_unf_var4.c \
    ../3rd-party/blis/frame/2/her2/bli_her2_var_oapi.c \
    ../3rd-party/blis/frame/2/trmv/bli_trmv_unb_var1.c \
    ../3rd-party/blis/frame/2/trmv/bli_trmv_unb_var2.c \
    ../3rd-party/blis/frame/2/trmv/bli_trmv_unf_var1.c \
    ../3rd-party/blis/frame/2/trmv/bli_trmv_unf_var2.c \
    ../3rd-party/blis/frame/2/trmv/bli_trmv_var_oapi.c \
    ../3rd-party/blis/frame/2/trsv/bli_trsv_unb_var1.c \
    ../3rd-party/blis/frame/2/trsv/bli_trsv_unb_var2.c \
    ../3rd-party/blis/frame/2/trsv/bli_trsv_unf_var1.c \
    ../3rd-party/blis/frame/2/trsv/bli_trsv_unf_var2.c \
    ../3rd-party/blis/frame/2/trsv/bli_trsv_var_oapi.c \
    ../3rd-party/blis/frame/3/bli_l3_blocksize.c \
    ../3rd-party/blis/frame/3/bli_l3_check.c \
    ../3rd-party/blis/frame/3/bli_l3_cntl.c \
    ../3rd-party/blis/frame/3/bli_l3_decor.c \
    ../3rd-party/blis/frame/3/bli_l3_direct.c \
    ../3rd-party/blis/frame/3/bli_l3_ind.c \
    ../3rd-party/blis/frame/3/bli_l3_int.c \
    ../3rd-party/blis/frame/3/bli_l3_oapi.c \
    ../3rd-party/blis/frame/3/bli_l3_oapi_ex.c \
    ../3rd-party/blis/frame/3/bli_l3_packab.c \
    ../3rd-party/blis/frame/3/bli_l3_prune.c \
    ../3rd-party/blis/frame/3/bli_l3_schema.c \
    ../3rd-party/blis/frame/3/bli_l3_sup.c \
    ../3rd-party/blis/frame/3/bli_l3_sup_decor.c \
    ../3rd-party/blis/frame/3/bli_l3_sup_int.c \
    ../3rd-party/blis/frame/3/bli_l3_sup_packm.c \
    ../3rd-party/blis/frame/3/bli_l3_sup_packm_var.c \
    ../3rd-party/blis/frame/3/bli_l3_sup_ref.c \
    ../3rd-party/blis/frame/3/bli_l3_sup_var12.c \
    ../3rd-party/blis/frame/3/bli_l3_sup_var1n2m.c \
    ../3rd-party/blis/frame/3/bli_l3_tapi.c \
    ../3rd-party/blis/frame/3/bli_l3_tapi_ex.c \
    ../3rd-party/blis/frame/3/bli_l3_thrinfo.c \
    ../3rd-party/blis/frame/3/bli_l3_ukr_fpa.c \
    ../3rd-party/blis/frame/3/bli_l3_ukr_oapi.c \
    ../3rd-party/blis/frame/3/bli_l3_ukr_tapi.c \
    ../3rd-party/blis/frame/3/gemm/bli_gemm_blk_var1.c \
    ../3rd-party/blis/frame/3/gemm/bli_gemm_blk_var2.c \
    ../3rd-party/blis/frame/3/gemm/bli_gemm_blk_var3.c \
    ../3rd-party/blis/frame/3/gemm/bli_gemm_cntl.c \
    ../3rd-party/blis/frame/3/gemm/bli_gemm_front.c \
    ../3rd-party/blis/frame/3/gemm/bli_gemm_ker_var2.c \
    ../3rd-party/blis/frame/3/gemm/bli_gemm_md.c \
    ../3rd-party/blis/frame/3/gemm/bli_gemm_md_c2r_ref.c \
    ../3rd-party/blis/frame/3/gemmt/bli_gemmt_front.c \
    ../3rd-party/blis/frame/3/gemmt/bli_gemmt_l_ker_var2.c \
    ../3rd-party/blis/frame/3/gemmt/bli_gemmt_l_ker_var2b.c \
    ../3rd-party/blis/frame/3/gemmt/bli_gemmt_u_ker_var2.c \
    ../3rd-party/blis/frame/3/gemmt/bli_gemmt_u_ker_var2b.c \
    ../3rd-party/blis/frame/3/gemmt/bli_gemmt_x_ker_var2.c \
    ../3rd-party/blis/frame/3/gemmt/bli_gemmt_x_ker_var2b.c \
    ../3rd-party/blis/frame/3/hemm/bli_hemm_front.c \
    ../3rd-party/blis/frame/3/symm/bli_symm_front.c \
    ../3rd-party/blis/frame/3/trmm/bli_trmm_front.c \
    ../3rd-party/blis/frame/3/trmm/bli_trmm_ll_ker_var2.c \
    ../3rd-party/blis/frame/3/trmm/bli_trmm_ll_ker_var2b.c \
    ../3rd-party/blis/frame/3/trmm/bli_trmm_lu_ker_var2.c \
    ../3rd-party/blis/frame/3/trmm/bli_trmm_lu_ker_var2b.c \
    ../3rd-party/blis/frame/3/trmm/bli_trmm_rl_ker_var2.c \
    ../3rd-party/blis/frame/3/trmm/bli_trmm_rl_ker_var2b.c \
    ../3rd-party/blis/frame/3/trmm/bli_trmm_ru_ker_var2.c \
    ../3rd-party/blis/frame/3/trmm/bli_trmm_ru_ker_var2b.c \
    ../3rd-party/blis/frame/3/trmm/bli_trmm_xx_ker_var2.c \
    ../3rd-party/blis/frame/3/trmm/bli_trmm_xx_ker_var2b.c \
    ../3rd-party/blis/frame/3/trmm3/bli_trmm3_front.c \
    ../3rd-party/blis/frame/3/trsm/bli_trsm_blk_var1.c \
    ../3rd-party/blis/frame/3/trsm/bli_trsm_blk_var2.c \
    ../3rd-party/blis/frame/3/trsm/bli_trsm_blk_var3.c \
    ../3rd-party/blis/frame/3/trsm/bli_trsm_cntl.c \
    ../3rd-party/blis/frame/3/trsm/bli_trsm_front.c \
    ../3rd-party/blis/frame/3/trsm/bli_trsm_ll_ker_var2.c \
    ../3rd-party/blis/frame/3/trsm/bli_trsm_lu_ker_var2.c \
    ../3rd-party/blis/frame/3/trsm/bli_trsm_rl_ker_var2.c \
    ../3rd-party/blis/frame/3/trsm/bli_trsm_ru_ker_var2.c \
    ../3rd-party/blis/frame/3/trsm/bli_trsm_xx_ker_var2.c \
    ../3rd-party/blis/frame/base/bli_apool.c \
    ../3rd-party/blis/frame/base/bli_arch.c \
    ../3rd-party/blis/frame/base/bli_array.c \
    ../3rd-party/blis/frame/base/bli_blksz.c \
    ../3rd-party/blis/frame/base/bli_check.c \
    ../3rd-party/blis/frame/base/bli_clock.c \
    ../3rd-party/blis/frame/base/bli_cntl.c \
    ../3rd-party/blis/frame/base/bli_cntx.c \
    ../3rd-party/blis/frame/base/bli_const.c \
    ../3rd-party/blis/frame/base/bli_cpuid.c \
    ../3rd-party/blis/frame/base/bli_env.c \
    ../3rd-party/blis/frame/base/bli_error.c \
    ../3rd-party/blis/frame/base/bli_func.c \
    ../3rd-party/blis/frame/base/bli_getopt.c \
    ../3rd-party/blis/frame/base/bli_gks.c \
    ../3rd-party/blis/frame/base/bli_ind.c \
    ../3rd-party/blis/frame/base/bli_info.c \
    ../3rd-party/blis/frame/base/bli_init.c \
    ../3rd-party/blis/frame/base/bli_machval.c \
    ../3rd-party/blis/frame/base/bli_malloc.c \
    ../3rd-party/blis/frame/base/bli_mbool.c \
    ../3rd-party/blis/frame/base/bli_memsys.c \
    ../3rd-party/blis/frame/base/bli_obj.c \
    ../3rd-party/blis/frame/base/bli_obj_scalar.c \
    ../3rd-party/blis/frame/base/bli_pack.c \
    ../3rd-party/blis/frame/base/bli_param_map.c \
    ../3rd-party/blis/frame/base/bli_part.c \
    ../3rd-party/blis/frame/base/bli_pba.c \
    ../3rd-party/blis/frame/base/bli_pool.c \
    ../3rd-party/blis/frame/base/bli_prune.c \
    ../3rd-party/blis/frame/base/bli_query.c \
    ../3rd-party/blis/frame/base/bli_rntm.c \
    ../3rd-party/blis/frame/base/bli_sba.c \
    ../3rd-party/blis/frame/base/bli_setgetijm.c \
    ../3rd-party/blis/frame/base/bli_setgetijv.c \
    ../3rd-party/blis/frame/base/bli_setri.c \
    ../3rd-party/blis/frame/base/bli_string.c \
    ../3rd-party/blis/frame/base/bli_winsys.c \
    ../3rd-party/blis/frame/base/cast/bli_castm.c \
    ../3rd-party/blis/frame/base/cast/bli_castnzm.c \
    ../3rd-party/blis/frame/base/cast/bli_castv.c \
    ../3rd-party/blis/frame/base/check/bli_obj_check.c \
    ../3rd-party/blis/frame/base/check/bli_part_check.c \
    ../3rd-party/blis/frame/base/noopt/bli_dlamch.c \
    ../3rd-party/blis/frame/base/noopt/bli_lsame.c \
    ../3rd-party/blis/frame/base/noopt/bli_slamch.c \
    ../3rd-party/blis/frame/base/proj/bli_projm.c \
    ../3rd-party/blis/frame/base/proj/bli_projv.c \
    ../3rd-party/blis/frame/compat/bla_amax.c \
    ../3rd-party/blis/frame/compat/bla_asum.c \
    ../3rd-party/blis/frame/compat/bla_axpy.c \
    ../3rd-party/blis/frame/compat/bla_copy.c \
    ../3rd-party/blis/frame/compat/bla_dot.c \
    ../3rd-party/blis/frame/compat/bla_gemm.c \
    ../3rd-party/blis/frame/compat/bla_gemv.c \
    ../3rd-party/blis/frame/compat/bla_ger.c \
    ../3rd-party/blis/frame/compat/bla_hemm.c \
    ../3rd-party/blis/frame/compat/bla_hemv.c \
    ../3rd-party/blis/frame/compat/bla_her.c \
    ../3rd-party/blis/frame/compat/bla_her2.c \
    ../3rd-party/blis/frame/compat/bla_her2k.c \
    ../3rd-party/blis/frame/compat/bla_herk.c \
    ../3rd-party/blis/frame/compat/bla_nrm2.c \
    ../3rd-party/blis/frame/compat/bla_scal.c \
    ../3rd-party/blis/frame/compat/bla_swap.c \
    ../3rd-party/blis/frame/compat/bla_symm.c \
    ../3rd-party/blis/frame/compat/bla_symv.c \
    ../3rd-party/blis/frame/compat/bla_syr.c \
    ../3rd-party/blis/frame/compat/bla_syr2.c \
    ../3rd-party/blis/frame/compat/bla_syr2k.c \
    ../3rd-party/blis/frame/compat/bla_syrk.c \
    ../3rd-party/blis/frame/compat/bla_trmm.c \
    ../3rd-party/blis/frame/compat/bla_trmv.c \
    ../3rd-party/blis/frame/compat/bla_trsm.c \
    ../3rd-party/blis/frame/compat/bla_trsv.c \
    ../3rd-party/blis/frame/compat/blis/thread/b77_thread.c \
    ../3rd-party/blis/frame/compat/extra/bla_axpby.c \
    ../3rd-party/blis/frame/compat/extra/bla_gemm3m.c \
    ../3rd-party/blis/frame/compat/extra/bla_gemm_batch.c \
    ../3rd-party/blis/frame/compat/extra/bla_gemmt.c \
    ../3rd-party/blis/frame/compat/f2c/bla_cabs1.c \
    ../3rd-party/blis/frame/compat/f2c/bla_gbmv.c \
    ../3rd-party/blis/frame/compat/f2c/bla_hbmv.c \
    ../3rd-party/blis/frame/compat/f2c/bla_hpmv.c \
    ../3rd-party/blis/frame/compat/f2c/bla_hpr.c \
    ../3rd-party/blis/frame/compat/f2c/bla_hpr2.c \
    ../3rd-party/blis/frame/compat/f2c/bla_lsame.c \
    ../3rd-party/blis/frame/compat/f2c/bla_rot.c \
    ../3rd-party/blis/frame/compat/f2c/bla_rotg.c \
    ../3rd-party/blis/frame/compat/f2c/bla_rotm.c \
    ../3rd-party/blis/frame/compat/f2c/bla_rotmg.c \
    ../3rd-party/blis/frame/compat/f2c/bla_sbmv.c \
    ../3rd-party/blis/frame/compat/f2c/bla_spmv.c \
    ../3rd-party/blis/frame/compat/f2c/bla_spr.c \
    ../3rd-party/blis/frame/compat/f2c/bla_spr2.c \
    ../3rd-party/blis/frame/compat/f2c/bla_tbmv.c \
    ../3rd-party/blis/frame/compat/f2c/bla_tbsv.c \
    ../3rd-party/blis/frame/compat/f2c/bla_tpmv.c \
    ../3rd-party/blis/frame/compat/f2c/bla_tpsv.c \
    ../3rd-party/blis/frame/compat/f2c/bla_xerbla.c \
    ../3rd-party/blis/frame/compat/f2c/bla_xerbla_array.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_c_abs.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_c_div.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_d_abs.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_d_cnjg.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_d_imag.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_d_sign.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_f__cabs.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_r_abs.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_r_cnjg.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_r_imag.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_r_sign.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_z_abs.c \
    ../3rd-party/blis/frame/compat/f2c/util/bla_z_div.c \
    ../3rd-party/blis/frame/thread/bli_pthread.c \
    ../3rd-party/blis/frame/thread/bli_thrcomm.c \
    ../3rd-party/blis/frame/thread/bli_thrcomm_openmp.c \
    ../3rd-party/blis/frame/thread/bli_thrcomm_pthreads.c \
    ../3rd-party/blis/frame/thread/bli_thrcomm_single.c \
    ../3rd-party/blis/frame/thread/bli_thread.c \
    ../3rd-party/blis/frame/thread/bli_thread_openmp.c \
    ../3rd-party/blis/frame/thread/bli_thread_pthreads.c \
    ../3rd-party/blis/frame/thread/bli_thread_range.c \
    ../3rd-party/blis/frame/thread/bli_thread_range_slab_rr.c \
    ../3rd-party/blis/frame/thread/bli_thread_range_tlb.c \
    ../3rd-party/blis/frame/thread/bli_thread_single.c \
    ../3rd-party/blis/frame/thread/bli_thrinfo.c \
    ../3rd-party/blis/frame/util/bli_util_check.c \
    ../3rd-party/blis/frame/util/bli_util_fpa.c \
    ../3rd-party/blis/frame/util/bli_util_oapi.c \
    ../3rd-party/blis/frame/util/bli_util_oapi_ba.c \
    ../3rd-party/blis/frame/util/bli_util_oapi_ex.c \
    ../3rd-party/blis/frame/util/bli_util_tapi.c \
    ../3rd-party/blis/frame/util/bli_util_tapi_ba.c \
    ../3rd-party/blis/frame/util/bli_util_tapi_ex.c \
    ../3rd-party/blis/frame/util/bli_util_unb_var1.c \

LOCAL_SRC_FILES += \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sger.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dger.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sdot.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dgemm.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_ddot.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_saxpy.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sscal.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_daxpy.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dscal.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_scopy.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_ssyrk.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_ssymm.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dcopy.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dsyrk.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dsymm.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sgemv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dgemv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sspmv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_stpmv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sgbmv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_stpsv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dspmv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dtpmv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dgbmv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dtpsv.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sspr2.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dspr2.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_srot.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_drot.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_sspr.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_dspr.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_globals.c \
    $(CBLAS_RELATIVE_PATH)/src/cblas_xerbla.c

LOCAL_C_INCLUDES := ../3rd-party/$(TARGET_ARCH_ABI)/blis-new
LOCAL_CFLAGS := $(LOCAL_CFLAGS_COMMON) -D_GNU_SOURCE -DBLIS_ENABLE_CBLAS

#LOCAL_C_INCLUDES := \
    ../3rd-party/$(TARGET_ARCH_ABI)/include/blis \
    ../3rd-party/blis \
    ../3rd-party/blis/frame/thread \
    ../3rd-party/blis/frame/include

LOCAL_STATIC_LIBRARIES := ref_kernels_firestorm ref_kernels_thunderx2 ref_kernels_cortexa57 ref_kernels_cortexa53 ref_kernels_generic blas
LOCAL_LDLIBS := -lm -ldl

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
