/**
 * \file pikeos/task.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 * \author Martin Stelzer <martin.stelzer@dlr.de>
 *
 * \date 07 Sep 2022
 *
 * \brief OSAL task pikeos source.
 *
 * OSAL task pikeos source.
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

#include <libosal/config.h>
#include <libosal/osal.h>
#include <libosal/task.h>

#include <stand/string.h>
#include <assert.h>

#include <vm.h>
#ifdef PIKEOSDEBUG
/* init_gdbstub(), gdb_breakpoint(). */
#include <vm_debug.h>
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
osal_retval_t osal_task_create(osal_task_t *hdl, const osal_task_attr_t *attr, 
        osal_task_handler_t handler, osal_task_handler_arg_t arg) 
{
    assert(hdl != NULL);

    osal_retval_t ret = OSAL_OK;
    P4_e_t local_ret;
    p4ext_thr_attr_t tattr;

    p4ext_thr_attr_init(&tattr);
    tattr.prio = attr->priority;
    tattr.context_flags = P4_THREAD_ARG_FPU | P4_THREAD_ARG_DEBUG;
    hdl->tid = P4EXT_THR_NUM_INVALID;

    local_ret = p4ext_thr_create(&hdl->tid, 0, 
            (strlen(attr->task_name) > 0u) ? attr->task_name : "thread", 
            handler, 1, arg);
    if (local_ret != P4_E_OK) {
        if (local_ret == P4_E_INVAL) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    return ret;
}

//! \brief Joins a task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[in]   retval  Task return value.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_join(osal_task_t *hdl, osal_task_retval_t *retval) {
    assert(hdl != NULL);

    (void)hdl;

    osal_retval_t ret = OSAL_OK;

    if (retval != NULL) {
        *retval = NULL;
    }

    return ret;
}

//! \brief Destroys a task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_destroy(osal_task_t *hdl) {
    assert(hdl != NULL);

    osal_retval_t ret = OSAL_OK;

    P4_e_t local_ret;
    local_ret = p4_thread_delete(hdl->tid);
    if (local_ret != P4_E_OK) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}


//! \brief Get the handle of the calling thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_get_hdl(osal_task_t *hdl) {
    assert(hdl != NULL);

    (void)hdl;

    osal_retval_t ret = OSAL_ERR_NOT_IMPLEMENTED;

    return ret;
}

//! \brief Change the task attributes of the specified task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[in]   attr    The thread's new attributes.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_set_task_attr(osal_task_t *hdl, osal_task_attr_t *attr) {
    assert(hdl != NULL);

    (void)hdl;
    (void)attr;

    osal_retval_t ret = OSAL_ERR_NOT_IMPLEMENTED;
    return ret;
}

//! \brief Get the current task attributes of the specified task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[out]  attr    The thread's current attributes.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_get_task_attr(osal_task_t *hdl, osal_task_attr_t *attr) {
    assert(hdl != NULL);

    (void)hdl;
    (void)attr;

    osal_retval_t ret = OSAL_ERR_NOT_IMPLEMENTED;
    return ret;
}

//! \brief Change the priority of the specified thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[in]   prio    The thread prio as member of osal_task_sched_priority_t
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_set_priority(osal_task_t *hdl,
                                        osal_task_sched_priority_t prio)
{
    assert(hdl != NULL);

    osal_retval_t ret = OSAL_OK;
    P4_e_t local_ret;

    local_ret = p4_thread_ex_priority(hdl->tid, NULL, prio);
    if (local_ret != P4_E_OK) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}

//! \brief Get the current priority of the specified thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[out]  prio    The thread's current prio
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_get_priority(osal_task_t *hdl,
                                        osal_task_sched_priority_t *prio)
{
    assert(hdl != NULL);
    assert(prio != NULL);

    osal_retval_t ret = OSAL_OK;
    P4_e_t local_ret;

    local_ret = p4_thread_get_priority(hdl->tid, prio);
    if (local_ret != P4_E_OK) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}

//! \brief Suspend a thread from running.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_suspend(osal_task_t *hdl) {
    osal_retval_t ret = OSAL_OK;
    P4_e_t local_ret;

    local_ret = p4_thread_stop(hdl->tid);
    if (local_ret != P4_E_OK) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}

//! \brief Resume a thread that has been suspended earlier.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_resume(osal_task_t *hdl) {
    osal_retval_t ret = OSAL_OK;
    P4_e_t local_ret;

    local_ret = p4_thread_resume(hdl->tid);
    if (local_ret != P4_E_OK) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}

//! \brief Delete the calling thread.
/*!
 * \note To be used at the end of a thread's execution
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_delete(osal_void_t) {
    osal_retval_t ret = OSAL_OK;
    P4_e_t local_ret;

    local_ret = p4_thread_delete(P4_THREAD_MYSELF);
    if (local_ret != P4_E_OK) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}

//! \brief Get the current state of a created thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[out]  state   A thread state according to osal_task_State_t
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_get_state(osal_task_t *hdl, osal_task_state_t *state) {
    assert(hdl != NULL);
    
    (void)hdl;
    (void)state;

    osal_retval_t ret = OSAL_ERR_NOT_IMPLEMENTED;

    return ret;
}

