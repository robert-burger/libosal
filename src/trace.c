/**
 * \file trace.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 02 May 2023
 *
 * \brief OSAL trace source.
 *
 * OSAL trace source.
 */

/*
 * This file is part of libosal.
 *
 * libosal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libosal is distributed in the hope that 
 * it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libosal. If not, see <http://www.gnu.org/licenses/>.
 */

#include <libosal/config.h>
#include <libosal/osal.h>
#include <libosal/trace.h>
#include <assert.h>

#if LIBOSAL_HAVE_MATH_H == 1
#include <math.h>
#endif

//! \brief Analyze trace and return average and jitters.
/*!
 * \param[in]   trace   Pointer to trace struct.
 * \param[out]  avg     Return average time interval.
 * \param[out]  avg_jit Return average jitter (std-dev).
 * \param[out]  max_jit Return maximum jitter.
 *
 * \return N/A
 */
void osal_trace_analyze(osal_trace_t *trace, osal_uint64_t *avg, osal_uint64_t *avg_jit, osal_uint64_t *max_jit) {
    assert(trace != NULL);
    assert(avg != NULL);
    assert(avg_jit != NULL);
    assert(max_jit != NULL);

    (*avg)     = 0u;
    (*avg_jit) = 0u;
    (*max_jit) = 0u;

    int act_buffer = trace->act_buf == 1 ? 0 : 1;

    for (unsigned i = 0; i < (trace->cnt - 1u); ++i) {
        trace->tmp[i] = trace->time_in_ns[act_buffer][i + 1u] - trace->time_in_ns[act_buffer][i];
        (*avg) += trace->tmp[i];
    }

    (*avg) /= trace->cnt;

    for (unsigned i = 0; i < (trace->cnt - 1u); ++i) {
        osal_int64_t dev = (osal_int64_t)(*avg) - trace->tmp[i];
        if (dev < 0) { dev *= -1; }
        if (dev > (*max_jit)) { (*max_jit) = dev; }

        (*avg_jit) += (dev * dev);
    }

    (*avg_jit) = sqrt((*avg_jit) / trace->cnt);
}
