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

#ifndef LIBOSAL_CONDVAR__H
#define LIBOSAL_CONDVAR__H

#include <libosal/osal.h>
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

#ifdef LIBOSAL_BUILD_WIN32
#include <libosal/win32/condvar.h>
#endif

#ifdef LIBOSAL_BUILD_STM32
#include <libosal/stm32/condvar.h>
#endif

/** \defgroup condvar_group Conditional Variable
 * The conditional variable are a synchronization mechanism with a 
 * surrounding mutex to work on shared data and signal waiters when
 * data was manipulated or can be safely manipulated.
 *
 * @{
 */

#define OSAL_CONDVAR_ATTR__TYPE__MASK             0x00000003u   //!< \brief Attribute type mask.
#define OSAL_CONDVAR_ATTR__TYPE__NORMAL           0x00000000u   //!< \brief Normal condition variable type.
#define OSAL_CONDVAR_ATTR__TYPE__ERRORCHECK       0x00000001u   //!< \brief Do error checking.
#define OSAL_CONDVAR_ATTR__TYPE__RECURSIVE        0x00000002u   //!< \brief Check if condvar was called recursively from same task.

#define OSAL_CONDVAR_ATTR__ROBUST                 0x00000010u   //!< \brief Condvar robustness, e.g. owner died.
#define OSAL_CONDVAR_ATTR__PROCESS_SHARED         0x00000020u   //!< \brief Condvar is shared between processes.

#define OSAL_CONDVAR_ATTR__PROTOCOL__MASK         0x00000300u   //!< \brief Protocol mask.
#define OSAL_CONDVAR_ATTR__PROTOCOL__NONE         0x00000000u   //!< \brief None (default) protocol.
#define OSAL_CONDVAR_ATTR__PROTOCOL__INHERIT      0x00000100u   //!< \brief Inherit protocol.
#define OSAL_CONDVAR_ATTR__PROTOCOL__PROTECT      0x00000200u   //!< \brief Protect protocol.

#define OSAL_CONDVAR_ATTR__PRIOCEILING__MASK      0xFFFF0000u   //!< \brief Mask for priority ceiling protocol.
#define OSAL_CONDVAR_ATTR__PRIOCEILING__SHIFT     16u           //!< \brief Priority ceiling value shift.

typedef osal_uint32_t osal_condvar_attr_t;                      //!< \brief Condition variable attribute type.

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Initialize a condvar.
/*!
 * This function initializes a condition variable.
 *
 * \param[in]   cv      Pointer to osal condvar structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial condvar attributes. Can be NULL then
 *                      the defaults of the underlying condvar will be used.
 *
 * \retval OSAL_OK                      On success.
 * \retval OSAL_ERR_OUT_OF_MEMORY       Not enough memory to initialize condition variable.
 * \retval OSAL_ERR_INVALID_PARAM       One of the parameters are invalid.
 * \retval OSAL_ERR_UNAVAILABLE         Initialization failed, try again.
 * \retval OSAL_ERR_BUSY                Condition was initialized before.
 * \retval OSAL_ERR_OPERATION_FAILED    Other errors.
 */
osal_retval_t osal_condvar_init(osal_condvar_t *cv, const osal_condvar_attr_t *attr);

//! \brief wait on a condvar.
/*!
 * This function waits on a condition variable until any other tasks call \ref osal_condvar_signal
 * or \ref osal_condvar_wait. The mutex \p mtx has to be locked before calling 
 * \ref osal_condvar_wait and is unlocked during wait.
 *
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 * \param[in]   mtx    Pointer to osal mutex structure. Content is OS dependent.
 *
 * \retval OSAL_OK                  On success.
 */
osal_retval_t osal_condvar_wait(osal_condvar_t *cv, osal_mutex_t *mtx);

//! \brief timed wait on a condvar.
/*!
 * This function waits on a condition variable until any other tasks call \ref osal_condvar_signal
 * or \ref osal_condvar_wait or it is running into the timeout. The mutex \p mtx has to be 
 * locked before calling \ref osal_condvar_wait and is unlocked during wait.
 *
 * \param[in]   cv      Pointer to osal condvar structure. Content is OS dependent.
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 * \param[in]   timeout Timeout
 *
 * \retval OSAL_OK                      On success.
 * \retval OSAL_ERR_TIMEOUT             Timeout expired waiting on condition.
 * \retval OSAL_ERR_PERMISSION_DENIED   Mutex was not owner by thread.
 * \retval OSAL_ERR_INVALID_PARAM       Condvar is invalid/not initalized.
 */
osal_retval_t osal_condvar_timedwait(osal_condvar_t *cv, osal_mutex_t *mtx, const osal_timer_t *timeout);

//! \brief Signals one waiter on a condvar.
/*!
 * This function signals one waiting task to resume it's work.
 *
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 *
 * \retval OSAL_OK                  On success.
 * \retval OSAL_ERR_INVALID_PARAM   Condvar is invalid/not initalized.
 */
osal_retval_t osal_condvar_signal(osal_condvar_t *cv);

//! \brief Broadcast (Wakes) all waiters on a condvar.
/*!
 * This functions signals all waiting tasks to resume their work.
 *
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 *
 * \retval OSAL_OK                  On success.
 * \retval OSAL_ERR_INVALID_PARAM   Condvar is invalid/not initalized.
 */
osal_retval_t osal_condvar_broadcast(osal_condvar_t *cv);

//! \brief Destroys on a condvar.
/*!
 * This function destroy the condition variable structure.
 *
 * \param[in]   cv     Pointer to osal condvar structure. Content is OS dependent.
 *
 * \retval OSAL_OK                      On success.
 * \retval OSAL_ERR_BUSY                Condition destruction alread in progress.
 */
osal_retval_t osal_condvar_destroy(osal_condvar_t *cv);

#ifdef __cplusplus
};
#endif

/** @} */

#endif /* LIBOSAL_CONDVAR__H */


