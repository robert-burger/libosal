/**
 * \file pikeos/semaphore.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL semaphore pikeos source.
 *
 * OSAL semaphore pikeos source.
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

    (void)attr;
    (void)initval;

    p4_sem_init(&sem->pikeos_sem, 0, 0);

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

    osal_retval_t ret = OSAL_OK;
    P4_e_t local_ret;

    local_ret = p4_sem_post(&sem->pikeos_sem);
    if (local_ret != P4_E_OK) {
        switch (local_ret) {
            case P4_E_INVAL:     // if \a sem is NULL or does not point
                                 // to a valid address or exceeds the caller's virtual address space.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case P4_E_PAGEFAULT: // if \a sem is not fully mapped
                                 // in the caller's virtual address space.
            case P4_E_BADUID:    // if \a sem references invalid waiting threads.
            case P4_E_LIMIT:     // if the semaphore counter is P4_SEM_MAX_COUNT and would overflow
                                 // on further increments.
            case P4_E_NOABILITY: // if the semaphore is shareable (P4_SEM_SHARED is used),#
                                 // but the task of the calling thread does not have the ability
                                 // P4_AB_ULOCK_SHARED enabled.
            default:
                ret = OSAL_ERR_OPERATION_FAILED;
                break;

        }
    }

    return ret;
}

//! \brief Wait for a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_semaphore_wait(osal_semaphore_t *sem) {
    assert(sem != NULL);

    osal_retval_t ret = OSAL_OK;
    P4_e_t local_ret;

    local_ret = p4_sem_wait(&sem->pikeos_sem, P4_TIMEOUT_INFINITE);
    if (local_ret != P4_E_OK) {
        switch (local_ret) {
            case P4_E_BADTIMEOUT:   // if the specified timeout is invalid or in the past.
            case P4_E_TIMEOUT:      // if the specified timeout has expired before the lock was acquired
                                    // by the caller.
            case P4_E_INVAL:        // if \a sem is NULL or does not point
                                    // to a valid address or exceeds the caller's virtual address space.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case P4_E_LIMIT:        // if the caller could not acquire \a sem and the number of waiting
                                    // threads would exceed P4_SEM_MAX_COUNT.
            case P4_E_PAGEFAULT:    // if \a sem is not fully mapped in the caller's virtual address space.
            case P4_E_BADUID:       // if \a sem references invalid waiting threads.
            case P4_E_CANCEL:       // if the function was canceled by another thread, the calling thread
                                    // was moved to another time partition, or the thread was migrated to
                                    // another CPU.
            case P4_E_NOABILITY:    // if the semaphore is shareable (P4_SEM_SHARED is used),
                                    // but the task of the calling thread does not have the ability
                                    // P4_AB_ULOCK_SHARED enabled.
            default:
                ret = OSAL_ERR_OPERATION_FAILED;
                break;
        }
    }

    return ret;
}

//! \brief Try to wait for a semaphore but don't block.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_semaphore_trywait(osal_semaphore_t *sem) {
    assert(sem != NULL);

    osal_retval_t ret = OSAL_OK;
    P4_e_t local_ret;

    local_ret = p4_sem_trywait(&sem->pikeos_sem);
    if (local_ret != P4_E_OK) {
        ret = OSAL_ERR_BUSY;
    }

    return ret;
}

//! \brief Wait for a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 * \param[in]   to      Timeout.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_semaphore_timedwait(osal_semaphore_t *sem, const osal_timer_t *to) {
    assert(sem != NULL);
    assert(to != NULL);

    osal_retval_t ret = OSAL_OK;
    P4_e_t local_ret;
    P4_time_t timeout = to->sec * 1E9 + to->nsec;

    local_ret = p4_sem_wait(&sem->pikeos_sem, P4_TIMEOUT_ABS(timeout));
    if (local_ret != P4_E_OK) {
        switch (local_ret) {
            case P4_E_TIMEOUT:      // if the specified timeout has expired before the lock was acquired
                                    // by the caller.
                ret = OSAL_ERR_TIMEOUT;
                break;
            case P4_E_BADTIMEOUT:   // if the specified timeout is invalid or in the past.
            case P4_E_INVAL:        // if \a sem is NULL or does not point
                                    // to a valid address or exceeds the caller's virtual address space.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case P4_E_LIMIT:        // if the caller could not acquire \a sem and the number of waiting
                                    // threads would exceed P4_SEM_MAX_COUNT.
                ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
                break;
            case P4_E_NOABILITY:    // if the semaphore is shareable (P4_SEM_SHARED is used),
                                    // but the task of the calling thread does not have the ability
                                    // P4_AB_ULOCK_SHARED enabled.
                ret = OSAL_ERR_PERMISSION_DENIED;
                break;
            case P4_E_PAGEFAULT:    // if \a sem is not fully mapped
                                    // in the caller's virtual address space.
            case P4_E_BADUID:       // if \a sem references invalid waiting threads.
            case P4_E_CANCEL:       // if the function was canceled by another thread, the calling thread
                                    // was moved to another time partition, or the thread was migrated to
                                    // another CPU.
            default:
                ret = OSAL_ERR_OPERATION_FAILED;
                break;
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

    // no destroy in pikeos
    (void)sem; 

    return OSAL_OK;
}


