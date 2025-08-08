/**
 * \file vxworks/semaphore.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL semaphore vxworks source.
 *
 * OSAL semaphore vxworks source.
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
int osal_semaphore_init(osal_semaphore_t *sem, const osal_semaphore_attr_t *attr, osal_int32_t initval) {
    assert(sem != NULL);

    (void)attr;

    sem->vx_sem = semCCreate(SEM_Q_FIFO, initval);
    return OSAL_OK;
}

//! \brief Post a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_semaphore_post(osal_semaphore_t *sem) {
    assert(sem != NULL);

    semGive(sem->vx_sem);

    return OSAL_OK;
}

//! \brief Wait for a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_semaphore_wait(osal_semaphore_t *sem) {
    assert(sem != NULL);

    semTake(sem->vx_sem, WAIT_FOREVER);

    return OSAL_OK;
}

//! \brief Try to wait for a semaphore but don't block.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_semaphore_trywait(osal_semaphore_t *sem) {
    assert(sem != NULL);

    semTake(sem->vx_sem, NO_WAIT);

    return OSAL_OK;
}

//! \brief Wait for a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 * \param[in]   to      Timeout.
 *
 * \return OK or ERROR_CODE.
 */
int osal_semaphore_timedwait(osal_semaphore_t *sem, osal_timer_t *to) {
    assert(sem != NULL);
    assert(to != NULL);

    int ret = OSAL_OK;

    // recalc timeout
    int ticks = ((to->sec * 1E3) + (to->nsec / 1E6)) / 60;
    semTake(sem->vx_sem, ticks);

    return ret;
}

//! \brief Destroys a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_semaphore_destroy(osal_semaphore_t *sem) {
    assert(sem != NULL);

    semDelete(sem->vx_sem);
    return OSAL_OK;
}


