/**
 * \file timer.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL posix timer source.
 *
 * OSAL posix timer source.
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
#include <time.h>
#include <assert.h>
#include <errno.h>

// sleep in nanoseconds
void osal_sleep(osal_uint64_t nsec) {
    struct timespec ts = { (nsec / NSEC_PER_SEC), (nsec % NSEC_PER_SEC) };
    struct timespec rest;
    
    while (1) {
        int ret = clock_nanosleep(LIBOSAL_CLOCK, 0, &ts, &rest);
        if (ret == 0) {
            break;
        }

        ts = rest;
    }
}

// Sleep until timer expired.
osal_retval_t osal_sleep_until(osal_timer_t *timer) {
    assert(timer != NULL);
    osal_retval_t ret = OSAL_OK;
    int local_ret;

    struct timespec ts = { timer->sec, timer->nsec };

    do {
        local_ret = clock_nanosleep(LIBOSAL_CLOCK, TIMER_ABSTIME, &ts, NULL);
    } while (local_ret == EINTR);

    if (local_ret == EINVAL) {
        ret = OSAL_ERR_INVALID_PARAM;
    } else if (local_ret != 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}

//! Sleep until current time equals nsec value expired
osal_retval_t osal_sleep_until_nsec(osal_uint64_t nsec) {
    osal_timer_t abs_to;
    abs_to.sec = nsec / NSEC_PER_SEC;
    abs_to.nsec = nsec % NSEC_PER_SEC;
    return osal_sleep_until(&abs_to);
}

//! gets timer 
osal_retval_t osal_timer_gettime(osal_timer_t *timer) {
    assert(timer != NULL);
    osal_retval_t ret = OSAL_OK;

    struct timespec ts;
    if (clock_gettime(LIBOSAL_CLOCK, &ts) == -1) {
        perror("clock_gettime");
        ret = OSAL_ERR_UNAVAILABLE;
    } else {
        timer->sec = ts.tv_sec;
        timer->nsec = ts.tv_nsec;
    }

    return ret;
}

// gets time in nanoseconds
osal_uint64_t osal_timer_gettime_nsec(void) {
    osal_uint64_t ret = 0;
    osal_timer_t tmr = { 0, 0 };
    int local_ret = osal_timer_gettime(&tmr);

    if (local_ret == OSAL_OK) {
        ret = ((tmr.sec * NSEC_PER_SEC) + tmr.nsec);
    }

    return ret;
}

// initialize timer with timeout 
void osal_timer_init(osal_timer_t *timer, osal_uint64_t timeout) {
    assert(timer != NULL);

    struct timespec ts;
    if (clock_gettime(LIBOSAL_CLOCK, &ts) == -1) {
        perror("clock_gettime");
    }

    osal_timer_t a;
    osal_timer_t b;
    a.sec = ts.tv_sec;
    a.nsec = ts.tv_nsec;

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

