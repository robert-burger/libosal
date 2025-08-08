/**
 * \file semaphore.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL semaphore header.
 *
 * OSAL semaphore include header.
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

#ifndef LIBOSAL_SEMAPHORE__H
#define LIBOSAL_SEMAPHORE__H

#include <libosal/osal.h>
#include <libosal/timer.h>

#ifdef LIBOSAL_BUILD_POSIX
#include <libosal/posix/semaphore.h>
#endif

#ifdef LIBOSAL_BUILD_VXWORKS
#include <libosal/vxworks/semaphore.h>
#endif

#ifdef LIBOSAL_BUILD_PIKEOS
#include <libosal/pikeos/semaphore.h>
#endif

#ifdef LIBOSAL_BUILD_WIN32
#include <libosal/win32/semaphore.h>
#endif

#ifdef LIBOSAL_BUILD_STM32
#include <libosal/stm32/semaphore.h>
#endif

/** \defgroup semaphore_group Semaphore
 *
 * The semaphores are a synchronization mechanism for 2 or more task with a
 * counter value.
 *
 * @{
 */

#define OSAL_SEMAPHORE_ATTR__PROCESS_SHARED         0x00000020u     //!< \brief Create a process shared semaphore.

typedef osal_uint32_t osal_semaphore_attr_t;        //!< \brief Semaphore attribute type.

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Initialize a semaphore.
/*!
 * This function initializes a semaphore structure given by \p sem. If passed \p attr 
 * is NULL a default semaphore is initialized. The counter is initilized to \p initval.
 *
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial semaphore attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 * \param[in]   initval Initial semaphore cound value
 *
 * \retval OSAL_OK                      On success.
 * \retval OSAL_ERR_NOT_IMPLEMENTED     Shared was requested but there's not support from OS.
 * \retval OSAL_ERR_INVALID_PARAM       Invalid input parameter.
 */
osal_retval_t osal_semaphore_init(osal_semaphore_t *sem, const osal_semaphore_attr_t *attr, osal_int32_t initval);

//! \brief Post a semaphore.
/*!
 * This function "posts" the semaphore. That means it increments the internal counter value 
 * and unblocks waiting tasks on that semaphores to continue their execution.
 *
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \retval OSAL_OK                      On success.
 * \retval OSAL_ERR_INVALID_PARAM       Invalid input parameter.
 * \retval OSAL_ERR_OPERATION_FAILED    Error occuered posting semaphore e.g. counter is at maximum value.
 */
osal_retval_t osal_semaphore_post(osal_semaphore_t *sem);

//! \brief Wait for a semaphore.
/*!
 * Wait for a semaphore to become available. If the internal counter is already greater than 0
 * it decrements the counter and return OSAL_OK immediately. If the counter is 0 it 
 * will block until some other tasks call \ref osal_semaphore_post or any error
 * occures.
 *
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \retval OSAL_OK                      On success.
 * \retval OSAL_ERR_INTERRUPTED         Call was interrupted by a signal during wait.
 * \retval OSAL_ERR_INVALID_PARAM       Invalid input parameter.
 */
osal_retval_t osal_semaphore_wait(osal_semaphore_t *sem);

//! \brief Try to wait for a semaphore but don't block.
/*!
 * If the semaphore counter is greater than 0 it decrements the counter and returns
 * OSAL_OK immediately. If the counter is 0 it returns OSAL_ERR_BUSY and does not block!
 *
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 *
 * \retval OSAL_OK                      On success.
 * \retval OSAL_ERR_BUSY                Waiting for semaphore would block.
 * \retval OSAL_ERR_OPERATION_FAILED    Other error occuered.
 */
osal_retval_t osal_semaphore_trywait(osal_semaphore_t *sem);

//! \brief Wait for a semaphore.
/*!
 * Wait for a semaphore to become available. If the internal counter is already greater than 0
 * it decrements the counter and return OSAL_OK immediately. If the counter is 0 it 
 * will block until some other tasks call \ref osal_semaphore_post or a timeout or any error
 * occures.
 *
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 * \param[in]   to      Timeout.
 *
 * \retval OSAL_OK                      On success.
 * \retval OSAL_ERR_INTERRUPTED         Call was interrupted by a signal during wait.
 * \retval OSAL_ERR_INVALID_PARAM       Invalid input parameter.
 * \retval OSAL_ERR_TIMEOUT             Timeout occured waiting for semaphore to become available.
 */
osal_retval_t osal_semaphore_timedwait(osal_semaphore_t *sem, const osal_timer_t *to);

//! \brief Destroys a semaphore.
/*!
 * \param[in]   sem     Pointer to osal semaphore structure. Content is OS dependent.
 * \param[in]   to      time value with deadline in absolute time of system real time clock
 *
 * \retval OSAL_OK                      On success.
 * \retval OSAL_ERR_INVALID_PARAM       Invalid input parameter.
 */
osal_retval_t osal_semaphore_destroy(osal_semaphore_t *sem);

#ifdef __cplusplus
};
#endif

/** @} */

#endif /* LIBOSAL_SEMAPHORE__H */

