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
#include <assert.h>
#include <errno.h>
#include <time.h>

#define timespec_add(tvp, sec, nsec) { \
    (tvp)->tv_nsec += (nsec); \
    (tvp)->tv_sec += (sec); \
    if ((tvp)->tv_nsec > (long int)1E9) { \
        (tvp)->tv_nsec -= (long int)1E9; \
        (tvp)->tv_sec++; } }

//! \brief Initialize a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial semaphore attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_init(osal_binary_semaphore_t *sem, const osal_binary_semaphore_attr_t *attr) {
    assert(sem != NULL);

    (void)attr;

    sem->value = 0;

    pthread_condattr_t cond_attr;
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setclock(&cond_attr, osal_timer_get_clock_source());

    pthread_mutexattr_t posix_attr;
    pthread_mutexattr_setprotocol(&posix_attr, PTHREAD_PRIO_INHERIT);
    pthread_mutex_init(&sem->posix_mtx, &posix_attr);
    pthread_cond_init(&sem->posix_cond, &cond_attr);
    return OSAL_OK;
}

//! \brief Post a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_post(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    pthread_mutex_lock(&sem->posix_mtx);

    if (sem->value == 0) {
        sem->value = 1;
        pthread_cond_signal(&sem->posix_cond);
    }

    pthread_mutex_unlock(&sem->posix_mtx);
    return OSAL_OK;
}

//! \brief Wait for a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_wait(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    pthread_mutex_lock(&sem->posix_mtx);

    while (!sem->value) {
        pthread_cond_wait(&sem->posix_cond, &sem->posix_mtx);
    }

    sem->value = 0;
    
    pthread_mutex_unlock(&sem->posix_mtx);
    return OSAL_OK;
}

//! \brief Wait for a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_trywait(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    osal_retval_t ret = OSAL_OK;

    pthread_mutex_lock(&sem->posix_mtx);

    if (sem->value == 0) {
        ret = OSAL_ERR_BUSY;
    } else {
        sem->value = 0;
    }

    pthread_mutex_unlock(&sem->posix_mtx);
    
    return ret;
}

//! \brief Wait for a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 * \param[in]   to      Timeout.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_timedwait(osal_binary_semaphore_t *sem, const osal_timer_t *to) {
    assert(sem != NULL);

    osal_retval_t ret = OSAL_OK;

    if (to != NULL) {
        struct timespec ts;
        ts.tv_sec = to->sec;
        ts.tv_nsec = to->nsec;

        pthread_mutex_lock(&sem->posix_mtx);
        while (!sem->value) {
            int local_ret = pthread_cond_timedwait(&sem->posix_cond, &sem->posix_mtx, &ts);
            if (local_ret == ETIMEDOUT) {
                ret = OSAL_ERR_TIMEOUT;
                break;
            }
        }

        if (ret == OSAL_OK) {        
            sem->value = 0;
        }

        pthread_mutex_unlock(&sem->posix_mtx);
    } else {
        if (sem->value == 0) {
            ret = OSAL_ERR_TIMEOUT;
        }
    }

    return ret;
}

//! \brief Destroys a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_destroy(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    pthread_mutex_destroy(&sem->posix_mtx);
    pthread_cond_destroy(&sem->posix_cond);

    return OSAL_OK;
}

