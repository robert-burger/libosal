/**
 * \file condvar.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Sep 2022
 *
 * \brief OSAL condvar header.
 *
 * OSAL condvar include header.
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

#ifndef LIBOSAL_CONDVAR__H
#define LIBOSAL_CONDVAR__H

#include <libosal/config.h>
#include <libosal/types.h>
#include <libosal/mutex.h>
#include <libosal/timer.h>

#ifdef LIBOSAL_BUILD_POSIX
#include <libosal/posix/condvar.h>
#endif

#ifdef LIBOSAL_BUILD_VXWORKS
#include <libosal/vxworks/condvar.h>
#endif

#ifdef LIBOSAL_BUILD_PIKEOS
#include <libosal/pikeos/condvar.h>
#endif

#define OSAL_CONDVAR_ATTR__TYPE__MASK             0x00000003u
#define OSAL_CONDVAR_ATTR__TYPE__NORMAL           0x00000000u
#define OSAL_CONDVAR_ATTR__TYPE__ERRORCHECK       0x00000001u
#define OSAL_CONDVAR_ATTR__TYPE__RECURSIVE        0x00000002u

#define OSAL_CONDVAR_ATTR__ROBUST                 0x00000010u
#define OSAL_CONDVAR_ATTR__PROCESS_SHARED         0x00000020u

#define OSAL_CONDVAR_ATTR__PROTOCOL__MASK         0x00000300u
#define OSAL_CONDVAR_ATTR__PROTOCOL__NONE         0x00000000u
#define OSAL_CONDVAR_ATTR__PROTOCOL__INHERIT      0x00000100u
#define OSAL_CONDVAR_ATTR__PROTOCOL__PROTECT      0x00000200u

#define OSAL_CONDVAR_ATTR__PRIOCEILING__MASK      0xFFFF0000u
#define OSAL_CONDVAR_ATTR__PRIOCEILING__SHIFT     16u

typedef osal_uint32_t osal_condvar_attr_t;

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Initialize a condvar.
/*!
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial condvar attributes. Can be NULL then
 *                      the defaults of the underlying condvar will be used.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_init(osal_condvar_t *cv, const osal_condvar_attr_t *attr);

//! \brief wait on a condvar.
/*!
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 * \param[in]   mtx    Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_wait(osal_condvar_t *cv, osal_mutex_t *mtx);

//! \brief timed wait on a condvar.
/*!
 * \param[in]   cv      Pointer to osal condvar structure. Content is OS dependent.
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 * \param[in]   to      Timeout
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_timedwait(osal_condvar_t *cv, osal_mutex_t *mtx, const osal_timer_t *timeout);

//! \brief Signals one waiter on a condvar.
/*!
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_signal(osal_condvar_t *cv);

//! \brief Broadcast (Wakes) all waiters on a condvar.
/*!
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_broadcast(osal_condvar_t *cv);

//! \brief Destroys on a condvar.
/*!
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_condvar_destroy(osal_condvar_t *cv);

#ifdef __cplusplus
};
#endif

#endif /* LIBOSAL_CONDVAR__H */


