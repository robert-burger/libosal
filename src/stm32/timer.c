/**
 * \file timer.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 * \author Marcel Beausencourt <marcel.beausencourt@dlr.de>
 *
 * \date 12 Dec 2024
 *
 * \brief OSAL stm32 timer source.
 *
 * OSAL stm32 timer source.
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
#include <assert.h>

#include "stm32h745xx.h"

// sleep in nanoseconds
void osal_sleep(osal_uint64_t nsec) {
    osal_timer_t timeout;
    osal_timer_init(&timeout, nsec);
    osal_sleep_until(&timeout);
}

// Sleep until timer expired.
osal_retval_t osal_sleep_until(osal_timer_t *timer) {
    assert(timer != NULL);
    osal_retval_t ret = OSAL_OK;

    while (osal_timer_expired(timer) != OSAL_ERR_TIMEOUT)
        ;

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

    DECLARE_CRITICAL_SECTION();
    ENTER_CRITICAL_SECTION();

    timer->sec = TIM4->CNT;
    timer->nsec = (TIM2->CNT) * 5; //TIM2 is working at 200MHz --> 1 clock cycle = 5ns

    LEAVE_CRITICAL_SECTION();

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

    osal_timer_t a;
    osal_timer_t b;

    osal_timer_gettime(&a);
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

