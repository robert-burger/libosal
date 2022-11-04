/**
 * \file condvar.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL pikeos condvar source.
 *
 * OSAL pikeos condvar source.
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

#include <libosal/condvar.h>
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

    P4_uint32_t flags = 0u;
    if (attr != NULL) {
        if ((*attr & OSAL_BINARY_SEMAPHORE_ATTR__PROCESS_SHARED) != 0u) {
            flags |= P4_COND_SHARED;
        }
    }

    p4_cond_init(&cv->pikeos_cond, flags);

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
    
    osal_retval_t ret = OSAL_OK;
    P4_e_t result;

    result = p4_cond_wake(&cv->pikeos_cond, 0);

    if (result != P4_E_OK) {
        if (result == P4_E_INVAL) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else if (result == P4_E_NOABILITY) {
            ret = OSAL_ERR_PERMISSION_DENIED;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    return ret;
}

//! \brief Broadcast (Wakes) all waiters on a condvar.
/*!
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_broadcast(osal_condvar_t *cv) {
    assert(cv != NULL);

    osal_retval_t ret = OSAL_OK;
    P4_e_t result;

    result = p4_cond_broadcast(&cv->pikeos_cond);

    if (result != P4_E_OK) {
        if (result == P4_E_INVAL) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else if (result == P4_E_NOABILITY) {
            ret = OSAL_ERR_PERMISSION_DENIED;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    return ret;
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
    assert(mtx != NULL);

    osal_retval_t ret = OSAL_OK;
    P4_e_t result;
    
    result = p4_cond_wait(&cv->pikeos_cond, &mtx->pikeos_mtx, P4_TIMEOUT_INFINITE);

    if (result != P4_E_OK) {
        if (result == P4_E_STATE) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }
    return ret;
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
    P4_e_t result;
    P4_time_t timeout = to->sec * 1E9 + to->nsec;

    result = p4_cond_wait(&cv->pikeos_cond, &mtx->pikeos_mtx, P4_TIMEOUT_ABS(timeout));

    if (result != P4_E_OK) {
        if (result == P4_E_STATE) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else if (result == P4_E_TIMEOUT) {
            ret = OSAL_ERR_TIMEOUT;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }
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

    // there is no destroy on pikeos
    (void)cv;

    return OSAL_OK;
}


