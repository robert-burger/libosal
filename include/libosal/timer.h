/**
 * \file timer.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL timer header.
 *
 * OSAL timer include header.
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

#ifndef LIBOSAL_TIMER__H
#define LIBOSAL_TIMER__H

#include <libosal/types.h>

#define NSEC_PER_SEC                1000000000

//! timer structure
typedef struct osal_timer {
    osal_uint64_t sec;       //!< seconds
    osal_uint64_t nsec;      //!< nanoseconds
} osal_timer_t;

#define osal_timer_add(a, b, result)                                \
    do {                                                            \
        (result)->sec = (a)->sec + (b)->sec;                        \
        (result)->nsec = (a)->nsec + (b)->nsec;                     \
        if ((result)->nsec >= 1E9)                                  \
        {                                                           \
            ++(result)->sec;                                        \
            (result)->nsec -= 1E9;                                  \
        }                                                           \
    } while (0)

#define osal_timer_add_nsec(a, n, result)                           \
    do {                                                            \
        (result)->sec = (a)->sec;                                   \
        (result)->nsec = (a)->nsec + (n);                           \
        if ((result)->nsec >= 1E9)                                  \
        {                                                           \
            ++(result)->sec;                                        \
            (result)->nsec -= 1E9;                                  \
        }                                                           \
    } while (0)

#define osal_timer_cmp(a, b, CMP)                                   \
    (((a)->sec == (b)->sec) ?                                       \
     ((a)->nsec CMP (b)->nsec) :                                    \
     ((a)->sec CMP (b)->sec))

#ifdef __cplusplus
extern "C" {
#endif

//! Sleep in nanoseconds
/*!
 * \param[in] nsec      Time to sleep in nanoseconds.
 */
void osal_sleep(osal_uint64_t nsec);

//! Sleep until timer expired
/*!
 * \param[in]   timer   Pointer to timer struct with absolute end time.
 *
 * \retval OSAL_OK      On success.
 */
osal_retval_t osal_sleep_until(osal_timer_t *timer);

//! Sleep until current time equals nsec value expired
/*!
 * \param[in]   nsec   Absolute time in [ns].
 *
 * \retval OSAL_OK      On success.
 */
osal_retval_t osal_sleep_until_nsec(osal_uint64_t nsec);

//! Gets filled timer struct with current time.
/*!
 * \param[out] timer    Pointer to timer struct which will be initialized
 *                      with current time.
 *
 * \retval OSAL_OK                  On success.
 * \retval OSAL_ERR_UNAVAILABLE     On error and errno set.
 */
osal_retval_t osal_timer_gettime(osal_timer_t *timer);

//! Gets time in nanoseconds.
/*!
 * \return              Current timer in nanosecond.
 */
osal_uint64_t osal_timer_gettime_nsec(void);

//! Initialize timer with timeout.
/*!
 * \param[out] timer    Pointer to timer struct which will be initialized
 *                      with current time plus an optional \p timeout.
 * \param[in] timeout   Timeout in nanoseconds. If set to 0, then this function
 *                      will do the same as \link osal_timer_gettime \endlink.
 */
void osal_timer_init(osal_timer_t *timer, osal_uint64_t timeout);

//! Checks if timer is expired.
/*!
 * \param[out] timer    Timer to check if it is expired.
 *
 * \retval OSAL_ERR_TIMEOUT     If \p timer is expired
 * \retval OSAL_OK              If \p timer is not expired
 */
osal_retval_t osal_timer_expired(osal_timer_t *timer);

#ifdef __cplusplus
};
#endif

#endif /* LIBOSAL_TIMER__H */

