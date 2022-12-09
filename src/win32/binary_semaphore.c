/**
 * \file win32/binary_semaphore.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL binary_semaphore win32 source.
 *
 * OSAL binary_semaphore win32 source.
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

//! \brief Initialize a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial binary_semaphore attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 * \param[in]   initval Initial binary_semaphore cound value
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_init(osal_binary_semaphore_t *sem, const osal_binary_semaphore_attr_t *attr) {
    assert(sem != NULL);

    int ret = OSAL_OK;
    LONG win32_initval = 0;
    LONG win32_maxval = 1;

    sem->win32_bs = CreateSemaphoreW(NULL, win32_initval, win32_maxval, NULL);
    if (sem->win32_bs == 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}

//! \brief Post a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_post(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    osal_retval_t ret = OSAL_OK;
    BOOL local_ret;

    local_ret = ReleaseSemaphore(sem->win32_bs, (LONG)1, NULL);
    if (local_ret == 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}

//! \brief Wait for a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_wait(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    osal_retval_t ret = OSAL_OK;
    DWORD local_ret;

    local_ret = WaitForSingleObject(sem->win32_bs, INFINITE);
    if (local_ret != WAIT_OBJECT_0) {
        if (local_ret == WAIT_ABANDONED) {
            ret = OSAL_ERR_OWNER_DEAD;
        } else if (local_ret == WAIT_TIMEOUT) {
            ret = OSAL_ERR_TIMEOUT;
        } else if (local_ret == WAIT_FAILED) {
            ret = OSAL_ERR_OPERATION_FAILED;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    return ret;
}

//! \brief Try to wait for a binary_semaphore but don't block.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_trywait(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    osal_retval_t ret = OSAL_OK;
    DWORD local_ret;

    local_ret = WaitForSingleObject(sem->win32_bs, 0);
    if (local_ret != WAIT_OBJECT_0) {
        if (local_ret == WAIT_ABANDONED) {
            ret = OSAL_ERR_OWNER_DEAD;
        } else if (local_ret == WAIT_TIMEOUT) {
            ret = OSAL_ERR_TIMEOUT;
        } else if (local_ret == WAIT_FAILED) {
            ret = OSAL_ERR_OPERATION_FAILED;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

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
    assert(to != NULL);

    osal_retval_t ret = OSAL_OK;
    DWORD local_ret;

    local_ret = WaitForSingleObject(sem->win32_bs, (DWORD)(to->sec*1000. + to->nsec/1000000.));
    if (local_ret != WAIT_OBJECT_0) {
        if (local_ret == WAIT_ABANDONED) {
            ret = OSAL_ERR_OWNER_DEAD;
        } else if (local_ret == WAIT_TIMEOUT) {
            ret = OSAL_ERR_TIMEOUT;
        } else if (local_ret == WAIT_FAILED) {
            ret = OSAL_ERR_OPERATION_FAILED;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
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

    osal_retval_t ret = OSAL_OK;
    BOOL local_ret;

    local_ret = CloseHandle(sem->win32_bs);
    if (local_ret == 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
    return OSAL_OK;
}


