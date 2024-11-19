/**
 * \file mutex.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 * \author Marcel Beausencourt <marcel.beausencourt@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL mutex header.
 *
 * OSAL mutex include header.
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

#ifndef LIBOSAL_MUTEX__H
#define LIBOSAL_MUTEX__H

#ifdef HAVE_CONFIG_H
#include <libosal/config.h>
#endif

#include <libosal/types.h>

#ifdef LIBOSAL_BUILD_POSIX
#include <libosal/posix/mutex.h>
#endif

#ifdef LIBOSAL_BUILD_VXWORKS
#include <libosal/vxworks/mutex.h>
#endif

#ifdef LIBOSAL_BUILD_PIKEOS
#include <libosal/pikeos/mutex.h>
#endif

#ifdef LIBOSAL_BUILD_WIN32
#include <libosal/win32/mutex.h>
#endif

#ifdef LIBOSAL_BUILD_STM32
#include <libosal/stm32/mutex.h>
#endif

/** \defgroup mutex_group Mutex
 * The mutexes are mutual exclusion locks which are commonly used to protect 
 * shared memory structures from concurrent access.
 *
 * @{
 */

#define OSAL_MUTEX_ATTR__TYPE__MASK             0x00000003u     //!< \brief Mutex attribute type mask.
#define OSAL_MUTEX_ATTR__TYPE__NORMAL           0x00000000u     //!< \brief Mutex normal (default) type.
#define OSAL_MUTEX_ATTR__TYPE__ERRORCHECK       0x00000001u     //!< \brief Mutex with error checks.
#define OSAL_MUTEX_ATTR__TYPE__RECURSIVE        0x00000002u     //!< \brief Mutex avoiding recursive deadlocks.

#define OSAL_MUTEX_ATTR__ROBUST                 0x00000010u     //!< \brief Robust mutex (unlocks if owner died)
#define OSAL_MUTEX_ATTR__PROCESS_SHARED         0x00000020u     //!< \brief Process shared mutex.

#define OSAL_MUTEX_ATTR__PROTOCOL__MASK         0x00000300u     //!< \brief Mutex protocol mask.
#define OSAL_MUTEX_ATTR__PROTOCOL__NONE         0x00000000u     //!< \brief Mutex protocol default.
#define OSAL_MUTEX_ATTR__PROTOCOL__INHERIT      0x00000100u     //!< \brief Mutex protocol inherit priority.
#define OSAL_MUTEX_ATTR__PROTOCOL__PROTECT      0x00000200u     //!< \brief Mutex protocol protect priority.

#define OSAL_MUTEX_ATTR__PRIOCEILING__MASK      0xFFFF0000u     //!< \brief Priority ceiling mask.
#define OSAL_MUTEX_ATTR__PRIOCEILING__SHIFT     16u             //!< \brief Priority ceiling value.

typedef osal_uint32_t osal_mutex_attr_t;                        //!< \brief Mutex attribute type.

#ifdef __cplusplus
extern "C" {
#endif
 
//! \brief Initialize a mutex.
/*!
 * This function initializes a mutex structure given by \p mtx. If no attributes
 * are given with \p attr a default mutex is initiazed.
 *
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial mutex attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_SYSTEM_LIMIT_REACHED    Not enough system resources.
 * \retval OSAL_ERR_OUT_OF_MEMORY           System is out of memory.
 * \retval OSAL_ERR_PERMISSION_DENIED       Permission denied opening a shared mutex.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 * \retval OSAL_ERR_UNAVAILABLE             Other errors. 
 */
osal_retval_t osal_mutex_init(osal_mutex_t *mtx, const osal_mutex_attr_t *attr);

//! \brief Locks a mutex.
/*!
 * This function tries to lock a mutex. If the mutex is already locked by another
 * task it blocks until the other task unlocks the mutex or an other error occures.
 *
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_SYSTEM_LIMIT_REACHED    Not enough system resources.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input paratemer.
 * \retval OSAL_ERR_NOT_RECOVERABLE         Mutex not recoverable.
 * \retval OSAL_ERR_OWNER_DEAD              Old mutex owner dead (see ROBUST).
 * \retval OSAL_ERR_DEAD_LOCK               Would dead-lock (see RECURSIVE).
 * \retval OSAL_ERR_UNAVAILABLE             Other errors.
 */
osal_retval_t osal_mutex_lock(osal_mutex_t *mtx);

//! \brief Tries to lock a mutex.
/*!
 * This function tries to lock a mutex. If it is available it locks the mutex 
 * and returns immediately with OSAL_OK. If it is already locked by an other
 * task it returns OSAL_ERR_BUSY without locking or waiting.
 *
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_SYSTEM_LIMIT_REACHED    Not enough system resources.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input paratemer.
 * \retval OSAL_ERR_NOT_RECOVERABLE         Mutex not recoverable.
 * \retval OSAL_ERR_OWNER_DEAD              Old mutex owner dead (see ROBUST).
 * \retval OSAL_ERR_BUSY                    Mutex is already locked.
 * \retval OSAL_ERR_UNAVAILABLE             Other errors.
 */
osal_retval_t osal_mutex_trylock(osal_mutex_t *mtx);

//! \brief Unlocks a mutex.
/*!
 * This function tries to unlock a previously locked mutex.
 *
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_PERMISSION_DENIED       Permission denied unlocking mutex.
 * \retval OSAL_ERR_UNAVAILABLE             Other errors.
 */
osal_retval_t osal_mutex_unlock(osal_mutex_t *mtx);

//! \brief Destroys a mutex.
/*!
 * This function tries to destroy a mutex.
 *
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_OPERATION_FAILED        Other errors.
 */
osal_retval_t osal_mutex_destroy(osal_mutex_t *mtx);

#ifdef __cplusplus
};
#endif

/** @} */

#endif /* LIBOSAL_MUTEX__H */

