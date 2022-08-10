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

#ifndef LIBOSAL_BINARY_SEMAPHORE__H
#define LIBOSAL_BINARY_SEMAPHORE__H

#include <libosal/config.h>
#include <libosal/types.h>
#include <libosal/timer.h>

#ifdef LIBOSAL_BUILD_POSIX
#include <libosal/posix/binary_semaphore.h>
#endif

#define OSAL_BINARY_SEMAPHORE_ATTR__PROCESS_SHARED         0x00000020u

typedef osal_uint32_t osal_binary_semaphore_attr_t;

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Initialize a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial semaphore attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 *
 * \return OK or ERROR_CODE.
 */
int osal_binary_semaphore_init(osal_binary_semaphore_t *sem, osal_binary_semaphore_attr_t *attr);

//! \brief Post a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_binary_semaphore_post(osal_binary_semaphore_t *sem);

//! \brief Wait for a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_binary_semaphore_wait(osal_binary_semaphore_t *sem);

//! \brief Wait for a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 * \param[in]   to      Timeout.
 *
 * \return OK or ERROR_CODE.
 */
int osal_binary_semaphore_timedwait(osal_binary_semaphore_t *sem, osal_timer_t *to);

//! \brief Destroys a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_binary_semaphore_destroy(osal_binary_semaphore_t *sem);

#ifdef __cplusplus
};
#endif

#endif /* LIBOSAL_BINARY_SEMAPHORE__H */

