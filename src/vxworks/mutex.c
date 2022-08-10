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

    int ret = OSAL_OK;
    mtx->vx_mtx = semMCreate(SEM_Q_FIFO);
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
    semTake(mtx->vx_mtx, WAIT_FOREVER);
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
    semTake(mtx->vx_mtx, NO_WAIT);
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
    semGive(mtx->vx_mtx);
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
    semDelete(mtx->vx_mtx);
    return ret;
}
