/**
 * \file win32/mutex.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL mutex win32 source.
 *
 * OSAL mutex win32 source.
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
#include <libosal/mutex.h>

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

    osal_retval_t ret = OSAL_OK;

    SECURITY_ATTRIBUTES sec_attr;
    sec_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
    sec_attr.lpSecurityDescriptor = NULL;
    sec_attr.bInheritHandle = FALSE;

    mtx->win32_mtx = CreateMutexA(&sec_attr, FALSE, NULL);

    if (mtx->win32_mtx == NULL) {
        //DWORD last_error = GetLastError();
        ret = OSAL_ERR_OPERATION_FAILED;
    } else {
        ret = OSAL_OK;
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
    DWORD local_ret;

    local_ret = WaitForSingleObject(mtx->win32_mtx, INFINITE);
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

//! \brief Tries to lock a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_trylock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    osal_retval_t ret = OSAL_OK;
    DWORD local_ret;

    local_ret = WaitForSingleObject(mtx->win32_mtx, 0);
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

//! \brief Unlocks a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_unlock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    osal_retval_t ret = OSAL_OK;
    BOOL local_ret;

    local_ret = ReleaseMutex(mtx->win32_mtx);
    if (local_ret == 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
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

    osal_retval_t ret = OSAL_OK;
    BOOL local_ret;

    local_ret = CloseHandle(mtx->win32_mtx);
    if (local_ret == 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}
