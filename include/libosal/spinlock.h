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

#ifndef LIBOSAL_SPINLOCK__H
#define LIBOSAL_SPINLOCK__H

#include <libosal/config.h>
#include <libosal/types.h>

#ifdef LIBOSAL_BUILD_POSIX
#include <libosal/posix/spinlock.h>
#endif

#ifdef LIBOSAL_BUILD_VXWORKS
#include <libosal/vxworks/spinlock.h>
#endif

#ifdef LIBOSAL_BUILD_PIKEOS
#include <libosal/pikeos/spinlock.h>
#endif

#define OSAL_SPINLOCK_ATTR__TYPE__MASK             0x00000003u
#define OSAL_SPINLOCK_ATTR__TYPE__NORMAL           0x00000000u
#define OSAL_SPINLOCK_ATTR__TYPE__ERRORCHECK       0x00000001u
#define OSAL_SPINLOCK_ATTR__TYPE__RECURSIVE        0x00000002u

#define OSAL_SPINLOCK_ATTR__ROBUST                 0x00000010u
#define OSAL_SPINLOCK_ATTR__PROCESS_SHARED         0x00000020u

#define OSAL_SPINLOCK_ATTR__PROTOCOL__MASK         0x00000300u
#define OSAL_SPINLOCK_ATTR__PROTOCOL__NONE         0x00000000u
#define OSAL_SPINLOCK_ATTR__PROTOCOL__INHERIT      0x00000100u
#define OSAL_SPINLOCK_ATTR__PROTOCOL__PROTECT      0x00000200u

#define OSAL_SPINLOCK_ATTR__PRIOCEILING__MASK      0xFFFF0000u
#define OSAL_SPINLOCK_ATTR__PRIOCEILING__SHIFT     16u

typedef osal_uint32_t osal_spinlock_attr_t;

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Initialize a spinlock.
/*!
 * \param[in]   mtx     Pointer to osal spinlock structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial spinlock attributes. Can be NULL then
 *                      the defaults of the underlying spinlock will be used.
 *
 * \return OK or ERROR_CODE.
 */
int osal_spinlock_init(osal_spinlock_t *mtx, const osal_spinlock_attr_t *attr);

//! \brief Locks a spinlock.
/*!
 * \param[in]   mtx     Pointer to osal spinlock structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_spinlock_lock(osal_spinlock_t *mtx);

//! \brief Unlocks a spinlock.
/*!
 * \param[in]   mtx     Pointer to osal spinlock structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_spinlock_unlock(osal_spinlock_t *mtx);

//! \brief Destroys a spinlock.
/*!
 * \param[in]   mtx     Pointer to osal spinlock structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_spinlock_destroy(osal_spinlock_t *mtx);

#ifdef __cplusplus
};
#endif

#endif /* LIBOSAL_SPINLOCK__H */

