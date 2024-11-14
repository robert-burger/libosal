/**
 * \file binary_semaphore.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL binary_semaphore header.
 *
 * OSAL binary_semaphore include header.
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

#ifndef LIBOSAL_BINARY_SEMAPHORE__H
#define LIBOSAL_BINARY_SEMAPHORE__H

#ifdef HAVE_CONFIG_H
#include <libosal/config.h>
#endif
#include <libosal/types.h>
#include <libosal/timer.h>

#ifdef LIBOSAL_BUILD_POSIX
#include <libosal/posix/binary_semaphore.h>
#endif

#ifdef LIBOSAL_BUILD_VXWORKS
#include <libosal/vxworks/binary_semaphore.h>
#endif

#ifdef LIBOSAL_BUILD_PIKEOS
#include <libosal/pikeos/binary_semaphore.h>
#endif

#ifdef LIBOSAL_BUILD_WIN32
#include <libosal/win32/binary_semaphore.h>
#endif

#ifdef LIBOSAL_BUILD_STM32
#include <libosal/stm32/binary_semaphore.h>
#endif

/** \defgroup binary_semaphore_group Binary Semaphore
 * The binary semaphores are a special case of semaphores. They do not 
 * have a counter and are used for a single signal event between two 
 * tasks or processes.
 *
 * @{
 */

//! Flag to make a process shared binary semaphore.
#define OSAL_BINARY_SEMAPHORE_ATTR__PROCESS_SHARED         0x00000020u

//! Binary semaphore attribute type.
typedef osal_uint32_t osal_binary_semaphore_attr_t;

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Initialize a binary_semaphore.
/*!
 * This function initializes a binary semaphore structure. Creation attributes can
 * be passed optionally.
 *
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial semaphore attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_init(osal_binary_semaphore_t *sem, const osal_binary_semaphore_attr_t *attr);

//! \brief Post a binary_semaphore.
/*!
 * This function 'posts' a binary semaphore. The state of the binary semaphore is preserved 
 * until any other task calls \ref osal_binary_semaphore_wait or \ref osal_binary_semaphore_trywait.
 *
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_post(osal_binary_semaphore_t *sem);

//! \brief Wait for a binary_semaphore. (blocking)
/*!
 * This function waits on a binary semaphore for some other task to call 
 * \ref osal_binary_semaphore_post. If \ref osal_binary_semaphore_post was called before
 * the state of the binary semaphore is updated and it is immediately returned.
 *
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_binary_semaphore_wait(osal_binary_semaphore_t *sem);

//! \brief Wait for a binary_semaphore.
/*!
 * This function tries to wait on a binary semaphore. That means it checks if the state of
 * the binary semaphore is already set and a call to \ref osal_binary_semaphore_wait would
 * not block.
 *
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \retval OK               on success.
 * \retval OSAL_ERR_TIMEOUT if a call of \ref osal_binary_semaphore_wait would block.
 */
osal_retval_t osal_binary_semaphore_trywait(osal_binary_semaphore_t *sem);

//! \brief Wait for a binary_semaphore.
/*!
 * This functions waits on a binary semaphore. If the binary semaphore state is not set it 
 * waits until someone calls \ref osal_binary_semaphore_post or the specified timeout \p to 
 * occures. If the state was already set it updates the state and returns immediately.
 *
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 * \param[in]   to      Timeout.
 *
 * \retval OK               on success.
 * \retval OSAL_ERR_TIMEOUT if there was no \ref osal_binary_semaphore_post in the specified timeout.
 */
osal_retval_t osal_binary_semaphore_timedwait(osal_binary_semaphore_t *sem, const osal_timer_t *to);

//! \brief Destroys a binary_semaphore.
/*!
 * This function destroys the binary semaphore and frees operating system resources.
 *
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \retval OK               on success.
 */
osal_retval_t osal_binary_semaphore_destroy(osal_binary_semaphore_t *sem);

#ifdef __cplusplus
};
#endif

/** @} */

#endif /* LIBOSAL_BINARY_SEMAPHORE__H */

