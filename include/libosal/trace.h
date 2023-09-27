/**
 * \file trace.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 02 May 2023
 *
 * \brief OSAL trace header.
 *
 * OSAL trace include header.
 */

/*
 * This file is part of libosal.
 *
 * libosal is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * libosal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with libosal; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LIBOSAL_TRACE__H
#define LIBOSAL_TRACE__H

#include <libosal/config.h>
#include <libosal/types.h>
#include <libosal/osal.h>
#include <libosal/trace.h>
#include <libosal/timer.h>

/** \defgroup trace_group Trace 
 * This module implements timing traces for use in realtime systems. 
 *
 * @{
 */

typedef struct osal_trace {
    osal_uint32_t cnt;                  //!< number of measurements
    osal_uint32_t act_buf;              //!< actual number of double buffer
    osal_uint32_t pos;                  //!< position in actual buffer.
    osal_binary_semaphore_t sync_sem;   //!< sync when buffer is full.
    osal_uint64_t *time_in_ns[2];       //!< time double buffer.
    osal_uint64_t *tmp;                 //!< calculation buffer.
} osal_trace_t;                         //!< Trace structure.

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Allocate trace struct.
/*!
 * \param[out]  trace   Pointer to trace* where allocated trace struct is returned.
 * \param[in]   cnt     Number of samples to allocate.
 *                      the defaults of the underlying task will be used.
 *
 * \retval OSAL_OK                      On success.
 * \retval OSAL_ERR_OUT_OF_MEMORY       System out of memory.
 */
osal_retval_t osal_trace_alloc(osal_trace_t **trace, osal_uint32_t cnt);

//! \brief Free trace struct.
/*!
 * \param[in]   trace   Pointer to trace struct to free.
 *
 * \return N/A
 */
void osal_trace_free(osal_trace_t *trace);

//! \brief Trace time.
/*!
 * \param[in]   trace   Pointer to trace struct.
 *
 * \return stored trace time in [ns].
 */
osal_uint64_t osal_trace_point(osal_trace_t *trace);

//! \brief Trace time.
/*!
 * \param[in]   trace   Pointer to trace struct.
 * \param[in]   time    Time to store in trace.
 *
 * \return N/A
 */
void osal_trace_time(osal_trace_t *trace, osal_uint64_t time);

//! \brief Return last time stored in trace.
/*!
 * \param[in]   trace   Pointer to trace struct.
 * 
 * \return last store trace time in [ns].
 */
osal_uint64_t osal_trace_get_last_time(osal_trace_t *trace);

//! \brief Sync to trace when buffer is full.
/*!
 * \param[in]   trace   Pointer to trace struct.
 * \param[in]   timeout Timeout.
 *
 * \retval OSAL_OK          success
 * \retval OSAL_ERR_TIMEOUT timeout occured
 */
osal_retval_t osal_trace_timedwait(osal_trace_t *trace, osal_timer_t *timeout);

//! \brief Analyze trace and return average and jitters.
/*!
 * \param[in]   trace   Pointer to trace struct.
 * \param[out]  avg     Return average time interval.
 * \param[out]  avg_jit Return average jitter (std-dev).
 * \param[out]  max_jit Return maximum jitter.
 *
 * \return N/A
 */
void osal_trace_analyze(osal_trace_t *trace, osal_uint64_t *avg, osal_uint64_t *avg_jit, osal_uint64_t *max_jit);

//! \brief Analyze trace with relative timestamps and return average and jitters.
/*!
 * \param[in]   trace   Pointer to trace struct.
 * \param[out]  avg     Return average time interval.
 * \param[out]  avg_jit Return average jitter (std-dev).
 * \param[out]  max_jit Return maximum jitter.
 *
 * \return N/A
 */
void osal_trace_analyze_rel(osal_trace_t *trace, osal_uint64_t *avg, osal_uint64_t *avg_jit, osal_uint64_t *max_jit);

#ifdef __cplusplus
};
#endif

/** @} */

#endif /* LIBOSAL_TRACE__H */

