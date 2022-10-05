/**
 * \file condvar.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL posix condvar source.
 *
 * OSAL posix condvar source.
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
#include <assert.h>
#include <errno.h>

#define timespec_add(tvp, sec, nsec) { \
    (tvp)->tv_nsec += (nsec); \
    (tvp)->tv_sec += (sec); \
    if ((tvp)->tv_nsec > (long int)1E9) { \
        (tvp)->tv_nsec -= (long int)1E9; \
        (tvp)->tv_sec++; } }

//! \brief Initialize a condvar.
/*!
 * \param[in]   cv      Pointer to osal condvar structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial condvar attributes. Can be NULL then
 *                      the defaults of the underlying condvar will be used.
 *
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_init(osal_condvar_t *cv, const osal_condvar_attr_t *attr) {
    assert(cv != NULL);

    (void)attr;

    pthread_condattr_t cond_attr;
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setclock(&cond_attr, CLOCK_REALTIME);

    pthread_cond_init(&cv->posix_cond, &cond_attr);
    return OSAL_OK;
}

//! \brief Signals one waiter on a condvar.
/*!
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_signal(osal_condvar_t *cv) {
    assert(cv != NULL);
    pthread_cond_signal(&cv->posix_cond);
    return OSAL_OK;
}

//! \brief Broadcast (Wakes) all waiters on a condvar.
/*!
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_broadcast(osal_condvar_t *cv) {
    assert(cv != NULL);
    pthread_cond_broadcast(&cv->posix_cond);
    return OSAL_OK;
}

//! \brief Wait for a condvar.
/*!
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 * \param[in]   mtx    Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_wait(osal_condvar_t *cv, osal_mutex_t *mtx) {
    assert(cv != NULL);
    pthread_cond_wait(&cv->posix_cond, &mtx->posix_mtx);
    return OSAL_OK;
}

//! \brief Wait for a condvar.
/*!
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 * \param[in]   mtx    Pointer to osal mutex structure. Content is OS dependent.
 * \param[in]   to     Timeout.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_timedwait(osal_condvar_t *cv, osal_mutex_t *mtx, const osal_timer_t *to) {
    assert(cv != NULL);
    assert(mtx != NULL);
    assert(to != NULL);

    osal_retval_t ret = OSAL_OK;
    int local_ret;

    struct timespec ts;
    ts.tv_sec = to->sec;
    ts.tv_nsec = to->nsec;

    do {
        local_ret = pthread_cond_timedwait(&cv->posix_cond, &mtx->posix_mtx, &ts);
        if (local_ret == ETIMEDOUT) {
            ret = OSAL_ERR_TIMEOUT;
            break;
        }
    } while (local_ret != 0);

    return ret;
}

//! \brief Destroys a condvar.
/*!
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_destroy(osal_condvar_t *cv) {
    assert(cv != NULL);

    pthread_cond_destroy(&cv->posix_cond);

    return OSAL_OK;
}


