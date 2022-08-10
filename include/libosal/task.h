/**
 * \file task.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
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
 * \return OK or ERROR_CODE.
 */
int osal_task_create(osal_task_t *hdl, const osal_task_attr_t *attr, 
        osal_task_handler_t handler, osal_task_handler_arg_t arg);

//! \brief Joins a task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[in]   retval  Task return value.
 *
 * \return OK or ERROR_CODE.
 */
int osal_task_join(osal_task_t *hdl, osal_task_retval_t *retval);

//! \brief Destroys a task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_task_destroy(osal_task_t *hdl);

#ifdef __cplusplus
};
#endif

#endif /* LIBOSAL_MUTEX__H */


