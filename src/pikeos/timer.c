/**
 * \file timer.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL pikeos timer source.
 *
 * OSAL pikeos timer source.
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

#include <libosal/osal.h>
#include <libosal/timer.h>

// cppcheck-suppress misra-c2012-21.6
#include <stdio.h>
// cppcheck-suppress misra-c2012-21.10
#include <assert.h>

// sleep in nanoseconds
void osal_sleep(osal_uint64_t nsec) {
    p4_sleep(P4_NSEC(nsec));
}

//! gets timer 
osal_retval_t osal_timer_gettime(osal_timer_t *timer) {
    assert(timer != NULL);
    osal_retval_t ret = OSAL_OK;

    osal_uint64_t local_time;
    local_time = p4_get_time();

    timer->sec = local_time / (osal_uint64_t)1E9;
    timer->nsec = local_time % (osal_uint64_t)1E9;

    return ret;
}

//! Sets globally the internal clock source. Unused for Pikeos
void osal_timer_set_clock_source(int clock_id){}

//! Returns the globally configured internal clock source. Unused for Pikeos
int osal_timer_get_clock_source(){
    return -1;
}

// gets time in nanoseconds
osal_uint64_t osal_timer_gettime_nsec(void) {
    osal_uint64_t ret = p4_get_time();

    return ret;
}

// initialize timer with timeout 
void osal_timer_init(osal_timer_t *timer, osal_uint64_t timeout) {
    assert(timer != NULL);

    osal_uint64_t local_time;
    local_time = p4_get_time();

    osal_timer_t a;
    osal_timer_t b;
    a.sec = local_time / (osal_uint64_t)1E9;
    a.nsec = local_time % (osal_uint64_t)1E9;

    b.sec = (timeout / NSEC_PER_SEC);
    b.nsec = (timeout % NSEC_PER_SEC);

    osal_timer_add(&a, &b, timer);
}

// checks if timer is expired
osal_retval_t osal_timer_expired(osal_timer_t *timer) {
    assert(timer != NULL);

    osal_timer_t act = { 0, 0 };
    osal_retval_t ret = OSAL_OK;
    ret = osal_timer_gettime(&act);    

    if (ret == OSAL_OK) {
        if (osal_timer_cmp(&act, timer, <) == 0) {
            ret = OSAL_ERR_TIMEOUT;
        }
    } 

    return ret;
}

// Sleep until timer expired.
osal_retval_t osal_sleep_until(osal_timer_t *timer) {
    uint64_t abs_time = timer->sec * 1E9 + timer->nsec;
    return osal_sleep_until_nsec(abs_time);
}

// Sleep until current time equals nsec value expired
osal_retval_t osal_sleep_until_nsec(osal_uint64_t nsec) {
    osal_retval_t ret = OSAL_OK;
    P4_timeout_t to = P4_TIMEOUT_ABS(nsec);

    while (1) {
        P4_e_t local_ret = p4_sleep(to);

        if (local_ret == P4_E_OK) {
            break; // time reached
        }

        if (local_ret == P4_E_BADTIMEOUT) {
            ret = OSAL_ERR_OPERATION_FAILED;
            break;
        }

        // P4_E_CANCEL
        // in this case try to sleep further on ...
    }

    return ret;
}

