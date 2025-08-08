/**
 * \file spinlock.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL spinlock header.
 *
 * OSAL spinlock include header.
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

#ifndef LIBOSAL_SPINLOCK__H
#define LIBOSAL_SPINLOCK__H

#include <libosal/osal.h>

#ifdef LIBOSAL_BUILD_POSIX
#include <libosal/posix/spinlock.h>
#endif

#ifdef LIBOSAL_BUILD_VXWORKS
#include <libosal/vxworks/spinlock.h>
#endif

#ifdef LIBOSAL_BUILD_PIKEOS
#include <libosal/pikeos/spinlock.h>
#endif

#ifdef LIBOSAL_BUILD_WIN32
#include <libosal/win32/spinlock.h>
#endif

#ifdef LIBOSAL_BUILD_STM32
#include <libosal/stm32/spinlock.h>
#endif



/** \defgroup spinlock_group Spinlocks
 *
 * Spinlocks are a mutual exclusion mechanism similar to mutexes. The main difference is, that 
 * waiting on a spinlock does an active/busy-wait and costs CPU-time, but does not have the 
 * side-effects of the OS-scheduler.
 *
 * @{
 */

#define OSAL_SPINLOCK_ATTR__TYPE__MASK             0x00000003u      //!< \brief Spinlock type mask.
#define OSAL_SPINLOCK_ATTR__TYPE__NORMAL           0x00000000u      //!< \brief Spinlock normal/default type.
#define OSAL_SPINLOCK_ATTR__TYPE__ERRORCHECK       0x00000001u      //!< \brief Spinlock error-checking type.
#define OSAL_SPINLOCK_ATTR__TYPE__RECURSIVE        0x00000002u      //!< \brief Spinlock recursive type.

#define OSAL_SPINLOCK_ATTR__ROBUST                 0x00000010u      //!< \brief Robust spinlock (unlocks if owner died).
#define OSAL_SPINLOCK_ATTR__PROCESS_SHARED         0x00000020u      //!< \brief Process shared spinlock.

#define OSAL_SPINLOCK_ATTR__PROTOCOL__MASK         0x00000300u      //!< \brief Spinlock protocol mask.
#define OSAL_SPINLOCK_ATTR__PROTOCOL__NONE         0x00000000u      //!< \brief Spinlock protocol default.
#define OSAL_SPINLOCK_ATTR__PROTOCOL__INHERIT      0x00000100u      //!< \brief Spinlock inherit protocol.
#define OSAL_SPINLOCK_ATTR__PROTOCOL__PROTECT      0x00000200u      //!< \brief Spinlock protect protocol.

#define OSAL_SPINLOCK_ATTR__PRIOCEILING__MASK      0xFFFF0000u      //!< \brief Spinlock priority ceiling mask.
#define OSAL_SPINLOCK_ATTR__PRIOCEILING__SHIFT     16u              //!< \brief Spinlock priority ceiling value.

typedef osal_uint32_t osal_spinlock_attr_t;         //!< \brief Spinlock attribute type.

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Initialize a spinlock.
/*!
 * This function initializes a spinlock structure given by \p mtx. If no attributes
 * are given with \p attr a default spinlock is initiazed.
 *
 * \param[in]   mtx     Pointer to osal spinlock structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial spinlock attributes. Can be NULL then
 *                      the defaults of the underlying spinlock will be used.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_SYSTEM_LIMIT_REACHED    Not enough system resources.
 * \retval OSAL_ERR_OUT_OF_MEMORY           System is out of memory.
 * \retval OSAL_ERR_PERMISSION_DENIED       Permission denied opening a shared mutex.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 * \retval OSAL_ERR_UNAVAILABLE             Other errors. 
 */
osal_retval_t osal_spinlock_init(osal_spinlock_t *mtx, const osal_spinlock_attr_t *attr);

//! \brief Locks a spinlock.
/*!
 * This function tries to lock a spinlock. If the spinlock is already locked by another
 * task it busy-waits until the other task unlocks the spinlock or an other error occures.
 *
 * \param[in]   mtx     Pointer to osal spinlock structure. Content is OS dependent.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_SYSTEM_LIMIT_REACHED    Not enough system resources.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input paratemer.
 * \retval OSAL_ERR_NOT_RECOVERABLE         Mutex not recoverable.
 * \retval OSAL_ERR_OWNER_DEAD              Old mutex owner dead (see ROBUST).
 * \retval OSAL_ERR_DEAD_LOCK               Would dead-lock (see RECURSIVE).
 * \retval OSAL_ERR_UNAVAILABLE             Other errors.
 */
osal_retval_t osal_spinlock_lock(osal_spinlock_t *mtx);

//! \brief Unlocks a spinlock.
/*!
 * This function tries to unlock a previously locked spinlock.
 *
 * \param[in]   mtx     Pointer to osal spinlock structure. Content is OS dependent.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_PERMISSION_DENIED       Permission denied unlocking mutex.
 * \retval OSAL_ERR_UNAVAILABLE             Other errors.
 */
osal_retval_t osal_spinlock_unlock(osal_spinlock_t *mtx);

//! \brief Destroys a spinlock.
/*!
 * This function tries to destroy a spinlock.
 *
 * \param[in]   mtx     Pointer to osal spinlock structure. Content is OS dependent.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_OPERATION_FAILED        Other errors.
 */
osal_retval_t osal_spinlock_destroy(osal_spinlock_t *mtx);

#ifdef __cplusplus
};
#endif

/** @} */

#endif /* LIBOSAL_SPINLOCK__H */

