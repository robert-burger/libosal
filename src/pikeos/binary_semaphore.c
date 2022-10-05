/**
 * \file pikeos/binary_semaphore.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL binary semaphore pikeos source.
 *
 * OSAL binary semaphore pikeos source.
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
#include <libosal/binary_semaphore.h>
#include <assert.h>
#include <errno.h>

//! \brief Initialize a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial semaphore attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_init(osal_binary_semaphore_t *sem, const osal_binary_semaphore_attr_t *attr) {
    assert(sem != NULL);

    (void)attr;

    unsigned int start = 0;
    P4_uint32_t flags  = 1;

    // set max count to 1
    p4_sem_init(&sem->pikeos_sem, start, flags);

    return OSAL_OK;
}

//! \brief Post a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_post(osal_binary_semaphore_t *sem) {
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
            case P4_E_NOABILITY: // if the semaphore is shareable (P4_SEM_SHARED is used),#
                                 // but the task of the calling thread does not have the ability
                                 // P4_AB_ULOCK_SHARED enabled.
                ret = OSAL_ERR_OPERATION_FAILED;
                break;
            case P4_E_LIMIT:     // if the semaphore counter is P4_SEM_MAX_COUNT and would overflow
                                 // on further increments.
            default:
                // this is no error in case of binary semaphore
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
osal_retval_t osal_binary_semaphore_wait(osal_binary_semaphore_t *sem) {
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
osal_retval_t osal_binary_semaphore_trywait(osal_binary_semaphore_t *sem) {
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
osal_retval_t osal_binary_semaphore_timedwait(osal_binary_semaphore_t *sem, const osal_timer_t *to) {
    assert(sem != NULL);
    assert(to != NULL);

    osal_retval_t ret = OSAL_OK;
    P4_e_t local_ret;
    P4_timeout_t timeout = 100; // TODO calc timeout

    local_ret = p4_sem_wait(&sem->pikeos_sem, timeout);
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
            case P4_E_PAGEFAULT:    // if \a sem is not fully mapped
                                    // in the caller's virtual address space.
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

//! \brief Destroys a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_destroy(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    // no destroy in pikeos
    (void)sem;
    
    return OSAL_OK;
}


