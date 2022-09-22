/**
 * \file posix/semaphore.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL semaphore posix source.
 *
 * OSAL semaphore posix source.
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

//! \brief Initialize a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial semaphore attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 * \param[in]   initval Initial semaphore cound value
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_semaphore_init(osal_semaphore_t *sem, const osal_semaphore_attr_t *attr, osal_int32_t initval) {
    assert(sem != NULL);

    int pshared = 0;
    int posix_initval = initval;
    if (attr != NULL) {
        if (((*attr) & OSAL_SEMAPHORE_ATTR__PROCESS_SHARED) == OSAL_SEMAPHORE_ATTR__PROCESS_SHARED) {
            pshared = 1;
        }
    }

    sem_init(&sem->posix_sem, pshared, posix_initval);
    return OSAL_OK;
}

//! \brief Post a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_semaphore_post(osal_semaphore_t *sem) {
    assert(sem != NULL);

    sem_post(&sem->posix_sem);

    return OSAL_OK;
}

//! \brief Wait for a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_semaphore_wait(osal_semaphore_t *sem) {
    assert(sem != NULL);

    sem_wait(&sem->posix_sem);

    return OSAL_OK;
}

//! \brief Try to wait for a semaphore but don't block.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_semaphore_trywait(osal_semaphore_t *sem) {
    assert(sem != NULL);

    sem_trywait(&sem->posix_sem);

    return OSAL_OK;
}

//! \brief Wait for a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 * \param[in]   to      Timeout.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_semaphore_timedwait(osal_semaphore_t *sem, osal_timer_t *to) {
    assert(sem != NULL);
    assert(to != NULL);

    osal_retval_t ret = OSAL_OK;

    struct timespec ts;
    ts.tv_sec = to->sec;
    ts.tv_nsec = to->nsec;

    while (ret != OSAL_ERR_TIMEOUT) {
        int local_ret = sem_timedwait(&sem->posix_sem, &ts);
        int local_errno = errno;

        if (local_ret == 0) {
            break;
        } else {
            if (local_errno == ETIMEDOUT) {
                ret = OSAL_ERR_TIMEOUT;
            }
        }
    }

    return ret;
}

//! \brief Destroys a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_semaphore_destroy(osal_semaphore_t *sem) {
    assert(sem != NULL);

    sem_destroy(&sem->posix_sem);
    return OSAL_OK;
}


