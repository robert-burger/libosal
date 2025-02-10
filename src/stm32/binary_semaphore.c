/**
 * \file timer.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL posix timer source.
 *
 * OSAL posix timer source.
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

//! \brief Initialize a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial semaphore attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_init(osal_binary_semaphore_t *sem, const osal_binary_semaphore_attr_t *attr) {
    assert(sem != NULL);

    (void)attr;
    __atomic_clear(&sem->value, __ATOMIC_RELAXED);

    return OSAL_OK;
}

//! \brief Post a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_post(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    DECLARE_CRITICAL_SECTION();
    ENTER_CRITICAL_SECTION();

    __atomic_test_and_set(&sem->value, __ATOMIC_ACQUIRE);

    LEAVE_CRITICAL_SECTION();

    return OSAL_OK;
}

//! \brief Wait for a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_wait(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    DECLARE_CRITICAL_SECTION();
    ENTER_CRITICAL_SECTION();

    while (__atomic_exchange_n(&sem->value, 0, __ATOMIC_RELAXED) == 0) {
        ;
    }

    LEAVE_CRITICAL_SECTION();

    return OSAL_OK;
}

//! \brief Wait for a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_trywait(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    osal_retval_t ret = OSAL_OK;

    DECLARE_CRITICAL_SECTION();
    ENTER_CRITICAL_SECTION();

    int old_value = __atomic_exchange_n(&sem->value, 0, __ATOMIC_RELAXED);

    LEAVE_CRITICAL_SECTION();

    if (old_value == 0) {
        ret = OSAL_ERR_BUSY;
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

    osal_retval_t ret = OSAL_OK;
    int old_value;

    do {
        if (osal_timer_expired((osal_timer_t *)to) == OSAL_ERR_TIMEOUT) {
            ret = OSAL_ERR_TIMEOUT;
            break;
        }

        DECLARE_CRITICAL_SECTION();
        ENTER_CRITICAL_SECTION();

        old_value = __atomic_exchange_n(&sem->value, 0, __ATOMIC_RELAXED);

        LEAVE_CRITICAL_SECTION();
    } while (old_value == 0);

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

    return OSAL_OK;
}

