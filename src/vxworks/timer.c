/**
 * \file timer.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL vxworks timer source.
 *
 * OSAL vxworks timer source.
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

#ifdef HAVE_CONFIG_H
#include <libosal/config.h>
#endif

#include <libosal/osal.h>
#include <libosal/timer.h>

// cppcheck-suppress misra-c2012-21.6
#include <stdio.h>
#include <assert.h>

#include <tickLib.h>

//! Global configuration option for the clock source used by the timer
//! functions.
static int global_clock_id = CLOCK_MONOTONIC;

// sleep in nanoseconds
void osal_sleep(osal_int64_t nsec) {
    struct timespec ts = { (nsec / NSEC_PER_SEC), (nsec % NSEC_PER_SEC) };
    struct timespec rest;
    
    while (1) {
        int ret = nanosleep(&ts, &rest);
        if (ret == 0) {
            break;
        }

        ts = rest;
    }
}

//! Sets globally the internal clock source
void osal_timer_set_clock_source(int clock_id) { global_clock_id = clock_id; }

//! Returns the globally configured internal clock source
int osal_timer_get_clock_source(){
    return global_clock_id;
}

//! gets timer 
int osal_timer_gettime(osal_timer_t *timer) {
    assert(timer != NULL);
    int ret = OSAL_OK;

    struct timespec ts;
    if (clock_gettime(global_clock_id, &ts) == -1) {
        perror("clock_gettime");
        ret = OSAL_ERR_UNAVAILABLE;
    } else {
        timer->sec = ts.tv_sec;
        timer->nsec = ts.tv_nsec;
    }

    return ret;
}

// gets time in nanoseconds
osal_int64_t osal_timer_gettime_nsec(void) {
    osal_int64_t ret = 0;
    osal_timer_t tmr = { 0, 0 };
    int local_ret = osal_timer_gettime(&tmr);

    if (local_ret == OSAL_OK) {
        ret = ((tmr.sec * 1E9) + tmr.nsec);
    }

    return ret;
}

// initialize timer with timeout 
void osal_timer_init(osal_timer_t *timer, osal_int64_t timeout) {
    assert(timer != NULL);

    struct timespec ts;
    if (clock_gettime(global_clock_id, &ts) == -1) {
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
int osal_timer_expired(osal_timer_t *timer) {
    assert(timer != NULL);

    osal_timer_t act = { 0, 0 };
    int ret = OSAL_OK;
    ret = osal_timer_gettime(&act);    

    if (ret == OSAL_OK) {
        if (osal_timer_cmp(&act, timer, <) == 0) {
            ret = OSAL_ERR_TIMEOUT;
        }
    } 

    return ret;
}

