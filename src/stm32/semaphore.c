/**
 * \file stm32/semaphore.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL semaphore stm32 source.
 *
 * OSAL semaphore stm32 source.
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

#include <libosal/osal.h>
#include <assert.h>

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

    osal_retval_t ret = OSAL_OK;
    __atomic_store_n(&sem->cnt, initval, __ATOMIC_RELAXED);

    return ret;
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

    (void)__atomic_add_fetch(&sem->cnt, 1, __ATOMIC_RELAXED);

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

    while (__atomic_load_n(&sem->cnt, __ATOMIC_ACQUIRE) == 0) {
        ;
    }

    (void)__atomic_fetch_sub(&sem->cnt, 1, __ATOMIC_RELEASE);

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

    if (__atomic_load_n(&sem->cnt, __ATOMIC_ACQUIRE) > 0) {
        (void)__atomic_fetch_sub(&sem->cnt, 1, __ATOMIC_RELEASE);
	} else {
		ret = OSAL_ERR_UNAVAILABLE;
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

    while (__atomic_load_n(&sem->cnt, __ATOMIC_ACQUIRE) == 0) {
    	if (osal_timer_expired((osal_timer_t *)to) == OSAL_ERR_TIMEOUT) {
    		ret = OSAL_ERR_TIMEOUT;
    		break;
    	}
    }

    if (ret == OSAL_OK) {
        (void)__atomic_fetch_sub(&sem->cnt, 1, __ATOMIC_RELEASE);
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

    osal_retval_t ret = OSAL_OK;

    return ret;
}


