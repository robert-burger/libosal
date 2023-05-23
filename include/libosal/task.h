/**
 * \file task.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 * \author Martin Stelzer <martin.stelzer@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL task header.
 *
 * OSAL task include header.
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

#ifndef LIBOSAL_TASK__H
#define LIBOSAL_TASK__H

#include <libosal/config.h>
#include <libosal/types.h>

#ifdef LIBOSAL_BUILD_POSIX
#include <libosal/posix/task.h>
#endif

#ifdef LIBOSAL_BUILD_VXWORKS
#include <libosal/vxworks/task.h>
#endif
    
#ifdef LIBOSAL_BUILD_PIKEOS
#include <libosal/pikeos/task.h>
#endif

#ifdef LIBOSAL_BUILD_WIN32
#include <libosal/win32/task.h>
#endif

/** \defgroup task_group Tasks
 *
 * Parallelize your work with separate tasks.
 *
 * @{
 */

#define OSAL_SCHED_POLICY_FIFO          ((osal_uint32_t)0x00000001u)
#define OSAL_SCHED_POLICY_ROUND_ROBIN   ((osal_uint32_t)0x00000002u)
#define OSAL_SCHED_POLICY_OTHER         ((osal_uint32_t)0x00000003u)

#define TASK_NAME_LEN   64u

typedef osal_uint32_t osal_task_sched_policy_t;
typedef osal_uint32_t osal_task_sched_priority_t;
typedef osal_uint32_t osal_task_sched_affinity_t;

typedef struct osal_task_attr {
    osal_char_t task_name[TASK_NAME_LEN];
    osal_task_sched_policy_t   policy;
    osal_task_sched_priority_t priority;
    osal_task_sched_affinity_t affinity;
} osal_task_attr_t;

typedef void *(*osal_task_handler_t)(void *arg);
typedef void * osal_task_handler_arg_t;
typedef void * osal_task_retval_t;

typedef osal_uint32_t osal_task_state_t;

#define OSAL_STATE_THREAD_UNKNOWN_ID    (0u)   /** @brief The thread has an unknown ID         */
#define OSAL_STATE_THREAD_ACTIVE        (1u)   /** @brief The thread is in an active state     */
#define OSAL_STATE_THREAD_INACTIVE      (2u)   /** @brief The thread is in an inactive state   */
#define OSAL_STATE_THREAD_BLOCKED       (3u)   /** @brief The thread is in a blocked state     */

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Create a task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial task attributes. Can be NULL then
 *                      the defaults of the underlying task will be used.
 * \param[in]   handler Task handler to be executed.
 * \param[in]   arg     Pointer to argument passed to task handler.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_SYSTEM_LIMIT_REACHED    System is out of resources.
 * \retval OSAL_ERR_PERMISSION_DENIED       Permission denied for priority/policy.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 * \retval OSAL_ERR_OPERATION_FAILED        Other errors.
 */
osal_retval_t osal_task_create(osal_task_t *hdl, const osal_task_attr_t *attr, 
        osal_task_handler_t handler, osal_task_handler_arg_t arg);

//! \brief Joins a task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[in]   retval  Task return value.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_DEAD_LOCK               Dead lock while joining task.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 * \retval OSAL_ERR_NOT_FOUND               No task found.
 * \retval OSAL_ERR_OPERATION_FAILED        Other errors.
 */
osal_retval_t osal_task_join(osal_task_t *hdl, osal_task_retval_t *retval);

//! \brief Destroys a task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_NOT_FOUND               No task found.
 */
osal_retval_t osal_task_destroy(osal_task_t *hdl);

//! \brief Get the handle of the calling thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_NOT_IMPLEMENTED         Not implemented.
 */
osal_retval_t osal_task_get_hdl(osal_task_t *hdl);

//! \brief Change the task attributes of the specified task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[in]   attr    The thread's new attributes.
 *
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_OPERATION_FAILED        Other errors.
 * \retval OSAL_ERR_PERMISSION_DENIED       Insufficient permission to set priority/affinity/policy.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 */
osal_retval_t osal_task_set_task_attr(osal_task_t *hdl, osal_task_attr_t *attr);

//! \brief Get the current task attributes of the specified task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[out]  attr    The thread's current attributes.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_OPERATION_FAILED        Other errors.
 * \retval OSAL_ERR_PERMISSION_DENIED       Insufficient permission to get priority/affinity/policy.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 */
osal_retval_t osal_task_get_task_attr(osal_task_t *hdl, osal_task_attr_t *attr);

//! \brief Change the policy of the specified thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *                      If <b> hdl is NULL, set policy for calling thread.
 * \param[in]   prio    The thread prio as member of osal_task_sched_policy_t
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_OPERATION_FAILED        Other errors.
 * \retval OSAL_ERR_PERMISSION_DENIED       Insufficient permission to set policy.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 */
osal_retval_t osal_task_set_policy(osal_task_t *hdl,
                                        osal_task_sched_policy_t policy);

//! \brief Get the current policy of the specified thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *                      If <b> hdl is NULL, get policy for calling thread.
 * \param[out]  prio    The thread's current policy
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_OPERATION_FAILED        Other errors.
 * \retval OSAL_ERR_PERMISSION_DENIED       Insufficient permission to get policy.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 */
osal_retval_t osal_task_get_policy(osal_task_t *hdl,
                                        osal_task_sched_policy_t *policy);

//! \brief Change the priority of the specified thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *                      If <b> hdl is NULL, set priority for calling thread.
 * \param[in]   prio    The thread prio as member of osal_task_sched_priority_t
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_OPERATION_FAILED        Other errors.
 * \retval OSAL_ERR_PERMISSION_DENIED       Insufficient permission to set priority.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 */
osal_retval_t osal_task_set_priority(osal_task_t *hdl,
                                        osal_task_sched_priority_t prio);

//! \brief Get the current priority of the specified thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *                      If <b> hdl is NULL, get priority for calling thread.
 * \param[out]  prio    The thread's current prio
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_OPERATION_FAILED        Other errors.
 * \retval OSAL_ERR_PERMISSION_DENIED       Insufficient permission to get priority.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 */
osal_retval_t osal_task_get_priority(osal_task_t *hdl,
                                        osal_task_sched_priority_t *prio);

//! \brief Change the affinity of the specified thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *                      If <b> hdl is NULL, set affinity for calling thread.
 * \param[in]   prio    The thread affinity as member of osal_task_sched_priority_t
 *
 * \retval OSAL_OK                          On success.
 */
osal_retval_t osal_task_set_affinity(osal_task_t *hdl, 
                                        osal_task_sched_affinity_t affinity);

//! \brief Change the affinity of the specified thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *                      If <b> hdl is NULL, set affinity for calling thread.
 * \param[in]   prio    The thread affinity as member of osal_task_sched_priority_t
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 */
osal_retval_t osal_task_get_affinity(osal_task_t *hdl, 
                                        osal_task_sched_affinity_t *affinity);

//! \brief Suspend a thread from running.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_NOT_IMPLEMENTED         Not implemented.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 */
osal_retval_t osal_task_suspend(osal_task_t *hdl);

//! \brief Resume a thread that has been suspended earlier.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_NOT_IMPLEMENTED         Not implemented.
 * \retval OSAL_ERR_INVALID_PARAM           Invalid input parameter.
 */
osal_retval_t osal_task_resume(osal_task_t *hdl);

//! \brief Delete the calling thread.
/*!
 * \note To be used at the end of a thread's execution
 *
 * \retval OSAL_OK                          On success.
 */
osal_retval_t osal_task_delete(osal_void_t);

//! \brief Get the current state of a created thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[out]  state   A thread state according to osal_task_State_t
 *
 * \retval OSAL_OK                          On success.
 * \retval OSAL_ERR_NOT_IMPLEMENTED         Not implemented.
 */
osal_retval_t osal_task_get_state(osal_task_t *hdl,
                                     osal_task_state_t *state);

#ifdef __cplusplus
};
#endif

/** @} */

#endif /* LIBOSAL_TASK__H */


