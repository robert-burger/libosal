/**
 * \file vxworks/mutex.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL mutex vxworks source.
 *
 * OSAL mutex vxworks source.
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

#ifdef HAVE_CONFIG_H
#include <libosal/config.h>
#endif

#include <libosal/osal.h>

#include <pthread.h>
#include <assert.h>

//! \brief Initialize a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial mutex attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 * \return OK or ERROR_CODE.
 */
int osal_mutex_init(osal_mutex_t *mtx, const osal_mutex_attr_t *attr) {
    assert(mtx != NULL);

    (void)attr;

    int ret = OSAL_OK;
    mtx->vxworks_mtx = semMCreate(SEM_Q_FIFO);
    if (mtx->vxworks_mtx == SEM_ID_NULL) {
        ret = OSAL_ERR_OUT_OF_MEMORY;
    }

    return ret;
}

//! \brief Locks a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_mutex_lock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
    int local_ret = semTake(mtx->vxworks_mtx, WAIT_FOREVER);
    if (local_ret != 0) {
        switch (local_ret) {
            default:
#ifdef _WRS_KERNEL
            case S_intLib_NOT_ISR_CALLABLE:
#endif
            case S_objLib_OBJ_UNAVAILABLE:
                ret = OSAL_ERR_UNAVAILABLE;
                break;
            case S_objLib_OBJ_ID_ERROR:
                ret = OSAL_ERR_INVALID_PARAM;
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
int osal_mutex_trylock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
    int local_ret = semTake(mtx->vxworks_mtx, NO_WAIT);
    if (local_ret != 0) {
        switch (local_ret) {
            default:
#ifdef _WRS_KERNEL
            case S_intLib_NOT_ISR_CALLABLE:
#endif
            case S_objLib_OBJ_UNAVAILABLE:
                ret = OSAL_ERR_UNAVAILABLE;
                break;
            case S_objLib_OBJ_ID_ERROR:
                ret = OSAL_ERR_INVALID_PARAM;
                break;
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
int osal_mutex_unlock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
    int local_ret = semGive(mtx->vxworks_mtx);
    if (local_ret != 0) {
        switch (local_ret) {
            default:
#ifdef _WRS_KERNEL
            case S_intLib_NOT_ISR_CALLABLE:
#endif
            case S_objLib_OBJ_UNAVAILABLE:
                ret = OSAL_ERR_UNAVAILABLE;
                break;
            case S_objLib_OBJ_ID_ERROR:
                ret = OSAL_ERR_INVALID_PARAM;
                break;
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
int osal_mutex_destroy(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
    int local_ret = semDelete(mtx->vxworks_mtx);
    if (local_ret != 0) {
        ret = OSAL_ERR_INVALID_PARAM;
    }

    return ret;
}
