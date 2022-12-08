/**
 * \file mutex.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
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

#ifndef LIBOSAL_MUTEX__H
#define LIBOSAL_MUTEX__H

#include <libosal/config.h>
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

#define OSAL_MUTEX_ATTR__TYPE__MASK             0x00000003u
#define OSAL_MUTEX_ATTR__TYPE__NORMAL           0x00000000u
#define OSAL_MUTEX_ATTR__TYPE__ERRORCHECK       0x00000001u
#define OSAL_MUTEX_ATTR__TYPE__RECURSIVE        0x00000002u

#define OSAL_MUTEX_ATTR__ROBUST                 0x00000010u
#define OSAL_MUTEX_ATTR__PROCESS_SHARED         0x00000020u

#define OSAL_MUTEX_ATTR__PROTOCOL__MASK         0x00000300u
#define OSAL_MUTEX_ATTR__PROTOCOL__NONE         0x00000000u
#define OSAL_MUTEX_ATTR__PROTOCOL__INHERIT      0x00000100u
#define OSAL_MUTEX_ATTR__PROTOCOL__PROTECT      0x00000200u

#define OSAL_MUTEX_ATTR__PRIOCEILING__MASK      0xFFFF0000u
#define OSAL_MUTEX_ATTR__PRIOCEILING__SHIFT     16u

typedef osal_uint32_t osal_mutex_attr_t;

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Initialize a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial mutex attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_init(osal_mutex_t *mtx, const osal_mutex_attr_t *attr);

//! \brief Locks a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_lock(osal_mutex_t *mtx);

//! \brief Tries to lock a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_trylock(osal_mutex_t *mtx);

//! \brief Unlocks a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_unlock(osal_mutex_t *mtx);

//! \brief Destroys a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_destroy(osal_mutex_t *mtx);

#ifdef __cplusplus
};
#endif

#endif /* LIBOSAL_MUTEX__H */

