/**
 * \file posix/task.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL task posix source.
 *
 * OSAL task posix source.
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

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>
#include <pthread.h>

#include <libosal/config.h>
#include <libosal/osal.h>
#include <libosal/task.h>

#if LIBOSAL_HAVE_SYS_PRCTL_H == 1
#include <sys/prctl.h>
#endif

#include <errno.h>
#include <assert.h>

#include <stdio.h>
#include <string.h>

typedef struct posix_start_args {
    int running;

    osal_task_handler_t user_handler;
    osal_task_handler_arg_t user_arg;
    const osal_task_attr_t *user_attr;
} posix_start_args_t;

static void *posix_task_wrapper(void *args) {
    // cppcheck-suppress misra-c2012-11.5
    posix_start_args_t *start_args = (posix_start_args_t *)args;

    // copy all stuff to local stack-objects, they will be destroyed after 'start_args->running = 1;'
    osal_task_handler_t user_handler = start_args->user_handler;
    osal_task_handler_arg_t user_arg = start_args->user_arg;
    const osal_task_attr_t *user_attr = start_args->user_attr;

    if (user_attr != NULL) {
        if (user_attr->priority != 0) {
            struct sched_param param;
            int policy = SCHED_FIFO;

            param.sched_priority = user_attr->priority;
            if (pthread_setschedparam(pthread_self(), policy, &param) != 0) {
                (void)printf("libosal: pthread_setschedparam(%p, %d, %u): %s\n",
                        (void *)pthread_self(), policy, user_attr->priority, strerror(errno));
            }

            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            for (uint32_t i = 0u; i < (sizeof(user_attr->affinity) * 8u); ++i) {
                if (user_attr->affinity & (1u << i)) {
                    CPU_SET(i, &cpuset);
                }
            }

            int ret = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
            if (ret != 0) {
                (void)printf("libosal: pthread_setaffinity_np(%p, %#x): %d %s\n", 
                        (void *) pthread_self(), user_attr->affinity, ret, strerror(ret));
            }
        }

#if LIBOSAL_HAVE_SYS_PRCTL_H == 1
        if (strlen(user_attr->task_name) > 0) {
            prctl(PR_SET_NAME, user_attr->task_name, 0, 0, 0);
        }
#endif
    }       
        
    // after setting running to 1, we start_args will be invalid
    start_args->running = 1;

    return (*user_handler)(user_arg);
}

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
        osal_task_handler_t handler, osal_task_handler_arg_t arg) {
    assert(hdl != NULL);

    int ret = OSAL_OK;
    int local_ret;
    posix_start_args_t start_args = { 0, handler, arg, attr };

    local_ret = pthread_create(&hdl->tid, NULL, posix_task_wrapper, &start_args);
    
    if (local_ret != 0) {
        if (local_ret == EAGAIN) {
            ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
        } else if (local_ret == EPERM) {
            ret = OSAL_ERR_PERMISSION_DENIED;
        } else if (local_ret == EINVAL) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    if (ret == OSAL_OK) {
        // only wait if thread has been started successfully
        while (start_args.running == 0) {
            osal_sleep(1000000);
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
int osal_task_join(osal_task_t *hdl, osal_task_retval_t *retval) {
    int ret = OSAL_OK;
    int local_ret;

    local_ret = pthread_join(hdl->tid, retval);
    (void)local_ret;

    if (local_ret != 0) {
        if (local_ret == EDEADLK) {
            ret = OSAL_ERR_DEAD_LOCK;
        } else if (local_ret == EINVAL) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else if (local_ret == ESRCH) {
            ret = OSAL_ERR_NOT_FOUND;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    return ret;
}

//! \brief Destroys a task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_task_destroy(osal_task_t *hdl) {
    int ret = OSAL_OK;

    pthread_cancel(hdl->tid);

    return ret;
}


