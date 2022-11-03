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

#include <libosal/osal.h>
#include <libosal/timer.h>

// cppcheck-suppress misra-c2012-21.6
#include <stdio.h>
// cppcheck-suppress misra-c2012-21.10
#include <assert.h>

// sleep in nanoseconds
void osal_sleep(osal_int64_t nsec) {
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

// gets time in nanoseconds
osal_int64_t osal_timer_gettime_nsec(void) {
    osal_int64_t ret = p4_get_time();

    return ret;
}

// initialize timer with timeout 
void osal_timer_init(osal_timer_t *timer, osal_int64_t timeout) {
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


