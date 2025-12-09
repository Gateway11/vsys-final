/*=============================================================================
*
* Project: a2bstack
*
* Copyright (c) 2025 - Analog Devices Inc. All Rights Reserved.
* This software is subject to the terms and conditions of the license set
* forth in the project LICENSE file. Downloading, reproducing, distributing or
* otherwise using the software constitutes acceptance of the license. The
* software may not be used except as expressly authorized under the license.
*
*=============================================================================
*
* \file:   spi.c
* \author: Automotive Software Team, Bangalore
* \brief:  Implements the stack SPI API for plugins and A2B applications.
*
*=============================================================================
*/

/*============================================================================*/
/**
* \ingroup  a2bstack_spi
* \defgroup a2bstack_spi_priv          \<Private\>
* \private
*
* This defines SPI API's that are private to the stack.
*/
/*============================================================================*/

/*======================= I N C L U D E S =========================*/

#include "a2b/pal.h"
#include "a2b/util.h"
#include "a2b/trace.h"
#include "a2b/error.h"
#include "a2b/regdefs.h"
#include "a2b/defs.h"
#include "a2b/msgrtr.h"
#include "a2b/stringbuffer.h"
#include "a2b/seqchart.h"
#include "a2b/util.h"
#include "stack_priv.h"
#include "spi_priv.h"
#include "stackctx.h"
#include "utilmacros.h"
#include "a2b/cpeng.h"


/*======================= D E F I N E S ===========================*/
#define A2B_SPI_TEMP_BUF_SIZE   (64u)
#define A2B_SPI_DATA_ARGS       (8u)


/*======================= L O C A L  P R O T O T Y P E S  =========*/

#if defined(A2B_FEATURE_TRACE) || defined(A2B_FEATURE_SEQ_CHART)
static a2b_Char* a2b_spiFormatString(a2b_Char* buf, a2b_UInt32  bufLen, const a2b_Char* fmt, void**  args, a2b_UInt16  numArgs, const a2b_Byte* data, a2b_UInt16 nBytes);
#endif  /* A2B_FEATURE_TRACE || A2B_FEATURE_SEQ_CHART */

static a2b_HResult isSpiLengthValid					(a2b_UInt16 length, a2b_UInt16 min, a2b_UInt16 max);
static a2b_HResult a2b_spiWrite						(a2b_StackContext* ctx, a2b_UInt16 ss, a2b_UInt16 nWrite, const a2b_Byte* wBuf);
static a2b_HResult a2b_spiWriteRead					(a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf);
static a2b_HResult a2b_spiFd						(a2b_StackContext* ctx, a2b_UInt16 spiSS, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf);
static a2b_HResult a2b_spiLocalRegWrite				(a2b_StackContext* ctx, a2b_UInt16 spiSs,  a2b_UInt16 nWrite, a2b_Byte *wbuf);
static a2b_HResult a2b_spiLocalRegWriteRead			(a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf);
static a2b_HResult a2b_spiBusRegWrite				(a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_Int16 nodeaddr, a2b_UInt16 nWrite, a2b_Byte *wbuf);
static a2b_HResult a2b_spiBusRegWriteRead			(a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_Int16 nodeaddr, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf);
static a2b_HResult a2b_spiExecuteAccess				(a2b_StackContext* ctx, a2b_SpiCmd cmd, a2b_Int16  nodeAddr, a2b_UInt16 chipAddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, void* wBuf, a2b_UInt16 nRead, void* rBuf);
static a2b_HResult a2b_spiRemoteI2cWrite			(a2b_StackContext* ctx, a2b_UInt16 spiSs,  a2b_Int16 nodeaddr, a2b_UInt16 nWrite, a2b_Byte *wbuf);
static a2b_HResult a2b_spiRemoteI2cWriteRead		(a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_Int16 nodeaddr, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf);
static a2b_HResult a2b_spiDtAtomicLargeWrite		(a2b_StackContext* ctx, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf);
static a2b_HResult a2b_spiDtAtomicLargeWriteNb		(a2b_StackContext* ctx, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf);
static a2b_HResult a2b_spiDtAtomicLargeWriteRead	(a2b_StackContext* ctx, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf);
static a2b_HResult a2b_spiDtAtomicLargeWriteReadNb	(a2b_StackContext* ctx, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf);
static a2b_HResult a2b_spiDtBulkWrite				(a2b_StackContext* ctx, a2b_SpiCmd cmd, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf);
static a2b_HResult a2b_spiDtBulkWriteNb				(a2b_StackContext* ctx, a2b_SpiCmd cmd, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf);
static a2b_HResult a2b_spiGenericWrite				(a2b_StackContext* ctx, a2b_Int16 node, a2b_UInt16 spiSs, a2b_UInt16 nWrite, void* wBuf);
static a2b_HResult a2b_spiGenericWriteRead			(a2b_StackContext* ctx, a2b_Int16 node, a2b_UInt16 spiSs, a2b_UInt16 nWrite, void* wBuf, a2b_UInt16 nRead, void* rBuf);
static a2b_HResult a2b_spiDtFullDuplex(a2b_StackContext* ctx,  a2b_SpiCmd cmd, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_Bool bFdCmdBased, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf);
static a2b_HResult a2b_spiDtFullDuplexNb(a2b_StackContext* ctx,  a2b_SpiCmd cmd, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_Bool bFdCmdBased, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf);
static a2b_HResult a2b_spiPrepAccess(a2b_StackContext* ctx, a2b_SpiCmd cmd, a2b_Int16 nodeAddr, a2b_UInt16 chipAddr);
/*======================= D A T A  ================================*/


/*==================== C O D E =================================*/

#if defined(A2B_FEATURE_TRACE) || defined(A2B_FEATURE_SEQ_CHART)

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiFormatString
*
*  This is an internal function for formatting a string that optionally
*  includes the contents of a data buffer. This is used for formatting strings
*  for trace and sequence chart diagnostics and is only available when one of
*  the features are enabled.
*
*  \param   [in,out]    buf     The character buffer in which the formatted
*                               string will be rendered.
*
*  \param   [in]        bufLen  The size of 'buf' in characters.
*
*  \param   [in]        fmt     The sprintf-like format string. The format
*                               string only supports a subset of options
*                               described by the <em>#a2b_vsnprintf</em>
*                               function.
*
*  \param   [in]        args    An array of void* entries pointing at the
*                               arguments for the format string.
*
*  \param   [in]        numArgs The number of arguments in the <em>args</em>
*
*  \param   [in]        data    An array of bytes to be formatted as hex
*                               digits at the end of the formatted string.
*                               At most #A2B_SPI_DATA_ARGS will be formatted.
*                               May be A2B_NULL if no data is available.
*
*  \param   [in]        nBytes  The number of data bytes in the <em>data</em>
*                               array.
*
*  \pre     Only available when #A2B_FEATURE_SEQ_CHART or #A2B_FEATURE_TRACE
*           is enabled.
*
*  \post    The <em>buf</em> hold the A2B_NULL terminated formatted string.
*
*  \return  A pointer to the <em>buf</em> that was passed in.
*
******************************************************************************/
static a2b_Char*
a2b_spiFormatString(a2b_Char* buf, a2b_UInt32 bufLen, const a2b_Char* fmt, void** args, a2b_UInt16 numArgs, const a2b_Byte* data, a2b_UInt16 nBytes)
{
	a2b_StringBuffer    sb;
	a2b_UInt16          idx;
	a2b_UInt16          nDataArgs;
	void*               argVec[1];

	a2b_stringBufferInit(&sb, buf, bufLen);

	(void)a2b_vsnprintfStringBuffer(&sb, fmt, args, numArgs);

	nDataArgs = A2B_MIN(nBytes, A2B_SPI_DATA_ARGS);
	for (idx = 0; idx < nDataArgs; ++idx)
	{
		argVec[0] = (void*)&data[idx];
		(void)a2b_vsnprintfStringBuffer(&sb, "%02bX", argVec, A2B_ARRAY_SIZE(argVec));
		if (idx + 1 < nDataArgs)
		{
			(void)a2b_vsnprintfStringBuffer(&sb, " ", A2B_NULL, 0);
		}
	}

	if (nBytes > nDataArgs)
	{
		(void)a2b_vsnprintfStringBuffer(&sb, " ...", A2B_NULL, 0);
	}

	return buf;

} /* a2b_spiFormatString */

#endif  /* A2B_FEATURE_TRACE || A2B_FEATURE_SEQ_CHART */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiWrite
*
*  Writes bytes to the SPI device. This is an synchronous call and will block
*  until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    ss	    Slave select.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wBuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiWrite(a2b_StackContext* ctx, a2b_UInt16 ss, a2b_UInt16 nWrite, const a2b_Byte* wBuf)
{
	a2b_HResult result = ctx->stk->pal.spiWrite(ctx->stk->spiHnd, ss, nWrite, wBuf);

#if defined(A2B_FEATURE_TRACE) || defined(A2B_FEATURE_SEQ_CHART)
	a2b_Char    buf[A2B_SPI_TEMP_BUF_SIZE];
	void*       args[3u];

#ifdef A2B_FEATURE_SEQ_CHART
	a2b_UInt32  callOrigin;
#endif  /* A2B_FEATURE_SEQ_CHART */

#ifdef A2B_FEATURE_TRACE
	a2b_UInt32  trcMask = A2B_TRC_DOM_SPI | A2B_TRC_LVL_TRACE2;
#endif

	args[0u] = &ss;
	args[1u] = &result;
	args[2u] = A2B_NULL;

#ifdef A2B_FEATURE_TRACE
	if (A2B_SUCCEEDED(result))
	{
		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf), "a2b_spiWrite[0x%02hX] -> ", args, 1, wBuf, nWrite);
	}
	else
	{
		trcMask |= A2B_TRC_LVL_ERROR;
		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf), "a2b_spiWrite[0x%02hX] Error: 0x%lX", args, 2, wBuf, nWrite);
	}
	A2B_TRACE1((ctx, trcMask, "%s", buf));
#endif  /* A2B_FEATURE_TRACE */

#ifdef A2B_FEATURE_SEQ_CHART
	args[0] = &ss;
	args[1] = &nWrite;
	args[2] = &result;
	callOrigin = (a2b_UInt32)((a2b_UInt32)(ctx->domain == A2B_DOMAIN_APP) ? A2B_SEQ_CHART_ENTITY_APP : A2B_NODE_ADDR_TO_CHART_PLUGIN_ENTITY(ctx->ccb.plugin.nodeSig.nodeAddr));

	(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf), "a2b_spiWrite(0x%02hX, %hu) -> ", args, 2, wBuf, nWrite);
	A2B_SEQ_CHART1((ctx, (a2b_SeqChartEntity)callOrigin, A2B_SEQ_CHART_ENTITY_PLATFORM, A2B_SEQ_CHART_COMM_REQUEST, A2B_SEQ_CHART_LEVEL_SPI, "%s", buf));

	if (A2B_SUCCEEDED(result))
	{
		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf), "a2b_spiWrite(0x%02hX, %hu) <- Success", args, 2, A2B_NULL, 0);
	}
	else
	{
		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf), "a2b_spiWrite(0x%02hX, %hu) <- Error: 0x%lX", args, 3, A2B_NULL, 0);
	}
	A2B_SEQ_CHART1((ctx, A2B_SEQ_CHART_ENTITY_PLATFORM, (a2b_SeqChartEntity)callOrigin, A2B_SEQ_CHART_COMM_REPLY, A2B_SEQ_CHART_LEVEL_SPI, "%s", buf));
#endif  /* A2B_FEATURE_SEQ_CHART */

#endif /* A2B_FEATURE_TRACE || A2B_FEATURE_SEQ_CHART */

	return result;
}	/* a2b_spiWrite */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiWriteRead
*
*  Writes and then reads bytes from the spi device *without* an spi stop
*  sequence separating the two operations. Instead a repeated spi start
*  sequence is used as the operation separator. This is an synchronous call and
*  will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write/read.
*
*  \param   [in]    spiSs    SPI slave select.
* 
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wBuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \param   [in]    nRead   The number of bytes to read from the device.
*
*  \param   [in]    rBuf    A buffer in which to write the results of the read.
*
*  \pre     None
*
*  \post    The read buffer holds the contents of the read on success.
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiWriteRead(a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
	a2b_HResult result = ctx->stk->pal.spiWriteRead(ctx->stk->spiHnd, spiSs, nWrite, wBuf, nRead, rBuf);

#if defined(A2B_FEATURE_SEQ_CHART) || defined(A2B_FEATURE_TRACE)
	a2b_Char    buf[A2B_SPI_TEMP_BUF_SIZE];
	void*       args[4u];

#if defined(A2B_FEATURE_TRACE)
	a2b_UInt32  trcMask = A2B_TRC_DOM_SPI | A2B_TRC_LVL_TRACE2;
#endif

#ifdef A2B_FEATURE_SEQ_CHART
	a2b_UInt32  callOrigin;
#endif  /* A2B_FEATURE_SEQ_CHART */

	args[0u] = &spiSs;
	args[1u] = &result;
	args[2u] = A2B_NULL;
	args[3u] = A2B_NULL;

#ifdef A2B_FEATURE_TRACE
	if (A2B_SUCCEEDED(result))
	{
		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf),
			"a2b_spiWriteRead[0x%02hX] -> ", args, 1, wBuf, nWrite);
		A2B_TRACE1((ctx, trcMask, "%s", buf));

		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf),
			"a2b_spiWriteRead[0x%02hX] <- ", args, 1, rBuf, nRead);
		A2B_TRACE1((ctx, trcMask, "%s", buf));
	}
	else
	{
		trcMask |= A2B_TRC_LVL_ERROR;
		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf),
			"a2b_spiWriteRead[0x%02hX] Error: 0x%lX",
			args, 2, A2B_NULL, 0);
		A2B_TRACE1((ctx, trcMask, "%s", buf));
	}
#endif /* A2B_FEATURE_TRACE */

#ifdef A2B_FEATURE_SEQ_CHART
	args[0] = &spiSs;
	args[1] = &nWrite;
	args[2] = &nRead;
	args[3] = &result;
	callOrigin = (a2b_UInt32)((a2b_UInt32)(ctx->domain == A2B_DOMAIN_APP) ?
		A2B_SEQ_CHART_ENTITY_APP :
		A2B_NODE_ADDR_TO_CHART_PLUGIN_ENTITY(
			ctx->ccb.plugin.nodeSig.nodeAddr));
	(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf),
		"a2b_spiWriteRead(0x%02hX, %hu, %hu) -> ",
		args, 3, wBuf, nWrite);
	A2B_SEQ_CHART1((ctx,
		(a2b_SeqChartEntity)callOrigin,
		A2B_SEQ_CHART_ENTITY_PLATFORM,
		A2B_SEQ_CHART_COMM_REQUEST,
		A2B_SEQ_CHART_LEVEL_SPI,
		"%s", buf));

	if (A2B_SUCCEEDED(result))
	{
		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf),
			"a2b_spiWriteRead(0x%02hX, %hu, %hu) <- ",
			args, 3, rBuf, nRead);
	}
	else
	{
		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf),
			"a2b_spiWriteRead(0x%02hX, %hu, %hu) <- Error: 0x%lX",
			args, 4, A2B_NULL, 0);
	}
	A2B_SEQ_CHART1((ctx,
		A2B_SEQ_CHART_ENTITY_PLATFORM,
		(a2b_SeqChartEntity)callOrigin,
		A2B_SEQ_CHART_COMM_REPLY,
		A2B_SEQ_CHART_LEVEL_SPI,
		"%s", buf));
#endif  /* A2B_FEATURE_SEQ_CHART */

#endif

	return result;

} /* a2b_spiWriteRead */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiFd
*
*  Writes and then reads bytes from the spi device via full duplex access
*  This is an synchronous call and will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write/read.
*
*  \param   [in]    spiSS   SPI Slave Select
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wBuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \param   [in]    nRead   The number of bytes to read from the device.
*
*  \param   [in]    rBuf    A buffer in which to write the results of the read.
*
*  \pre     None
*
*  \post    The read buffer holds the contents of the read on success.
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiFd(a2b_StackContext* ctx, a2b_UInt16 spiSS, a2b_UInt16 nWrite, const a2b_Byte* wBuf, a2b_UInt16 nRead, a2b_Byte* rBuf)
{
	a2b_HResult result = ctx->stk->pal.spiFd(ctx->stk->spiHnd, spiSS, nWrite, wBuf, nRead, rBuf);

#if defined(A2B_FEATURE_SEQ_CHART) || defined(A2B_FEATURE_TRACE)
	a2b_Char    buf[A2B_SPI_TEMP_BUF_SIZE];
	void*       args[4u];

#if defined(A2B_FEATURE_TRACE)
	a2b_UInt32  trcMask = A2B_TRC_DOM_SPI | A2B_TRC_LVL_TRACE2;
#endif

#ifdef A2B_FEATURE_SEQ_CHART
	a2b_UInt32  callOrigin;
#endif  /* A2B_FEATURE_SEQ_CHART */

	args[0u] = &spiSS;
	args[1u] = &result;
	args[2u] = A2B_NULL;
	args[3u] = A2B_NULL;

#ifdef A2B_FEATURE_TRACE
	if (A2B_SUCCEEDED(result))
	{
		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf),
			"a2b_spiFd[0x%02hX] -> ", args, 1, wBuf, nWrite);
		A2B_TRACE1((ctx, trcMask, "%s", buf));

		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf),
			"a2b_spiFd[0x%02hX] <- ", args, 1, rBuf, nRead);
		A2B_TRACE1((ctx, trcMask, "%s", buf));
	}
	else
	{
		trcMask |= A2B_TRC_LVL_ERROR;
		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf),
			"a2b_spiFd[0x%02hX] Error: 0x%lX",
			args, 2, A2B_NULL, 0);
		A2B_TRACE1((ctx, trcMask, "%s", buf));
	}
#endif /* A2B_FEATURE_TRACE */

#ifdef A2B_FEATURE_SEQ_CHART
	args[0] = &spiSS;
	args[1] = &nWrite;
	args[2] = &nRead;
	args[3] = &result;
	callOrigin = (a2b_UInt32)((a2b_UInt32)(ctx->domain == A2B_DOMAIN_APP) ?
		A2B_SEQ_CHART_ENTITY_APP :
		A2B_NODE_ADDR_TO_CHART_PLUGIN_ENTITY(
			ctx->ccb.plugin.nodeSig.nodeAddr));
	(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf),
		"a2b_spiFd(0x%02hX, %hu, %hu) -> ",
		args, 3, wBuf, nWrite);
	A2B_SEQ_CHART1((ctx,
		(a2b_SeqChartEntity)callOrigin,
		A2B_SEQ_CHART_ENTITY_PLATFORM,
		A2B_SEQ_CHART_COMM_REQUEST,
		A2B_SEQ_CHART_LEVEL_SPI,
		"%s", buf));

	if (A2B_SUCCEEDED(result))
	{
		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf),
			"a2b_spiFd(0x%02hX, %hu, %hu) <- ",
			args, 3, rBuf, nRead);
	}
	else
	{
		(void)a2b_spiFormatString(buf, A2B_ARRAY_SIZE(buf),
			"a2b_spiFd(0x%02hX, %hu, %hu) <- Error: 0x%lX",
			args, 4, A2B_NULL, 0);
	}
	A2B_SEQ_CHART1((ctx,
		A2B_SEQ_CHART_ENTITY_PLATFORM,
		(a2b_SeqChartEntity)callOrigin,
		A2B_SEQ_CHART_COMM_REPLY,
		A2B_SEQ_CHART_LEVEL_SPI,
		"%s", buf));
#endif  /* A2B_FEATURE_SEQ_CHART */

#endif

	return result;

} /* a2b_spiFd */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiLocalRegWrite
*
*  The SPI local register write command is used to write local registers.
*  This is an synchronous call and will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    spiSs   SPI Slave Select
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiLocalRegWrite(a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_LOCAL_REG_WRITE, A2B_CMD_MAX_LENINBYTES_SPI_LOCAL_REG_WRITE);

	if (A2B_SUCCEEDED(result))
	{
		(void)a2b_spiCheckBusyStat(ctx, spiSs,  A2B_SPI_MAX_BUSY_CHECK_CNT);

		/* Formatting the A2B SPI protocol buffer based on the cmd */
		spiProcolBuf[0u] = (a2b_Byte)A2B_CMD_SPI_LOCAL_REG_WRITE;

		/* Copy the actual SPI data to A2B SPI protocol buffer */
		(void)a2b_memcpy(&spiProcolBuf[1], wbuf, nWrite);

		/* Call SPI write function */
		result = a2b_spiWrite(ctx, spiSs, nWrite + 1u, spiProcolBuf);
	}
	else
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "Failed SPI LocalRegWrite: %lu", &result));
	}

	return result;
}	/* a2b_spiLocalRegWrite */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiLocalRegWriteRead
*
*  The SPI local register write command is used to write local registers. 
*  This is an synchronous call and will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    spiSs   SPI Slave Select
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \param   [in]    nRead   The number of bytes to read from the device.
*
*  \param   [in]    rbuf    A buffer in which to write the results of the read.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiLocalRegWriteRead(a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	result = isSpiLengthValid(nRead, A2B_CMD_MIN_LENINBYTES_SPI_LOCAL_REG_READ, A2B_CMD_MAX_LENINBYTES_SPI_LOCAL_REG_READ);

	if ((A2B_SUCCEEDED(result)) && (nWrite == 1U))
	{
		(void)a2b_spiCheckBusyStat(ctx, spiSs,  A2B_SPI_MAX_BUSY_CHECK_CNT);

		/* Formatting the A2B SPI protocol buffer based on the cmd */
		spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_LOCAL_REG_READ;
	
		/* Copy the actual SPI data to A2B SPI protocol buffer */
		(void)a2b_memcpy(&spiProcolBuf[1u], wbuf, (a2b_Size)nWrite);

		spiProcolBuf[nWrite + 1u] = (a2b_UInt8)(nRead - 1u);

		/* Call SPI Write Read function */
		result = a2b_spiWriteRead(ctx, spiSs, nWrite + 2u, spiProcolBuf, nRead, rbuf);
	}
	else
	{
		result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_SPI_INVALID_LENGTH);
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "Failed SPI LocalRegWriteRead: %lu", &result));
	}

	return result;
}	/* a2b_spiLocalRegWriteRead */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiBusRegWrite
*
*  The SPI local register write command is used to write local registers.
*  This is an synchronous call and will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    spiSs   SPI Slave Select
*
*  \param   [in]    nodeaddr Address of the node for which the SPI transaction is intended for.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiBusRegWrite(a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_Int16 nodeaddr, a2b_UInt16 nWrite, a2b_Byte *wbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_SLAVE_REG_WRITE, A2B_CMD_MAX_LENINBYTES_SPI_SLAVE_REG_WRITE);

	if (A2B_SUCCEEDED(result))
	{
		/* Formatting the A2B SPI protocol buffer based on the cmd */
		spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_SLAVE_REG_WRITE;
		spiProcolBuf[1u] = (a2b_UInt8)nodeaddr;

		/* Copy the actual SPI data to A2B SPI protocol buffer */
		(void)a2b_memcpy(&spiProcolBuf[2], wbuf, (a2b_Size)nWrite);

		/* Call SPI write function */
		result = a2b_spiWrite(ctx, spiSs, nWrite + 2u, spiProcolBuf);

		/* Monitor the SPI busy status  */
		(void)a2b_spiCheckBusyStat(ctx, spiSs, A2B_SPI_MAX_BUSY_CHECK_CNT);
	}
	else
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "Failed SPI Bus RegWrite: %lu", &result));
	}

	return result;
}	/* a2b_spiBusRegWrite */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiBusRegWriteRead
*
*  The SPI local register write command is used to write local registers.
*  This is an synchronous call and will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    spiSs   SPI Slave Select
*
*  \param   [in]    nodeaddr Address of the node for which the SPI transaction is intended for.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \param   [in]    nRead   The number of bytes to read from the device.
*
*  \param   [in]    rbuf    A buffer in which to write the results of the read.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiBusRegWriteRead(a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_Int16 nodeaddr, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	result = isSpiLengthValid(nRead, A2B_CMD_MIN_LENINBYTES_SPI_SLAVE_REGISTER_READ_REQUEST, A2B_CMD_MAX_LENINBYTES_SPI_SLAVE_REGISTER_READ_REQUEST);

	if (A2B_SUCCEEDED(result) && (nWrite <= 1U))
	{
		/* Formatting the A2B SPI protocol buffer based on the cmd */
		spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_SLAVE_REG_READ_REQUEST | (a2b_UInt8)(nRead - 1u);
		spiProcolBuf[1u] = (a2b_UInt8)nodeaddr;

		/* Copy the actual SPI data to A2B SPI protocol buffer */
		(void)a2b_memcpy(&spiProcolBuf[2], wbuf, (a2b_Size)nWrite);

		/* Call SPI write function */
		result = a2b_spiWrite(ctx, spiSs, nWrite + 2u, spiProcolBuf);

		/* Monitor the SPI busy status  */
		(void)a2b_spiCheckBusyStat(ctx, spiSs,  A2B_SPI_MAX_BUSY_CHECK_CNT);

		spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_BUS_FIFO_READ;

		/* Call SPI Write Read function */
		result = a2b_spiWriteRead(ctx, spiSs, 1u, spiProcolBuf, nRead, rbuf);

		/* Monitor the SPI busy status  */
		(void)a2b_spiCheckBusyStat(ctx, spiSs, A2B_SPI_MAX_BUSY_CHECK_CNT);
	}
	else
	{
		result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_SPI_INVALID_LENGTH);
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "Failed SPI Bus RegWriteRead: %lu", &result));
	}

	return result;
}	/* a2b_spiBusRegWriteRead */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiRemoteI2cWrite
*
*  The SPI remote I2C write command is used to write to remote peripherals connected to slave nodes.
*  This is an synchronous call and will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    spiSs   SPI Slave Select
*
*  \param   [in]    nodeaddr Address of the slave node for which the SPI to I2C transaction is intended for.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiRemoteI2cWrite(a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_Int16 nodeaddr, a2b_UInt16 nWrite, a2b_Byte *wbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_REMOTE_I2C_WRITE, A2B_CMD_MAX_LENINBYTES_SPI_REMOTE_I2C_WRITE);

	if (A2B_SUCCEEDED(result))
	{
		/* Formatting the A2B SPI protocol buffer based on the cmd */
		spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_REMOTE_I2C_WRITE;
		spiProcolBuf[1u] = (a2b_UInt8)nodeaddr;

		/* Copy the actual SPI data to A2B SPI protocol buffer */
		(void)a2b_memcpy(&spiProcolBuf[2u], wbuf, (a2b_Size)nWrite);

		/* Call SPI write function */
		result = a2b_spiWrite(ctx, spiSs, nWrite + 2u, spiProcolBuf);

		/* Monitor the SPI busy status  */
		(void)a2b_spiCheckBusyStat(ctx, spiSs,  A2B_SPI_MAX_BUSY_CHECK_CNT);
	}
	else
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "Failed SPI to I2C Write: %lu", &result));
	}

	return result;
}	/* a2b_spiRemoteI2cWrite */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiBusRegWriteRead
*
*  The SPI remote I2C read command is used to read from remote peripherals connected to slave nodes.
*  This is an synchronous call and will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    spiSs   SPI Slave Select
*
*  \param   [in]    nodeaddr Address of the node for which the SPI transaction is intended for.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \param   [in]    nRead   The number of bytes to read from the device.
*
*  \param   [in]    rbuf    A buffer in which to write the results of the read.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiRemoteI2cWriteRead(a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_Int16 nodeaddr, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	result = isSpiLengthValid(nRead, A2B_CMD_MIN_LENINBYTES_SPI_REMOTE_I2C_READ_REQUEST, A2B_CMD_MAX_LENINBYTES_SPI_REMOTE_I2C_READ_REQUEST);

	if (A2B_SUCCEEDED(result))
	{
		/* Write the reg address before read */
		result = a2b_spiRemoteI2cWrite(ctx, spiSs, nodeaddr, nWrite, wbuf);

		/* Formatting the A2B SPI protocol buffer based on the cmd */
		spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST;
		spiProcolBuf[1u] = (a2b_UInt8)nodeaddr;
		spiProcolBuf[2u] = (a2b_UInt8)(nRead - 1u);

		/* Call SPI write function */
		result = a2b_spiWrite(ctx, spiSs, 3u, spiProcolBuf);

		/* Monitor the SPI busy status  */
		(void)a2b_spiCheckBusyStat(ctx, spiSs,  A2B_SPI_MAX_BUSY_CHECK_CNT);

		spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_BUS_FIFO_READ;

		/* Call SPI Write Read function */
		result = a2b_spiWriteRead(ctx, spiSs, 1u, spiProcolBuf, nRead, rbuf);

		/* Monitor the SPI busy status  */
		(void)a2b_spiCheckBusyStat(ctx, spiSs, A2B_SPI_MAX_BUSY_CHECK_CNT);
	}
	else
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "Failed SPI to I2C WriteRead: %lu", &result));
	}

	return result;
}

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiDtAtomicLargeWrite
*
*  The SPI data tunnel atomic write command is used to write to remote SPI peripherals connected to slave nodes.
*  This is an synchronous call and will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    nodeaddr Address of the slave node for which the SPI to I2C transaction is intended for.
*
*  \param   [in]    spiSs   SPI slave select. Only applicable for remote SPI peripheral access using SPI.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiDtAtomicLargeWrite(a2b_StackContext* ctx, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE, A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE);

	if (A2B_SUCCEEDED(result))
	{
		/* Formatting the A2B SPI protocol buffer based on the cmd */
		spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE;

		/* Prepares the SPI slave select/Node byte. Currently it is assumed that SPI target is always the slave node */
		spiProcolBuf[1u] = a2b_prepSpiSsNodeByte(nodeaddr, 0U, spiSs);

		/* Copy the actual SPI data to A2B SPI protocol buffer */
		(void)a2b_memcpy(&spiProcolBuf[2u], wbuf, (a2b_Size)nWrite);

		/* Call SPI write function */
		result = a2b_spiWrite(ctx, A2B_SPI_SLV_SEL, nWrite + A2B_CMD_LEN_ATOMIC_WRITE_MODE, spiProcolBuf);

		/* Monitor the SPI busy status  */
		(void)a2b_spiCheckBusyStat(ctx, A2B_SPI_SLV_SEL, A2B_SPI_MAX_BUSY_CHECK_CNT);
	}
	else
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "a2b_spiDtAtomicLargeWrite - Failed SPI to SPI Write: %lu", &result));
	}

	return result;
}	/* a2b_spiDtAtomicLargeWrite */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiDtAtomicLargeWriteNb
*
*  The SPI data tunnel atomic write command is used to write to remote SPI peripherals connected to slave nodes.
*  This is asynchronous call and will not block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    nodeaddr Address of the slave node for which the SPI to I2C transaction is intended for.
*
*  \param   [in]    spiSs   SPI slave select. Only applicable for remote SPI peripheral access using SPI.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiDtAtomicLargeWriteNb(a2b_StackContext* ctx, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE, A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE);

	if (A2B_SUCCEEDED(result))
	{
		/* Formatting the A2B SPI protocol buffer based on the cmd */
		spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE;

		/* Prepares the SPI slave select/Node byte. Currently it is assumed that SPI target is always the slave node */
		spiProcolBuf[1u] = a2b_prepSpiSsNodeByte(nodeaddr, 0U, spiSs);

		/* Copy the actual SPI data to A2B SPI protocol buffer */
		(void)a2b_memcpy(&spiProcolBuf[2u], wbuf, (a2b_Size)nWrite);

		/* Call SPI write function */
		result = a2b_spiWrite(ctx, 0u, nWrite + A2B_CMD_LEN_ATOMIC_WRITE_MODE, spiProcolBuf);

		/* Start a timer */
		a2b_remoteDevConfigSpiToSpiStartTimer(ctx);
	}
	else
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "a2b_spiDtAtomicLargeWriteNb - Failed SPI to SPI Write: %lu", &result));
	}

	return result;
}	/* a2b_spiDtAtomicLargeWriteNb */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiDtAtomicLargeWriteRead
*
*  The SPI data tunnel atomic read command is used to read from remote SPI peripherals connected to slave nodes.
*  This is an synchronous call and will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    nodeaddr Address of the node for which the SPI transaction is intended for.
*
*  \param   [in]    spiSs   SPI slave select. Only applicable for remote SPI peripheral access using SPI.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \param   [in]    nRead   The number of bytes to read from the device.
*
*  \param   [in]    rbuf    A buffer in which to write the results of the read.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiDtAtomicLargeWriteRead(a2b_StackContext* ctx, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST, A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST);

	if (A2B_SUCCEEDED(result))
	{
		/* Formatting the A2B SPI protocol buffer based on the cmd */
		spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST;

		/* Prepares the SPI slave select/Node byte. Currently it is assumed that SPI target is always the slave node */
		spiProcolBuf[1u] = a2b_prepSpiSsNodeByte(nodeaddr, 0U, spiSs);

		spiProcolBuf[2u] = (a2b_UInt8)(nRead - 1u);

		/* Copy the actual SPI data to A2B SPI protocol buffer */
		(void)a2b_memcpy(&spiProcolBuf[3], wbuf, (a2b_Size)nWrite);

		/* Call SPI write function */
		result = a2b_spiWrite(ctx, A2B_SPI_SLV_SEL, nWrite + A2B_CMD_LEN_ATOMIC_READ_MODE, spiProcolBuf);

		/* Monitor the SPI busy status  */
		(void)a2b_spiCheckBusyStat(ctx, A2B_SPI_SLV_SEL,  A2B_SPI_MAX_BUSY_CHECK_CNT);

		spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_DATA_TUNNEL_FIFO_READ;

		/* Call SPI Write Read function */
		result = a2b_spiWriteRead(ctx, A2B_SPI_SLV_SEL, 1u, spiProcolBuf, nRead, rbuf);

		/* Monitor the SPI busy status  */
		(void)a2b_spiCheckBusyStat(ctx, A2B_SPI_SLV_SEL, A2B_SPI_MAX_BUSY_CHECK_CNT);
	}
	else
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "a2b_spiDtAtomicLargeWriteRead - Failed SPI to SPI WriteRead: %lu", &result));
	}

	return result;
}	/* a2b_spiDtAtomicLargeWriteRead */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiDtAtomicLargeWriteReadNb
*
*  The SPI data tunnel atomic read command is used to read from remote SPI peripherals connected to slave nodes.
*  This is an asynchronous call and will not block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    nodeaddr Address of the node for which the SPI transaction is intended for.
*
*  \param   [in]    spiSs   SPI slave select. Only applicable for remote SPI peripheral access using SPI.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \param   [in]    nRead   The number of bytes to read from the device.
*
*  \param   [in]    rbuf    A buffer in which to write the results of the read.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiDtAtomicLargeWriteReadNb(a2b_StackContext* ctx, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST, A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST);

	if (A2B_SUCCEEDED(result))
	{
		/* Formatting the A2B SPI protocol buffer based on the cmd */
		spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST;

		/* Prepares the SPI slave select/Node byte. Currently it is assumed that SPI target is always the slave node */
		spiProcolBuf[1u] = a2b_prepSpiSsNodeByte(nodeaddr, 0U, spiSs);

		spiProcolBuf[2u] = (a2b_UInt8)(nRead - 1u);

		/* Copy the actual SPI data to A2B SPI protocol buffer */
		(void)a2b_memcpy(&spiProcolBuf[3], wbuf, (a2b_Size)nWrite);

		/* Call SPI write function */
		result = a2b_spiWrite(ctx, 0u, nWrite + A2B_CMD_LEN_ATOMIC_READ_MODE, spiProcolBuf);

		/* Set a flag if SPI Peripheral split transaction required. */
		ctx->stk->oSpiInfo.fSplitTransRequired = A2B_TRUE;

		/* Start a timer */
		a2b_remoteDevConfigSpiToSpiStartTimer(ctx);
	}
	else
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "a2b_spiDtAtomicLargeWriteReadNb - Failed SPI to SPI WriteRead: %lu", &result));
	}
	A2B_UNUSED(rbuf);
	return result;
}	/* a2b_spiDtAtomicLargeWriteReadNb */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiDtBulkWrite
*
*  The SPI bulk write command is used to write to remote SPI peripherals connected to slave nodes.
*  Bulk SPI transactions do not wait for all of the payload data to reach the remote node before starting the remote transaction.
*  This is an synchronous call and will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    cmd		 A2B SPI command added to the SPI packet.
*
*  \param   [in]    nodeaddr Address of the slave node for which the SPI to I2C transaction is intended for.
*
*  \param   [in]    spiSs   SPI slave select. Only applicable for remote SPI peripheral access using SPI.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiDtBulkWrite(a2b_StackContext* ctx, a2b_SpiCmd cmd, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_TUNNEL_BULK_WRITE, A2B_CMD_MAX_LENINBYTES_SPI_TUNNEL_BULK_WRITE);

	if (A2B_SUCCEEDED(result))
	{
		/* Formatting the A2B SPI protocol buffer based on the cmd */
		spiProcolBuf[0u] = (a2b_UInt8)cmd;

		/* Prepares the SPI slave select/Node byte. Currently it is assumed that SPI target is always the slave node */
		spiProcolBuf[1u] = a2b_prepSpiSsNodeByte(nodeaddr, 0U, spiSs);

		spiProcolBuf[2u] = (a2b_UInt8)(nWrite - 1u);

		/* Copy the actual SPI data to A2B SPI protocol buffer */
		(void)a2b_memcpy(&spiProcolBuf[3], wbuf, (a2b_Size)nWrite);

		/* Call SPI write function */
		result = a2b_spiWrite(ctx, A2B_SPI_SLV_SEL, nWrite + A2B_CMD_LEN_BULK_MODE, spiProcolBuf);

		/* Monitor the SPI busy status  */
		(void)a2b_spiCheckBusyStat(ctx, A2B_SPI_SLV_SEL,  A2B_SPI_MAX_BUSY_CHECK_CNT);
	}
	else
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "a2b_spiDtBulkWrite - Failed SPI to SPI Write: %lu", &result));
	}

	return result;
}

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiDtBulkWriteNb
*
*  The SPI bulk write command is used to write to remote SPI peripherals connected to slave nodes.
*  Bulk SPI transactions do not wait for all of the payload data to reach the remote node before starting the remote transaction.
*  This is an asynchronous call and will not block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    cmd		 A2B SPI command added to the SPI packet.
*
*  \param   [in]    nodeaddr Address of the slave node for which the SPI to I2C transaction is intended for.
*
*  \param   [in]    spiSs   SPI slave select. Only applicable for remote SPI peripheral access using SPI.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiDtBulkWriteNb(a2b_StackContext* ctx, a2b_SpiCmd cmd, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, a2b_Byte *wbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_TUNNEL_BULK_WRITE, A2B_CMD_MAX_LENINBYTES_SPI_TUNNEL_BULK_WRITE);


	if (A2B_SUCCEEDED(result))
	{
		/* Formatting the A2B SPI protocol buffer based on the cmd */
		spiProcolBuf[0u] = (a2b_UInt8)cmd;

		/* Prepares the SPI slave select/Node byte. Currently it is assumed that SPI target is always the slave node */
		spiProcolBuf[1u] = a2b_prepSpiSsNodeByte(nodeaddr, 0U, spiSs);

		spiProcolBuf[2u] = (a2b_UInt8)(nWrite - 1u);

		/* Copy the actual SPI data to A2B SPI protocol buffer */
		(void)a2b_memcpy(&spiProcolBuf[3], wbuf, (a2b_Size)nWrite);

		/* Call SPI write function */
		result = a2b_spiWrite(ctx, A2B_SPI_SLV_SEL, nWrite + A2B_CMD_LEN_BULK_MODE, spiProcolBuf);

		/* Start a timer */
		a2b_remoteDevConfigSpiToSpiStartTimer(ctx);
	}
	else
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "a2b_spiDtBulkWriteNb - Failed SPI to SPI Write: %lu", &result));
	}

	return result;
}

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiDtFullDuplex
*
*  The SPI Full duplex command is used to write to remote SPI peripherals connected to slave nodes and read data from the remote slave nodes.
*  This is an synchronous call and will block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    cmd		 A2B SPI command added to the SPI packet.
*
*  \param   [in]    nodeaddr Address of the slave node for which the SPI to I2C transaction is intended for.
*
*  \param   [in]    spiSs   SPI slave select. Only applicable for remote SPI peripheral access using SPI.
*
*  \param	[in]	bFdCmdBased Flag to indicate whether the full duplex mode is register based or command based.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \param   [in]    nRead  The number of bytes to read.
*
*  \param   [in]    rbuf    Values read from the remote slave nodes is stored in this buffer.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiDtFullDuplex(a2b_StackContext* ctx, a2b_SpiCmd cmd, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_Bool bFdCmdBased, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	if(bFdCmdBased == A2B_TRUE)
	{
		result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED, A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED);
	}
	else
	{
		result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED, A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED);
	}

	if (A2B_SUCCEEDED(result))
	{
		if(bFdCmdBased == A2B_TRUE)
		{
			/* Formatting the A2B SPI protocol buffer based on the cmd */
			spiProcolBuf[0u] = (a2b_UInt8)cmd;

			/* Prepares the SPI slave select/Node byte. Currently it is assumed that SPI target is always the slave node */
			spiProcolBuf[1u] = a2b_prepSpiSsNodeByte(nodeaddr, 0U, spiSs);

			spiProcolBuf[2u] = (a2b_UInt8)(nRead - 1u);

			/* Copy the actual SPI data to A2B SPI protocol buffer */
			(void)a2b_memcpy(&spiProcolBuf[3], wbuf, (a2b_Size)nWrite);

			/* Call SPI Fd function */
			result = a2b_spiFd(ctx, A2B_SPI_SLV_SEL, nWrite + A2B_CMD_LEN_FULL_DUPLEX_MODE, spiProcolBuf, nRead + A2B_CMD_LEN_FULL_DUPLEX_MODE, rbuf);
		}
		else
		{
			/* Call SPI Fd function */
			result = a2b_spiFd(ctx, A2B_SPI_SLV_SEL, nWrite, wbuf, nRead, rbuf);
		}

		/* Monitor the SPI busy status  */
		(void)a2b_spiCheckBusyStat(ctx, A2B_SPI_SLV_SEL,  A2B_SPI_MAX_BUSY_CHECK_CNT);
	}
	else
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "a2b_spiDtFullDuplex - Failed SPI to SPI Full Duplex: %lu", &result));
	}

	return result;
}

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiDtFullDuplexNb
*
*  The SPI Full duplex command is used to write to remote SPI peripherals connected to slave nodes and read data from the remote slave nodes.
*  This is an asynchronous call and will not block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    cmd		 A2B SPI command added to the SPI packet.
*
*  \param   [in]    nodeaddr Address of the slave node for which the SPI to I2C transaction is intended for.
*
*  \param   [in]    spiSs   SPI slave select. Only applicable for remote SPI peripheral access using SPI.
*
*  \param	[in]	bFdCmdBased Flag to indicate whether the full duplex mode is register based or command based.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wbuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \param   [in]    nRead  The number of bytes to read.
*
*  \param   [in]    rbuf    Values read from the remote slave nodes is stored in this buffer.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiDtFullDuplexNb(a2b_StackContext* ctx,  a2b_SpiCmd cmd, a2b_Int16 nodeaddr, a2b_UInt16 spiSs, a2b_Bool bFdCmdBased, a2b_UInt16 nWrite, a2b_Byte *wbuf, a2b_UInt16 nRead, a2b_Byte *rbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	if(bFdCmdBased == A2B_TRUE)
	{
		result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED, A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED);
	}
	else
	{
		result = isSpiLengthValid(nWrite, A2B_CMD_MIN_LENINBYTES_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED, A2B_CMD_MAX_LENINBYTES_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED);
	}

	if (A2B_SUCCEEDED(result))
	{
		if(bFdCmdBased == A2B_TRUE)
		{
			/* Formatting the A2B SPI protocol buffer based on the cmd */
			spiProcolBuf[0u] = (a2b_UInt8)cmd;

			/* Prepares the SPI slave select/Node byte. Currently it is assumed that SPI target is always the slave node */
			spiProcolBuf[1u] = a2b_prepSpiSsNodeByte(nodeaddr, 0U, spiSs);

			spiProcolBuf[2u] = (a2b_UInt8)(nRead - 1u);

			/* Copy the actual SPI data to A2B SPI protocol buffer */
			(void)a2b_memcpy(&spiProcolBuf[3], wbuf, (a2b_Size)nWrite);

			/* Call SPI Fd function */
			result = a2b_spiFd(ctx, A2B_SPI_SLV_SEL, nWrite + A2B_CMD_LEN_FULL_DUPLEX_MODE, spiProcolBuf, nRead + A2B_CMD_LEN_FULL_DUPLEX_MODE, rbuf);
		}
		else
		{
			/* Call SPI Fd function */
			result = a2b_spiFd(ctx, A2B_SPI_SLV_SEL, nWrite, wbuf, nRead, rbuf);
		}

		/* Start a timer */
		a2b_remoteDevConfigSpiToSpiStartTimer(ctx);
	}
	else
	{
		A2B_TRACE1((ctx, (A2B_TRC_DOM_STACK | A2B_TRC_LVL_ERROR), "a2b_spiDtFullDuplexNb - Failed SPI to SPI Full Duplex: %lu", &result));
	}

	return result;
}

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               isSpiLengthValid
*
*  Checks the SPI length validity based on the minimum and maximum possible
*  lengths for a given A2B API command
*
*  \param   [in]    length	Actual SPI length in bytes.
*
*  \param   [in]    min		Allowed minimum A2B SPI length in bytes.
*
*  \param   [in]    max		Maximum possible A2B SPI length in bytes
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult isSpiLengthValid(a2b_UInt16 length, a2b_UInt16 min, a2b_UInt16 max)
{
	a2b_HResult	result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_SPI_INVALID_LENGTH);

	if ((length >= min) && (length <= max))
	{
		result = A2B_RESULT_SUCCESS;
	}

	return result;
}	/* isSpiLengthValid */



/*!****************************************************************************
* \ingroup         a2bstack_spi_priv
*
* \b               a2b_spiPrepAccess
*
* Prepares SPI access to the master node, slave nodes, or peripheral
* devices attached to a slave node. Since all SPI access is tunneled
* through the master node the master must be configured to pass the SPI
* command appropriately. The last access "mode" is cached to avoid
* repeated configuration of AD2410 registers that do not change between
* access to the same entity.
*
* \param   [in]    ctx         The stack context.
*
* \param   [in]    cmd         The SPI command to execute.
*
* \param   [in]    nodeAddr    The destination A2B node address. Only
*                              applicable to slave and peripheral accesses
*                              and ignored otherwise.
*
* \param   [in]    chipAddr    SPI slave select or the peripheral I2C address. Only applicable
*                              to peripheral accesses and ignored
*                              otherwise.
*
* \pre     None
*
* \post    None
*
* \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*          #A2B_FAILED() for success or failure of the operation.
*
******************************************************************************/
static a2b_HResult a2b_spiPrepAccess(a2b_StackContext* ctx, a2b_SpiCmd cmd, a2b_Int16 nodeAddr, a2b_UInt16 chipAddr)
{
    a2b_Byte buf[2];
    a2b_HResult result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INVALID_PARAMETER);

    if ( A2B_NULL != ctx )
    {
        switch ( cmd )
        {
			case A2B_CMD_SPI_LOCAL_REG_WRITE:
			case A2B_CMD_SPI_LOCAL_REG_READ:
			case A2B_CMD_SPI_SLAVE_REG_WRITE:
			case A2B_CMD_SPI_SLAVE_REG_READ_REQUEST:
			case A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE:
			case A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST:
			case A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE:
			case A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED:
			case A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED:
			case A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE_EXTENDED:
			case A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_EXTENDED:
				/* No chip or node address change */
				result = A2B_RESULT_SUCCESS;
				break;

			case A2B_CMD_SPI_REMOTE_I2C_WRITE:
			case A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST:
				if ( ((a2b_UInt32)ctx->stk->spiMode.access == (a2b_UInt32)A2B_I2C_ACCESS_PERIPH) &&
				     (nodeAddr == ctx->stk->spiMode.nodeAddr) &&
				     (chipAddr == ctx->stk->spiMode.chipAddr) &&
				     (!ctx->stk->spiMode.broadcast ))
				{
					/* No changes to the current settings */
					result = A2B_RESULT_SUCCESS;
				}
				else
				{
					buf[0] = A2B_REG_CHIP;
					buf[1] = (a2b_Byte)chipAddr;
					result = a2b_spiBusRegWrite(ctx, A2B_SPI_SLV_SEL, nodeAddr, 2u, buf);
					if ( A2B_FAILED(result) )
					{
						/* Reset the last access mode so the next time the
						 * cached values won't be assumed.
						 */
						a2b_stackResetSpiLastMode(&ctx->stk->spiMode);
					}
					/* Else successfully prepared for access */
					else
					{
						/* Cache the last SPI access mode */
						ctx->stk->spiMode.access 	= A2B_SPI_ACCESS_PERIPH;
						ctx->stk->spiMode.nodeAddr 	= nodeAddr;
						ctx->stk->spiMode.chipAddr 	= chipAddr;
						ctx->stk->spiMode.broadcast = A2B_FALSE;
					}
				}
				break;

			default:
				/* Unknown command */
				result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INTERNAL);
				break;
        }
    }

    return result;

} /* a2b_spiPrepAccess */

/*!****************************************************************************
* \ingroup         a2bstack_spi_priv
*
* \b               a2b_spiExecuteAccess
*
* Prepares SPI access to the master node, slave nodes, or peripheral
* devices attached to a slave node. Since all SPI access is tunneled
* through the master node the master must be configured to pass the SPI
* command appropriately. The last access "mode" is cached to avoid
* repeated configuration of AD2410 registers that do not change between
* access to the same entity.
*
* \param   [in]    ctx         The stack context.
*
* \param   [in]    cmd         The SPI command to execute.
*
* \param   [in]    nodeAddr    The destination A2B node address. Only
*                              applicable to slave and peripheral accesses
*                              and ignored otherwise.
*
* \param   [in]    chipAddr    Peripheral I2C address. Only applicable for remote I2C peripheral access using SPI
*
* \param   [in]    spiSs       Peripheral SPI slave select. Only applicable for remote SPI peripheral access using SPI.
*
* \param   [in]    nWrite      Length in bytes to write
*
* \param   [in]    wBuf		   Write Buffer pointer of actual SPI bytes
*
* \param   [in]    nRead       Length in bytes to read
*
* \param   [in]    rBuf		   Read Buffer pointer for actual SPI bytes
*
* \pre     None
*
* \post    None
*
* \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*          #A2B_FAILED() for success or failure of the operation.
*
******************************************************************************/
static a2b_HResult a2b_spiExecuteAccess(a2b_StackContext* ctx, a2b_SpiCmd cmd, a2b_Int16  nodeAddr, a2b_UInt16 chipAddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, void* wBuf, a2b_UInt16 nRead, void* rBuf)
{
	a2b_HResult		result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INVALID_PARAMETER);

	if (A2B_NULL != ctx)
	{
		/* Prepare the access to the master/slave/peripheral by setting
		 * up some configuration registers in the master node.
		 */
		result = a2b_spiPrepAccess(ctx, cmd, nodeAddr, chipAddr);

		if ( A2B_SUCCEEDED(result) )
		{
			switch (cmd)
			{
				case A2B_CMD_SPI_LOCAL_REG_WRITE:
					result = a2b_spiLocalRegWrite		(ctx, spiSs, nWrite, wBuf);
					break;

				case A2B_CMD_SPI_LOCAL_REG_READ:
					result = a2b_spiLocalRegWriteRead	(ctx, spiSs, nWrite, wBuf, nRead, rBuf);
					break;

				case A2B_CMD_SPI_SLAVE_REG_WRITE:
					result = a2b_spiBusRegWrite			(ctx, spiSs, nodeAddr, nWrite, wBuf);
					break;

				case A2B_CMD_SPI_SLAVE_REG_READ_REQUEST:
					result = a2b_spiBusRegWriteRead		(ctx, spiSs, nodeAddr, nWrite, wBuf, nRead, rBuf);
					break;

				case A2B_CMD_SPI_REMOTE_I2C_WRITE:
					result = a2b_spiRemoteI2cWrite		(ctx, spiSs, nodeAddr, nWrite, wBuf);
					break;

				case A2B_CMD_SPI_REMOTE_I2C_READ_REQUEST:
					result = a2b_spiRemoteI2cWriteRead	(ctx, spiSs, nodeAddr, nWrite, wBuf, nRead, rBuf);
					break;

				case A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_WRITE:
					if(ctx->stk->oSpiInfo.eApiMode == A2B_API_BLOCKING)
					{
						result = a2b_spiDtAtomicLargeWrite(ctx, nodeAddr, spiSs, nWrite, wBuf);
					}
					else
					{
						result = a2b_spiDtAtomicLargeWriteNb(ctx, nodeAddr, spiSs, nWrite, wBuf);
					}
					break;

				case A2B_CMD_SPI_DATA_TUNNEL_ATOMIC_LARGE_READ_REQUEST:
					if(ctx->stk->oSpiInfo.eApiMode == A2B_API_BLOCKING)
					{
						result = a2b_spiDtAtomicLargeWriteRead (ctx, nodeAddr, spiSs, nWrite, wBuf, nRead, rBuf);
					}
					else
					{
						result = a2b_spiDtAtomicLargeWriteReadNb(ctx, nodeAddr, spiSs, nWrite, wBuf, nRead, rBuf);
					}
					break;

				case A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE:
				case A2B_CMD_SPI_DATA_TUNNEL_BULK_WRITE_EXTENDED:
					if(ctx->stk->oSpiInfo.eApiMode == A2B_API_BLOCKING)
					{
						result = a2b_spiDtBulkWrite(ctx, cmd, nodeAddr, spiSs, nWrite, wBuf);
					}
					else
					{
						result = a2b_spiDtBulkWriteNb(ctx, cmd, nodeAddr, spiSs, nWrite, wBuf);
					}
					break;

				case A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_CMD_BASED:
				case A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_EXTENDED:
					if(ctx->stk->oSpiInfo.eApiMode == A2B_API_BLOCKING)
					{
						result = a2b_spiDtFullDuplex (ctx, cmd, nodeAddr, spiSs, A2B_TRUE, nWrite, wBuf, nRead, rBuf);
					}
					else
					{
						result = a2b_spiDtFullDuplexNb(ctx, cmd, nodeAddr, spiSs, A2B_TRUE, nWrite, wBuf, nRead, rBuf);
					}
					break;

				case A2B_CMD_SPI_DATA_TUNNEL_FULL_DUPLEX_REG_BASED:
					if(ctx->stk->oSpiInfo.eApiMode == A2B_API_BLOCKING)
					{
						result = a2b_spiDtFullDuplex(ctx, cmd, nodeAddr, spiSs, A2B_FALSE, nWrite, wBuf, nRead, rBuf);
					}
					else
					{
						result = a2b_spiDtFullDuplexNb(ctx, cmd, nodeAddr, spiSs, A2B_FALSE, nWrite, wBuf, nRead, rBuf);
					}
					break;

				case A2B_CMD_SPI_ABORT:
					break;

				default:
					result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE, A2B_FAC_SPI, A2B_EC_INVALID_PARAMETER);
					break;
			}
		}
	}

	return result;
} /* a2b_spiPrepAccess */

/*!****************************************************************************
*
*  \b   a2b_spiMasterWrite
*
*  Writes bytes to the master node. Only the master plugin can issue a
*  write request. All other slave plugins and applications requesting to
*  write the master node will fail. For the initial write, the first
*  byte of the buffer is treated as the AD2410 register offset by the chip.
*  For subsequent write bytes the chip will auto-increment the register
*  offset with each byte written. This is an synchronous call and will block
*  until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wBuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_spiMasterWrite(struct a2b_StackContext* ctx, a2b_UInt16 nWrite, void* wBuf)
{
	a2b_HResult result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
		A2B_FAC_SPI,
		A2B_EC_INVALID_PARAMETER);

	if ((A2B_NULL != ctx) &&
		((A2B_NULL != wBuf) || (nWrite == 0u)))
	{
		/* Only the master plugin is allowed to read/write the master node */
		if ((ctx->domain != A2B_DOMAIN_PLUGIN) ||
			(ctx->ccb.plugin.nodeSig.nodeAddr != A2B_NODEADDR_MASTER))
		{
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
				A2B_FAC_SPI,
				A2B_EC_PERMISSION);
		}
		else
		{
			result = a2b_spiExecuteAccess(	ctx,
											A2B_CMD_SPI_LOCAL_REG_WRITE,
											A2B_NODEADDR_MASTER,
											0u,         /* chipAddr - unused */
											A2B_SPI_SLV_SEL,
											nWrite,
											wBuf,
											0u,         /* nRead - unused */
											A2B_NULL); /* rBuf - unused */
		}
	}

	return result;

} /* a2b_spiMasterWrite */

/*!****************************************************************************
*
*  \b   a2b_spiMasterWriteRead
*
*  Writes and then reads bytes from the master node *without* an SPI stop
*  sequence separating the two operations. Instead a repeated SPI start
*  sequence is used as the operation separator. Only the master plugin can
*  issue a write/read request. All other slave plugins and applications
*  requesting to write/read the master node will fail. For the initial write,
*  the first byte of the buffer is treated as the AD2410 register offset by
*  the chip. For subsequent write bytes the chip will auto-increment the
*  register offset with each byte written. The read operation will continue
*  at the last register offset. This is an synchronous call and will block
*  until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write/read.
*
*  \param   [in]    nWrite  The number of bytes to write.
*
*  \param   [in]    wBuf    A buffer containing the data to write. The buffer
*                           is of size 'nWrite' bytes.
*
*  \param   [in]    nRead   The number of bytes to read from the master node.
*
*  \param   [in]    rBuf    A buffer in which to write the results of the read.
*
*  \pre     None
*
*  \post    The read buffer holds the contents of the read on success.
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_spiMasterWriteRead(struct a2b_StackContext* ctx, a2b_UInt16 nWrite, void* wBuf, a2b_UInt16 nRead, void* rBuf)
{
	a2b_HResult result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
		A2B_FAC_SPI,
		A2B_EC_INVALID_PARAMETER);

	if ((A2B_NULL != ctx) &&
		((A2B_NULL != wBuf) || (nWrite == 0u)) &&
		((A2B_NULL != rBuf) || (nRead == 0u)))
	{
		/* Only the master plugin is allowed to read/write the master node */
		if ((ctx->domain != A2B_DOMAIN_PLUGIN) ||
			(ctx->ccb.plugin.nodeSig.nodeAddr != A2B_NODEADDR_MASTER))
		{
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
				A2B_FAC_SPI,
				A2B_EC_PERMISSION);
		}
		else
		{
			result = a2b_spiExecuteAccess(  ctx,
											A2B_CMD_SPI_LOCAL_REG_READ,
											A2B_NODEADDR_MASTER,
											0u,         /* chipAddr - unused */
											A2B_SPI_SLV_SEL,
											nWrite,
											wBuf,
											nRead,
											rBuf);
		}
	}

	return result;

} /* a2b_spiMasterWriteRead */

/*!****************************************************************************
*
*  \b   a2b_spiSlaveWrite
*
*  Writes bytes to the slave. Only the master plugin can issue a write request.
*  All other slave plugins and applications requesting to write to a slave node
*  will fail. The first byte of the buffer is treated as the AD2410 register
*  offset by the chip. For subsequent write bytes the chip will auto-increment
*  the register offset with each byte written. This is an synchronous call and
*  will block until the operation is complete.
*
*  \param   [in]        ctx         The stack context associated with the
*                                   write.
*
*  \param   [in]        node        The slave node to write.
*
*  \param   [in]        nWrite      The number of bytes in the 'wBuf' buffer to
*                                   write to the slave.
*
*  \param   [in]        wBuf        A buffer containing the data to write.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_spiSlaveWrite(struct a2b_StackContext* ctx, a2b_Int16 node, a2b_UInt16 nWrite, void* wBuf)
{
	a2b_HResult result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
		A2B_FAC_SPI,
		A2B_EC_INVALID_PARAMETER);

	if ((A2B_NULL != ctx) &&
		((A2B_NULL != wBuf) || (nWrite == 0u)))
	{
		/* Only the master plugin is allowed to read/write the slave node */
		if ((ctx->domain != A2B_DOMAIN_PLUGIN) ||
			(ctx->ccb.plugin.nodeSig.nodeAddr != A2B_NODEADDR_MASTER))
		{
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
				A2B_FAC_SPI,
				A2B_EC_PERMISSION);
		}
		else if (((a2b_Bool)(node < 0)) || ((a2b_Bool)(node >= (a2b_Int16)A2B_CONF_MAX_NUM_SLAVE_NODES)))
		{
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
				A2B_FAC_SPI,
				A2B_EC_INVALID_PARAMETER);
		}
		else
		{
			result = a2b_spiExecuteAccess(	ctx,
											A2B_CMD_SPI_SLAVE_REG_WRITE,
											node,
											0u,         /* chipAddr - unused */
											A2B_SPI_SLV_SEL,
											nWrite,
											wBuf,
											0u,         /* nRead - unused */
											A2B_NULL); /* rBuf - unused*/
		}
	}

	return result;

} /* a2b_spiSlaveWrite */

/*!****************************************************************************
*
* \b   a2b_spiSlaveWriteRead
*
* Writes and then reads bytes from the slave node *without* an SPI stop
* sequence separating the two operations. Instead a repeated SPI start
* sequence is used as the operation separator. Only the master plugin can
* issue a write request. All other slave plugins and applications requesting
* to write/read a slave node will fail. For the initial write, the first
* byte of the buffer is treated as the AD2410 register offset by the chip.
* For subsequent write bytes the chip will auto-increment the register
* offset with each byte written. The read operation will continue at the
* last register offset. This is an synchronous call and will block until
* the operation is complete.
*
* \param   [in]        ctx         The stack context associated with the
*                                  write/read.
*
* \param   [in]        node        The slave node to write/read.
*
* \param   [in]        nWrite      The number of bytes in the 'wBuf' buffer to
*                                  write to the peripheral.
*
* \param   [in]        wBuf        A buffer containing the data to write.
*
* \param   [in]        nRead       The number of bytes to read from the slave
*                                  node.
*
* \param   [in,out]    rBuf        A buffer in which to write the results of
*                                  the read.
*
* \pre     None
*
* \post    On success 'rBuf' holds the data that was read from the slave.
*
* \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*          #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_spiSlaveWriteRead(struct a2b_StackContext* ctx, a2b_Int16 node, a2b_UInt16 nWrite, void* wBuf, a2b_UInt16 nRead, void* rBuf)
{
	a2b_HResult result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
		A2B_FAC_SPI,
		A2B_EC_INVALID_PARAMETER);

	if ((A2B_NULL != ctx) &&
		((A2B_NULL != wBuf) || (nWrite == 0u)) &&
		((A2B_NULL != rBuf) || (nRead == 0u)))
	{
		/* Only the master plugin is allowed to read/write the slave node */
		if ((ctx->domain != A2B_DOMAIN_PLUGIN) ||
			(ctx->ccb.plugin.nodeSig.nodeAddr != A2B_NODEADDR_MASTER))
		{
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
				A2B_FAC_SPI,
				A2B_EC_PERMISSION);
		}
		else if (((a2b_Bool)(node < 0)) || ((a2b_Bool)(node >= (a2b_Int16)A2B_CONF_MAX_NUM_SLAVE_NODES)))
		{
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
				A2B_FAC_SPI,
				A2B_EC_INVALID_PARAMETER);
		}
		else
		{
			result = a2b_spiExecuteAccess(	ctx,
											A2B_CMD_SPI_SLAVE_REG_READ_REQUEST,
											node,
											0u,         /* chipAddr - unused */
											A2B_SPI_SLV_SEL,
											nWrite,
											wBuf,
											nRead,
											rBuf);
		}
	}

	return result;

} /* a2b_spiSlaveWriteRead */

/*!****************************************************************************
*
*  \b   a2b_spiSlaveWrite
*
*  Writes bytes to the slave. Only the master plugin can issue a write request.
*  All other slave plugins and applications requesting to write to a slave node
*  will fail. The first byte of the buffer is treated as the AD2410 register
*  offset by the chip. For subsequent write bytes the chip will auto-increment
*  the register offset with each byte written. This is an synchronous call and
*  will block until the operation is complete.
*
*  \param   [in]        ctx         The stack context associated with the
*                                   write.
*
*  \param   [in]        nWrite      The number of bytes in the 'wBuf' buffer to
*                                   write to the slave.
*
*  \param   [in]        wBuf        A buffer containing the data to write.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_spiSlaveBroadcastWrite(struct a2b_StackContext* ctx, a2b_UInt16 nWrite, void* wBuf)
{
	a2b_HResult result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
		A2B_FAC_SPI,
		A2B_EC_INVALID_PARAMETER);

	if ((A2B_NULL != ctx) &&
		((A2B_NULL != wBuf) || (nWrite == 0u)))
	{
		/* Only the master plugin is allowed to read/write the slave node */
		if ((ctx->domain != A2B_DOMAIN_PLUGIN) ||
			(ctx->ccb.plugin.nodeSig.nodeAddr != A2B_NODEADDR_MASTER))
		{
			result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
				A2B_FAC_SPI,
				A2B_EC_PERMISSION);
		}
		else
		{
			result = a2b_spiExecuteAccess(	ctx,
											A2B_CMD_SPI_SLAVE_REG_WRITE,
											0x80,
											0u,         /* chipAddr - unused */
											A2B_SPI_SLV_SEL,
											nWrite,
											wBuf,
											0u,         /* nRead - unused */
											A2B_NULL); /* rBuf - unused*/
		}
	}

	return result;

} /* a2b_spiSlaveWrite */

/*!****************************************************************************
*
*  \b   a2b_spiPeriphWrite
*
*  Writes bytes to the slave node's peripheral. This is a synchronous call and will block until
*  the operation is complete.
*
*  \param   [in]        ctx     The stack context associated with the write.
*
*  \param   [in]        node    The slave node to write.
*
*  \param   [in]        spiCmd	The SPI command to execute.
*
*  \param   [in]    	chipAddr Peripheral I2C address. Only applicable for remote I2C peripheral access using SPI
*
*  \param   [in]    	spiSs   Peripheral SPI slave select. Only applicable for remote SPI peripheral access using SPI.
*
*  \param   [in]        nWrite  The number of bytes in the 'wBuf' buffer to
*                               write to the peripheral.
*
*  \param   [in]        wBuf    A buffer containing the data to write. The
*                               amount of data to write is specified by the
*                               'nWrite' parameter.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_spiPeriphWrite(struct a2b_StackContext* ctx, a2b_Int16 node, a2b_UInt16 spiCmd, a2b_UInt16 chipAddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, void* wBuf)
{
    a2b_HResult result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                        A2B_FAC_SPI,
                                        A2B_EC_INVALID_PARAMETER);

    if ( (A2B_NULL != ctx) &&
        ((A2B_NULL != wBuf) || (nWrite == 0u)) )
    {

	    if((a2b_Bool)(node  == A2B_NODEADDR_MASTER))
    	{
    		return(a2b_spiGenericWrite(ctx, node, spiSs, nWrite, wBuf));
    	}
        if ( ((a2b_Bool)(node < 0)) || ((a2b_Bool)(node >= (a2b_Int16)A2B_CONF_MAX_NUM_SLAVE_NODES)) )
        {
            result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                    A2B_FAC_SPI,
                                    A2B_EC_INVALID_PARAMETER);
        }
        /* Only applications, the master plugin, or a slave plugin with
         * the same node address is allowed to access attached peripherals.
         */
        else if ( (ctx->domain == A2B_DOMAIN_PLUGIN) &&
                (ctx->ccb.plugin.nodeSig.nodeAddr != A2B_NODEADDR_MASTER) &&
                (ctx->ccb.plugin.nodeSig.nodeAddr != node) )
        {
            result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                        A2B_FAC_SPI,
                                        A2B_EC_PERMISSION);
        }
        else
        {
            result = a2b_spiExecuteAccess(ctx,
            							  (a2b_SpiCmd)spiCmd,
                                          node,
                                          chipAddr,
										  spiSs,
                                          nWrite,
                                          wBuf,
                                          0u,            /* nRead - unused */
                                          A2B_NULL);     /* rBuf - unused */
        }
    }

    return result;

} /* a2b_spiPeriphWrite */

/*!****************************************************************************
*
*  \b   a2b_spiPeriphWriteRead
*
*  Writes and then reads bytes from the slave node's peripheral. This is a synchronous call and will block until
*  the operation is complete.
*
*  \param   [in]        ctx     The stack context associated with the
*                               write/read.
*
*  \param   [in]        node    The slave node to write/read.
*
*  \param   [in]        spiCmd	The SPI command to execute.
*
*  \param   [in]    	chipAddr Peripheral I2C address. Only applicable for remote I2C peripheral access using SPI
*
*  \param   [in]    	spiSs   Peripheral SPI slave select. Only applicable for remote SPI peripheral access using SPI.
*
*  \param   [in]        nWrite  The number of bytes in the 'wBuf' buffer to
*                               write to the peripheral.
*
*  \param   [in]        wBuf    A buffer containing the data to write. The
*                               amount of data to write is specified by the
*                               'nWrite' parameter.
*
*  \param   [in]        nRead   The number of bytes to read from the
*                               peripheral. The 'rBuf' parameter must have
*                               enough space to hold this number of bytes.
*
*  \param   [in,out]    rBuf    The buffer in which to write the results of
*                               the read. It's assumed the buffer is sized to
*                               accept 'nRead' bytes of data.
*
*  \pre     None
*
*  \post    On success 'rBuf' holds the data that was read from the
*           peripheral.
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_spiPeriphWriteRead(struct a2b_StackContext* ctx, a2b_Int16 node, a2b_UInt16 spiCmd, a2b_UInt16 chipAddr, a2b_UInt16 spiSs, a2b_UInt16 nWrite, void* wBuf, a2b_UInt16 nRead, void* rBuf)
{
    a2b_HResult result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                        A2B_FAC_SPI,
                                        A2B_EC_INVALID_PARAMETER);

    if ( (A2B_NULL != ctx) &&
        ((A2B_NULL != rBuf) || (nRead == 0u)) &&
        ((A2B_NULL != wBuf) || (nWrite == 0u)) )
    {
    	if((a2b_Bool)(node  == A2B_NODEADDR_MASTER))
    	{
    		return(a2b_spiGenericWriteRead(ctx, node, spiSs, nWrite, wBuf, nRead, rBuf));
    	}

        if ( ((a2b_Bool)(node < 0)) || ((a2b_Bool)(node >= (a2b_Int16)A2B_CONF_MAX_NUM_SLAVE_NODES)) )
        {
            result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                    A2B_FAC_SPI,
                                    A2B_EC_INVALID_PARAMETER);
        }
        /* Only applications, the master plugin, or a slave plugin with
         * the same node address is allowed to access attached peripherals.
         */
        else if ( (ctx->domain == A2B_DOMAIN_PLUGIN) &&
                (ctx->ccb.plugin.nodeSig.nodeAddr != A2B_NODEADDR_MASTER) &&
                (ctx->ccb.plugin.nodeSig.nodeAddr != node) )
        {
            result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                        A2B_FAC_SPI,
                                        A2B_EC_PERMISSION);
        }
        else
        {
            result = a2b_spiExecuteAccess(ctx,
            							  (a2b_SpiCmd)spiCmd,
                                          node,
                                          chipAddr,
										  spiSs,
                                          nWrite,
                                          wBuf,
                                          nRead,
                                          rBuf);
        }
    }

    return result;

} /* a2b_spiPeriphWriteRead */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiStatusRead
*
*  The SPI status read transaction is used to read the A2B_SPISTAT register and to determine the status of ongoing transactions.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    spiSs   SPI slave select.
* 
*  \param   [in]    nRead   The number of bytes to read from the device.
*
*  \param   [in]    rbuf    A buffer in which to write the results of the read.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_spiStatusRead(struct a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_UInt16 nRead, a2b_Byte *rbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_STATUS_READ;

	/* Call SPI Write Read function */
	result = a2b_spiWriteRead(ctx, spiSs, 1u, spiProcolBuf, nRead, rbuf);

	return result;
}	/* a2b_spiStatusRead */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiAbort
*
*  The SPI ABORT Command transaction is used to clear the SPI FIFO Buffer.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    spiSs   Peripheral SPI slave select. Only applicable for remote SPI peripheral access using SPI.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_spiAbort(struct a2b_StackContext* ctx, a2b_UInt16 spiSs)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;
	a2b_Byte			rbuf;

	spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_ABORT;

	/* Call SPI Write Read function */
	result = a2b_spiWriteRead(ctx, spiSs, 1u, spiProcolBuf, 1, &rbuf);

	return result;
}	/* a2b_spiAbort */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_spiDtFifoRead
*
*  The SPI data tunnel FIFO read command is used to read from remote SPI peripherals connected to slave nodes.
*  This is first asynchronous call and will not block until the operation is complete.
*
*  \param   [in]    ctx     The stack context associated with the write.
*
*  \param   [in]    nRead   The number of bytes to read from the device.
*
*  \param   [in]    rbuf    A buffer in which to write the results of the read.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_HResult a2b_spiDtFifoRead(struct a2b_StackContext* ctx, a2b_UInt16 nRead, a2b_Byte *rbuf)
{
	a2b_HResult	result;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;

	spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_DATA_TUNNEL_FIFO_READ;

	/* Call SPI Write Read function */
	result = a2b_spiWriteRead(ctx, 0u, 1u, spiProcolBuf, nRead, rbuf);

	/* Start a timer */
	a2b_remoteDevConfigSpiToSpiStartTimer(ctx);

	return result;
}	/* a2b_spiDtFifoRead */

/*!****************************************************************************
*  \ingroup         a2bstack_spi_priv
*
*  \b               a2b_prepSpiSsNodeByte
*
*  The function prepares the SPI slave select/Node byte. This byte shall be used
*  for all SPI over distance communication types.
*
*  \param   [in]    nNodeaddr		The slave node id of the target when in the target is a slave (M/S = 0)
*
*  \param   [in]    nTrgtMstrSlave	Indicates if the target is the master node or slave node.
*  									If set (=1), it is the master node. If cleared (=0), it is the slave node.
*
*  \param   [in]    nSpiSs			Indicates the slave select to target
*
*  \pre     None
*
*  \post    None
*
*  \return  Prepared nSpiSsNodeByte
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_UInt8   a2b_prepSpiSsNodeByte(a2b_Int16 nNodeaddr, a2b_UInt16 nTrgtMstrSlave, a2b_UInt16 nSpiSs)
{
	a2b_UInt8   nSpiSsNodeByte = 0u;

	nSpiSsNodeByte   = ( (((a2b_UInt8)((a2b_UInt8)nNodeaddr 	  << A2B_BITP_SPISSNB_NODEID) & A2B_BITM_SPISSNB_NODEID)) |
						 (((a2b_UInt8)((a2b_UInt8)nTrgtMstrSlave << A2B_BITP_SPISSNB_MS) 	  & A2B_BITM_SPISSNB_MS)) 	 |
						 (((a2b_UInt8)((a2b_UInt8)nSpiSs 		  << A2B_BITP_SPISSNB_SSEL))	  & A2B_BITM_SPISSNB_SSEL) 	 );

	return (nSpiSsNodeByte);
}

/*!****************************************************************************
* \ingroup         a2bstack_spi_priv
*
* \b               a2b_spiCheckBusyStat
*
* Checks for SPI busy status
*
* \param   [in]    ctx         The stack context.
*
* \param   [in]    spiSs  SPI slave select
* \param   [in]    maxTryCnt  max retry count
* \pre     None
*
* \post    None
*
* \return  0 if SPI status is free
* 		   1 if SPI status is busy
*
******************************************************************************/
A2B_DSO_PUBLIC a2b_UInt8 a2b_spiCheckBusyStat(struct a2b_StackContext* ctx, a2b_UInt16 spiSs, a2b_UInt32 maxTryCnt)
{
	a2b_UInt8 	spiStat = 0, spiBusy = 0;
	a2b_HResult res;
	a2b_Byte		     *spiProcolBuf = ctx->stk->oSpiInfo.spiProcolBuf;
	a2b_UInt32			  tryCnt = 0u;
	spiProcolBuf[0u] = (a2b_UInt8)A2B_CMD_SPI_STATUS_READ;

	do
	{
		/* Call SPI Write Read function */
		res = a2b_spiWriteRead(ctx, spiSs, 1u, spiProcolBuf, 1u, &spiStat);
		if (res == 0u)
		{
			spiBusy = spiStat & (a2b_UInt8)A2B_BITM_SPISTAT_SPIBUSY;
		}
		else
		{
			/* assumed to be busy & break; */
			spiBusy = 0x01u;
			break;
		}
		/* Stay in the loop till busy */
		tryCnt++;
	}while((tryCnt < maxTryCnt) && (spiBusy == 0x01u));

	return (spiBusy);
}	/* a2b_spiCheckBusyStat */


/*!****************************************************************************
*
*  \b   a2b_spiGenericWrite
*
*  Writes bytes to the any SPI device. Only applications, or the
*  master plugin can issue an SPI write request to a peripheral device
*  attached to the specified node.
*  This is a synchronous call and will block until the operation is
*  complete.
*
*  \param   [in]        ctx     The stack context associated with the write.
*
*  \param   [in]        node    The slave node to write.
*
*  \param   [in]        chipAddr SPI slave select or the peripheral I2C address. Only applicable
*                                to peripheral accesses and ignored
*                                otherwise.
*
*  \param   [in]        nWrite  The number of bytes in the 'wBuf' buffer to
*                               write to the peripheral.
*
*  \param   [in]        wBuf    A buffer containing the data to write. The
*                               amount of data to write is specified by the
*                               'nWrite' parameter.
*
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiGenericWrite(a2b_StackContext* ctx, a2b_Int16 node, a2b_UInt16 spiSs, a2b_UInt16 nWrite, void* wBuf)
{
    a2b_HResult result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                        A2B_FAC_SPI,
                                        A2B_EC_INVALID_PARAMETER);

    if ( (A2B_NULL != ctx) &&
        ((A2B_NULL != wBuf) || (nWrite == 0u)) )
    {
		 if ( ((a2b_Bool)(node < A2B_NODEADDR_MASTER)) || ((a2b_Bool)(node >= (a2b_Int16)A2B_CONF_MAX_NUM_SLAVE_NODES)) )
        {
            result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                    A2B_FAC_SPI,
                                    A2B_EC_INVALID_PARAMETER);
        }
        /* Only applications, the master plugin is allowed to access attached peripherals. */
        else if ( (ctx->domain == A2B_DOMAIN_PLUGIN) &&
                (ctx->ccb.plugin.nodeSig.nodeAddr != A2B_NODEADDR_MASTER))
        {
            result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                        A2B_FAC_SPI,
                                        A2B_EC_PERMISSION);
        }
        else
        {
            result = a2b_spiWrite(ctx, spiSs, nWrite, wBuf);
        }
    }

    return result;

} /* a2b_spiGenericWrite */

/*!****************************************************************************
*
*  \b   a2b_spiGenericWriteRead
*
*  Writes bytes to the any SPI device. Only applications, or the
*  master plugin can issue an SPI write request to a peripheral device
*  attached to the specified node.
*  This is a synchronous call and will block until the operation is
*  complete.
*
*  \param   [in]        ctx     The stack context associated with the write.
*
*  \param   [in]        node    The slave node to write.
*
*  \param   [in]        chipAddr SPI slave select or the peripheral I2C address. Only applicable
*                                to peripheral accesses and ignored
*                                otherwise.
*
*  \param   [in]        nWrite  The number of bytes in the 'wBuf' buffer to
*                               write to the peripheral.
*
*  \param   [in]        wBuf    A buffer containing the data to write. The
*                               amount of data to write is specified by the
*                               'nWrite' parameter.
*
*  \param   [in]        nRead   The number of bytes to read from the
*                               peripheral. The 'rBuf' parameter must have
*                               enough space to hold this number of bytes.
*
*  \param   [in,out]    rBuf    The buffer in which to write the results of
*                               the read. It's assumed the buffer is sized to
*                               accept 'nRead' bytes of data.
*  \pre     None
*
*  \post    None
*
*  \return  A status code that can be checked with the #A2B_SUCCEEDED() or
*           #A2B_FAILED() for success or failure of the request.
*
******************************************************************************/
static a2b_HResult a2b_spiGenericWriteRead(a2b_StackContext* ctx, a2b_Int16 node, a2b_UInt16 spiSs, a2b_UInt16 nWrite, void* wBuf, a2b_UInt16 nRead, void* rBuf)
{
    a2b_HResult result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                        A2B_FAC_SPI,
                                        A2B_EC_INVALID_PARAMETER);

    if ( (A2B_NULL != ctx) &&
        ((A2B_NULL != rBuf) || (nRead == 0u)) &&
        ((A2B_NULL != wBuf) || (nWrite == 0u)) )
    {
        if ( ((a2b_Bool)(node < A2B_NODEADDR_MASTER)) ||
        		((a2b_Bool)(node >= (a2b_Int16)A2B_CONF_MAX_NUM_SLAVE_NODES)) )
        {
            result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                    A2B_FAC_SPI,
                                    A2B_EC_INVALID_PARAMETER);
        }
        /* Only applications, the master plugin is allowed to access attached peripherals. */
        else if ( (ctx->domain == A2B_DOMAIN_PLUGIN) &&
                (ctx->ccb.plugin.nodeSig.nodeAddr != A2B_NODEADDR_MASTER))
        {
            result = A2B_MAKE_HRESULT(A2B_SEV_FAILURE,
                                        A2B_FAC_SPI,
                                        A2B_EC_PERMISSION);
        }
        else
        {
            result = a2b_spiWriteRead(ctx, spiSs, nWrite, wBuf, nRead, rBuf);
        }
    }

    return result;

} /* a2b_spiGenericWrite */
