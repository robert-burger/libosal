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
#include <time.h>
#include <assert.h>
#include <errno.h>

/**
 * set_normalized_timespec - set timespec sec and nsec parts and normalize
 *
 * @ts:		pointer to timespec variable to be set
 * @sec:	seconds to set
 * @nsec:	nanoseconds to set
 *
 * Set seconds and nanoseconds field of a timespec variable and
 * normalize to the timespec storage format
 *
 * Note: The tv_nsec part is always in the range of
 *	0 <= tv_nsec < NSEC_PER_SEC
 * For negative values only the tv_sec field is negative !
 */
#define NSEC_PER_SEC 1000000000
void set_normalized_timespec(struct timespec *ts, time_t sec, int64_t nsec)
{
    while (nsec >= NSEC_PER_SEC) {
        /*
         * The following asm() prevents the compiler from
         * optimising this loop into a modulo operation. See
         * also __iter_div_u64_rem() in include/linux/time.h
         */
        asm("" : "+rm"(nsec));
        nsec -= NSEC_PER_SEC;
        ++sec;
    }
    while (nsec < 0) {
        asm("" : "+rm"(nsec));
        nsec += NSEC_PER_SEC;
        --sec;
    }
    ts->tv_sec = sec;
    ts->tv_nsec = nsec;
}

static inline struct timespec timespec_sub(struct timespec a, struct timespec b) {
    struct timespec ret;
    set_normalized_timespec(&ret, a.tv_sec - b.tv_sec, a.tv_nsec - b.tv_nsec);

    return ret;
}

// sleep in nanoseconds
void osal_sleep(osal_uint64_t nsec) {
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

// Sleep until timer expired.
osal_retval_t osal_sleep_until(osal_timer_t *timer) {
    assert(timer != NULL);
    osal_retval_t ret = OSAL_OK;
    int local_ret;

    struct timespec ts_end = { timer->sec, timer->nsec }, ts_now, ts_diff, ts_rem;

    clock_gettime(CLOCK_REALTIME, &ts_now);
    ts_diff = timespec_sub(ts_end, ts_now);

    do {
        local_ret = nanosleep(&ts_diff, &ts_rem);
        ts_diff = ts_rem;
    } while (local_ret == EINTR);

    if (local_ret != 0) {
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
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
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
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
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

