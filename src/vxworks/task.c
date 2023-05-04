/**
 * \file task.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL vxworks task.
 *
 * OSAL vxworks.
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
#include <vxWorks.h>
#include <taskLib.h>

#include <string.h>

#include <libosal/osal.h>
#include <libosal/task.h>

static int vxworks_task_entry(_Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2) {
    osal_task_handler_t hdl = (osal_task_handler_t)arg1;
    osal_task_handler_arg_t arg = (osal_task_handler_arg_t)arg2;

    (*hdl)(arg);

    return 0;
}

//! \brief Create a task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial task attributes. Can be NULL then
 *                      the defaults of the underlying task will be used.
 * \param[in]   hdl     Task handler to be executed.
 * \param[in]   arg     Pointer to argument passed to task handler.
 *
 * \return OK or ERROR_CODE.
 */
int osal_task_create(osal_task_t *hdl, const osal_task_attr_t *attr, 
        osal_task_handler_t handler, osal_task_handler_arg_t arg) {
    int options = 0;
    size_t stackSize = 0x1000;

    int priority = attr->priority;
    char task_name[TASK_NAME_LEN];
    (void)strcpy(task_name, attr->task_name);

    // cppcheck-suppress misra-c2012-11.1
    hdl->tid = taskSpawn(&task_name[0], priority, options, stackSize, vxworks_task_entry, (_Vx_usr_arg_t)handler, (_Vx_usr_arg_t)arg, 3, 4, 5, 6, 7, 8, 9, 10);

    return OSAL_OK;
}

