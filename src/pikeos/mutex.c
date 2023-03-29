/**
 * \file pikeos/mutex.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL mutex pikeos source.
 *
 * OSAL mutex pikeos source.
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

#include <libosal/mutex.h>
#include <libosal/osal.h>

#include <assert.h>

//! \brief Initialize a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial mutex attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_init(osal_mutex_t *mtx, const osal_mutex_attr_t *attr) {
    assert(mtx != NULL);

    P4_prio_t ceil_prio = 0u;
    P4_uint32_t flags = 0u;
    if (attr != NULL) {
        if ((*attr & OSAL_MUTEX_ATTR__PROCESS_SHARED) != 0u) {
            flags |= P4_MUTEX_SHARED;
        }

        if ((*attr & OSAL_MUTEX_ATTR__ROBUST) != 0u) {
            flags |= P4_MUTEX_ROBUST;
        }

        if ((*attr & OSAL_MUTEX_ATTR__TYPE__MASK) == OSAL_MUTEX_ATTR__TYPE__RECURSIVE) {
            flags |= P4_MUTEX_RECURSIVE;
        }

        if ((*attr & OSAL_MUTEX_ATTR__PROTOCOL__MASK) == OSAL_MUTEX_ATTR__PROTOCOL__INHERIT) {
            flags |= P4_MUTEX_PRIO_INHERIT;
        }

        if ((*attr & OSAL_MUTEX_ATTR__PRIOCEILING__MASK) != 0u) {
            flags |= P4_MUTEX_PRIO_CEILING;
            ceil_prio = (*attr & OSAL_MUTEX_ATTR__PRIOCEILING__MASK) >> OSAL_MUTEX_ATTR__PRIOCEILING__SHIFT;
        }
    }

    osal_retval_t ret = OSAL_OK;

    if (ceil_prio == 0u) {
        p4_mutex_init(&mtx->pikeos_mtx, flags);
    } else {
        p4_mutex_init_ext(&mtx->pikeos_mtx, flags, ceil_prio);
    }

    return ret;
}

//! \brief Locks a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_lock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    osal_retval_t ret = OSAL_OK;
    int local_ret = p4_mutex_lock(&mtx->pikeos_mtx, P4_TIMEOUT_NULL);
    if (local_ret != P4_E_OK) {
        switch (local_ret) {
            case P4_E_STATE:        // if the caller already owns the mutex (recursive 
                                    // locking attempt on non-recursive mutex).
                ret = OSAL_ERR_BUSY;
                break;
            case P4_E_LIMIT:        // if the maximum recursion level P4_MUTEX_MAX_RECURSION 
                                    // is reached. / if the maximum number of robust mutexes 
                                    // is reached.
                ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
                break;
            case P4_E_TIMEOUT:      // if the specified timeout has expired before the lock 
                                    // was acquired by the caller.
                ret = OSAL_ERR_TIMEOUT;
                break;
            case P4_E_BADTIMEOUT:   // if the specified timeout is invalid or in the past.
            case P4_E_BADUID:       // if mutex references invalid waiting threads.
            case P4_E_INVAL:        // if mutex is NULL or does not point to a valid address 
                                    // or exceeds the caller’s virtual address space.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case P4_E_PAGEFAULT:    // if mutex is not fully mapped in the caller’s virtual 
                                    // address space.
            case P4_E_CANCEL:       // if the mutex is cancelable (flag P4_MUTEX_CANCELABLE 
                                    // is set), and the function was canceled by another thread, 
                                    // the calling thread was moved to another time partition, 
                                    // or the thread was migrated to another CPU.
            case P4_E_ABORT:        // if the previous lock owner of the robust mutex mutex died. 
                                    // Note that at most
            case P4_ULOCK_LIMIT:    // threads waiting for a robust mutex are woken up when the 
                                    // lock owner dies.
            case P4_E_NOABILITY:    // if the mutex is shareable (P4_MUTEX_SHARED is used), 
                                    // but the task of the calling thread does not have the 
                                    // ability P4_AB_ULOCK_SHARED enabled.
            default:
                ret = OSAL_ERR_UNAVAILABLE;
                break;
        }
    }

    return ret;
}

//! \brief Tries to lock a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_trylock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    osal_retval_t ret = OSAL_OK;
    int local_ret = p4_mutex_trylock(&mtx->pikeos_mtx);
    if (local_ret != 0) {
    }

    return ret;
}

//! \brief Unlocks a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_unlock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    osal_retval_t ret = OSAL_OK;
    int local_ret = p4_mutex_unlock(&mtx->pikeos_mtx);
    if (local_ret != P4_E_OK) {
        if (local_ret == P4_E_STATE) {  /* specifically check for this error code */
            ret = OSAL_ERR_MUTEX_IS_LOCKED;
        } else {                        /* generic error code */
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    return ret;
}

//! \brief Destroys a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_destroy(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    (void)mtx;

    osal_retval_t ret = OSAL_OK;
    return ret;
}
