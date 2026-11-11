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

#ifdef HAVE_CONFIG_H
#include <libosal/config.h>
#endif

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>
#include <pthread.h>
#include <signal.h>

#include <libosal/osal.h>
#include <libosal/task.h>
#include <libosal/io.h>

#if LIBOSAL_HAVE_SYS_PRCTL_H == 1
#include <sys/prctl.h>
#endif

#include <errno.h>
#include <assert.h>

#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef struct posix_start_args {
    int running;

    osal_task_handler_t user_handler;
    osal_task_handler_arg_t user_arg;
    const osal_task_attr_t *user_attr;
} posix_start_args_t;

/*
 * Preallocated task thread pool (optional, --enable-task-pool).
 *
 * When enabled, osal_task_create() hands out threads from a pool of
 * permanently running worker threads instead of calling pthread_create()
 * on every invocation. This removes the clone()/stack-mapping cost and
 * the start-up handshake delay from the creation path.
 *
 * The layout of the public osal_task_t is unchanged (a single pthread_t).
 * It holds the real pthread id of the pool worker serving the task, so
 * all functions operating on the handle keep working. If no pool slot
 * is available, creation falls back to the regular pthread_create()
 * path, so the pool never imposes a hard limit on the number of
 * concurrent tasks.
 *
 * Workers are created detached: the pool never pthread_join()s them
 * ("join" is virtualized by the slot state machine), so the runtime
 * reclaims the resources of a dead worker automatically and a revived
 * slot does not leak the previous worker.
 */
#if LIBOSAL_HAVE_TASK_POOL == 1

typedef enum pool_state {
    POOL_STATE_FREE = 0,   /* idle, waiting for a job             */
    POOL_STATE_ASSIGNED,   /* job assigned, worker not running yet */
    POOL_STATE_RUNNING,    /* task handler running                */
    POOL_STATE_DONE        /* handler finished, waiting for join  */
} pool_state_t;

typedef struct pool_slot {
    pthread_t tid;                /* worker thread, copied to hdl->tid */
    osal_task_t *owner;           /* app handle currently using the slot */
    osal_task_handler_t handler;
    osal_task_handler_arg_t arg;
    osal_task_attr_t attr;        /* zeroed when the create attr was NULL */
    osal_task_sched_affinity_t default_aff; /* affinity of the creating thread, a new pthread would inherit it */
    osal_task_retval_t retval;
    pool_state_t state;
    int worker_dead;              /* worker thread gone (cancel/delete)  */
    pthread_cond_t cv;
} pool_slot_t;

static pthread_mutex_t pool_mtx = PTHREAD_MUTEX_INITIALIZER;
static pool_slot_t pool_slots[LIBOSAL_TASK_POOL_SIZE];
static osal_uint32_t pool_size = 0u;
static int pool_ready = 0;
static int pool_atfork_registered = 0;

/*
 * Guards the pool initialization only. Unlike pool_mtx it is never
 * re-initialized, so it can be held while pool_init_locked() resets
 * pool_mtx after a fork().
 */
static pthread_mutex_t pool_guard_mtx = PTHREAD_MUTEX_INITIALIZER;

static int pool_debug(void) {
    static int d = -1;
    if (d < 0) {
        d = (getenv("POOL_DEBUG") != NULL);
    }
    return d;
}

#define POOL_DBG(...) \
    do { \
        if (pool_debug()) { \
            struct timespec __ts; \
            clock_gettime(CLOCK_MONOTONIC, &__ts); \
            fprintf(stderr, "[t=%ld.%03ld] ", (long)__ts.tv_sec, \
                    (long)(__ts.tv_nsec / 1000000L)); \
            fprintf(stderr, __VA_ARGS__); \
        } \
    } while (0)

/*
 * The handle refers to a pool thread:
 *   >= 0: pool slot index, the slot has an active (or awaiting join) job
 *         for this exact handle
 *      0: (not used, POOL_STATE_FREE is 0 - indices start at 0, that's fine)
 *     -1: handle does not refer to a pool thread (use legacy path)
 *     -2: handle refers to a pool thread, but without an active job for
 *         this handle (the task is gone, like ESRCH on a joined thread)
 */
#define POOL_SLOT_NOT_POOL  (-1)
#define POOL_SLOT_FINISHED  (-2)

/*
 * pool_find_slot - find the pool slot owning hdl->tid.
 *
 * Returns the slot index (>= 0) if the slot has an active (or
 * awaiting join) job for this exact handle, POOL_SLOT_NOT_POOL if the
 * handle does not refer to a pool thread, or POOL_SLOT_FINISHED if it
 * refers to a pool thread without an active job for this handle.
 *
 * The pool mutex must be held by the caller.
 */
static int pool_find_slot(osal_task_t *hdl) {
    osal_uint32_t i;

    for (i = 0u; i < pool_size; ++i) {
        if (pthread_equal(pool_slots[i].tid, hdl->tid) != 0) {
            if ((pool_slots[i].owner == hdl) && (pool_slots[i].state != POOL_STATE_FREE)) {
                return (int)i;
            }
            return POOL_SLOT_FINISHED;
        }
    }

    return POOL_SLOT_NOT_POOL;
}

/*
 * pool_configured_size - maximum number of pool threads.
 *
 * Set at configure time with --with-task-pool-size, the environment
 * variable OSAL_TASK_POOL_SIZE may shrink it at runtime.
 */
static osal_uint32_t pool_configured_size(void) {
    osal_uint32_t size = (osal_uint32_t)LIBOSAL_TASK_POOL_SIZE;
    const char *env = getenv("OSAL_TASK_POOL_SIZE");

    if (env != NULL) {
        int v = atoi(env);
        if ((v >= 1) && ((osal_uint32_t)v < size)) {
            size = (osal_uint32_t)v;
        }
    }

    return size;
}

/*
 * Run as cleanup handler when a pool worker thread exits, either
 * cancelled (osal_task_destroy) or via osal_task_delete() inside the
 * task handler. Marks the slot so joiners are released and the slot
 * can be revived by a later osal_task_create().
 */
static void pool_worker_cleanup(void *arg) {
    // cppcheck-suppress misra-c2012-11.5
    pool_slot_t *slot = (pool_slot_t *)arg;

    pthread_mutex_lock(&pool_mtx);
    slot->worker_dead = 1;
    if ((slot->state == POOL_STATE_ASSIGNED) || (slot->state == POOL_STATE_RUNNING)) {
        slot->state = POOL_STATE_DONE;
    }
    pthread_cond_broadcast(&slot->cv);
    pthread_mutex_unlock(&pool_mtx);
}

/*
 * Apply the per-job task attributes on the calling (worker) thread.
 *
 * A pool worker survives across jobs, so fields that are not set by the
 * new job must be explicitly reset to the defaults a freshly created
 * thread would have. Otherwise the policy, priority, affinity or name of
 * a previous job would leak into the next one.
 */
static void pool_apply_attrs(const pool_slot_t *slot) {
    const osal_task_attr_t *attr = &slot->attr;
    int cur_policy;
    struct sched_param cur_param;

    if (attr->policy != 0u) {
        (void)osal_task_set_policy(NULL, attr->policy);
    } else if (pthread_getschedparam(pthread_self(), &cur_policy, &cur_param) == 0) {
        if (cur_policy != SCHED_OTHER) {
            (void)osal_task_set_policy(NULL, OSAL_SCHED_POLICY_OTHER);
        }
    }

    if (attr->priority != 0u) {
        (void)osal_task_set_priority(NULL, attr->priority);
    } else {
        osal_task_sched_priority_t cur_prio = 0u;
        if ((osal_task_get_priority(NULL, &cur_prio) == OSAL_OK) && (cur_prio != 0u)) {
            (void)osal_task_set_priority(NULL, 0u);
        }
    }

#if LIBOSAL_HAVE_PTHREAD_SETAFFINITY_NP
    if (attr->affinity != 0u) {
        (void)osal_task_set_affinity(NULL, attr->affinity);
    } else {
        /* a new pthread would inherit the affinity of the thread that
         * created it, reset the worker to that captured value */
        osal_task_sched_affinity_t cur_aff = 0u;
        if ((slot->default_aff != 0u) &&
                (osal_task_get_affinity(NULL, &cur_aff) == OSAL_OK) &&
                (cur_aff != slot->default_aff)) {
            (void)osal_task_set_affinity(NULL, slot->default_aff);
        }
    }
#endif

#if LIBOSAL_HAVE_SYS_PRCTL_H == 1
    if (strlen(attr->task_name) > 0u) {
        prctl(PR_SET_NAME, attr->task_name, 0, 0, 0);
    } else {
        /* a real new thread would carry a fresh name, reset ours */
        prctl(PR_SET_NAME, "osal-pool", 0, 0, 0);
    }
#endif
}

/*
 * The main loop of a pool worker thread. The worker is created once
 * and stays alive between jobs: it waits for a job to be assigned,
 * runs the task handler, then waits for osal_task_join() to release
 * the slot before going idle again.
 */
static void *pool_worker(void *arg) {
    // cppcheck-suppress misra-c2012-11.5
    pool_slot_t *slot = (pool_slot_t *)arg;

    pthread_cleanup_push(pool_worker_cleanup, slot);

#if LIBOSAL_HAVE_SYS_PRCTL_H == 1
    prctl(PR_SET_NAME, "osal-pool", 0, 0, 0);
#endif

    for (;;) {
        osal_task_handler_t handler;
        osal_task_handler_arg_t arg;
        osal_task_retval_t retval;

        pthread_mutex_lock(&pool_mtx);
        while (slot->state != POOL_STATE_ASSIGNED) {
            pthread_cond_wait(&slot->cv, &pool_mtx);
        }
        POOL_DBG("worker %d: got job, applying attrs\n", (int)(slot - pool_slots));
        handler = slot->handler;
        arg = slot->arg;
        slot->state = POOL_STATE_RUNNING;
        pthread_cond_broadcast(&slot->cv);
        pthread_mutex_unlock(&pool_mtx);

        /* set policy/priority/affinity/name before starting the task,
         * at the same point the legacy wrapper does it. The slot's attr
         * stays valid until the next job is assigned, which can only
         * happen once this job is joined again. */
        pool_apply_attrs(slot);

        POOL_DBG("worker %d: starting handler\n", (int)(slot - pool_slots));
        retval = (*handler)(arg);
        POOL_DBG("worker %d: handler done\n", (int)(slot - pool_slots));

        pthread_mutex_lock(&pool_mtx);
        slot->retval = retval;
        slot->state = POOL_STATE_DONE;
        pthread_cond_broadcast(&slot->cv);
        while (slot->state == POOL_STATE_DONE) {
            pthread_cond_wait(&slot->cv, &pool_mtx);
        }
        /* state == POOL_STATE_FREE, the slot was released by join */
        pthread_mutex_unlock(&pool_mtx);
        POOL_DBG("worker %d: slot released, idle\n", (int)(slot - pool_slots));
    }

    pthread_cleanup_pop(0);
    return NULL;
}

/*
 * (Re-)create the pool. Called with the pool guard mutex held, and
 * with pool_mtx NOT held, so pool_mtx may be (re-)initialized here
 * safely. After fork() in the child the workers are gone, so the pool
 * is simply re-created here.
 */
static void pool_init_locked(void) {
    osal_uint32_t i;
    pthread_attr_t attr;

    pool_size = pool_configured_size();
    if (pool_size > LIBOSAL_TASK_POOL_SIZE) {
        pool_size = (osal_uint32_t)LIBOSAL_TASK_POOL_SIZE;
    }

    pthread_mutex_init(&pool_mtx, NULL);
    (void)pthread_attr_init(&attr);
    (void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    for (i = 0u; i < pool_size; ++i) {
        pool_slot_t *slot = &pool_slots[i];
        (void)pthread_cond_init(&slot->cv, NULL);
        slot->owner = NULL;
        slot->retval = NULL;
        slot->state = POOL_STATE_FREE;
        slot->worker_dead = 0;
        if (pthread_create(&slot->tid, &attr, pool_worker, slot) != 0) {
            /* degraded pool: this slot stays unusable until a later
             * osal_task_create() manages to revive it */
            slot->worker_dead = 1;
        }
    }
    (void)pthread_attr_destroy(&attr);

    pool_ready = 1;
}

static void pool_fork_child(void) {
    /* workers do not survive fork(). The mutexes may have been locked
     * at the time of the fork, so reset them here (the child has a
     * single thread at this point) and mark the pool for re-creation. */
    pthread_mutex_init(&pool_guard_mtx, NULL);
    pthread_mutex_init(&pool_mtx, NULL);
    pool_ready = 0;
    pool_atfork_registered = 0;
}

static void pool_ensure_init(void) {
    pthread_mutex_lock(&pool_guard_mtx);
    if (pool_ready == 0) {
        if (pool_atfork_registered == 0) {
            (void)pthread_atfork(NULL, NULL, pool_fork_child);
            pool_atfork_registered = 1;
        }
        POOL_DBG("pool: init start (pid %d)\n", (int)getpid());
        pool_init_locked();
        POOL_DBG("pool: init done, size %u\n", pool_size);
    }
    pthread_mutex_unlock(&pool_guard_mtx);
}

/*
 * pool_try_create - start a task on a pool worker.
 *
 * Returns:
 *   0: task started on a pool worker, *ret carries the result
 *   1: no pool slot available, caller must use the legacy path
 */
static int pool_try_create(osal_task_t *hdl, const osal_task_attr_t *attr,
        osal_task_handler_t handler, osal_task_handler_arg_t arg, osal_retval_t *ret) {
    osal_uint32_t i;
    int idx = -1;

    pool_ensure_init();

    pthread_mutex_lock(&pool_mtx);
    if (pool_ready != 0) {
        /* prefer a live idle worker */
        for (i = 0u; i < pool_size; ++i) {
            if ((pool_slots[i].state == POOL_STATE_FREE) && (pool_slots[i].worker_dead == 0)) {
                idx = (int)i;
                break;
            }
        }
        /* else revive a slot whose worker died */
        if (idx < 0) {
            for (i = 0u; i < pool_size; ++i) {
                if ((pool_slots[i].state == POOL_STATE_FREE) && (pool_slots[i].worker_dead != 0)) {
                    idx = (int)i;
                    break;
                }
            }
        }
    }
    if (idx < 0) {
        pthread_mutex_unlock(&pool_mtx);
        return 1; /* no slot: use the legacy path */
    }

    {
        pool_slot_t *slot = &pool_slots[idx];

        if (slot->worker_dead != 0) {
            pthread_attr_t attr;

            (void)pthread_cond_init(&slot->cv, NULL);
            (void)pthread_attr_init(&attr);
            (void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            if (pthread_create(&slot->tid, &attr, pool_worker, slot) != 0) {
                (void)pthread_attr_destroy(&attr);
                pthread_mutex_unlock(&pool_mtx);
                return 1; /* cannot revive: use the legacy path */
            }
            (void)pthread_attr_destroy(&attr);
            slot->worker_dead = 0;
        }

        {
            pthread_t old_tid = hdl->tid;

            slot->handler = handler;
            slot->arg = arg;
            if (attr != NULL) {
                slot->attr = *attr;
            } else {
                memset(&slot->attr, 0, sizeof(slot->attr));
            }
#if LIBOSAL_HAVE_PTHREAD_SETAFFINITY_NP
            {
                /* a new pthread would inherit the affinity of this
                 * (creating) thread, capture it for the reset */
                cpu_set_t creator_cpuset;
                uint32_t j;
                slot->default_aff = 0u;
                if (pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &creator_cpuset) == 0) {
                    for (j = 0u; j < (sizeof(slot->default_aff) * 8u); ++j) {
                        if (CPU_ISSET(j, &creator_cpuset) != 0) {
                            slot->default_aff |= ((osal_task_sched_affinity_t)1u << j);
                        }
                    }
                }
            }
#else
            slot->default_aff = 0u;
#endif
            slot->retval = NULL;
            slot->owner = hdl;
            hdl->tid = slot->tid;
            slot->state = POOL_STATE_ASSIGNED;

#if LIBOSAL_HAVE_PTHREAD_KILL == 1 && LIBOSAL_HAVE_SIGCONT == 1
            /* clear a stale SIGSTOP left by an unbalanced suspend */
            (void)pthread_kill(slot->tid, SIGCONT);
#endif

            pthread_cond_broadcast(&slot->cv);

            /* wait until the worker actually started the job, like the
             * legacy implementation waits for start_args.running */
            while ((slot->state != POOL_STATE_RUNNING) && (slot->worker_dead == 0)) {
                pthread_cond_wait(&slot->cv, &pool_mtx);
            }

            if (slot->state == POOL_STATE_RUNNING) {
                *ret = OSAL_OK;
            } else {
                /* worker died before the job could start */
                slot->owner = NULL;
                slot->state = POOL_STATE_FREE;
                hdl->tid = old_tid;
                *ret = OSAL_ERR_OPERATION_FAILED;
            }
        }
    }
    pthread_mutex_unlock(&pool_mtx);
    POOL_DBG("create: slot %d ret %d\n", idx, (int)*ret);
    return 0;
}

/*
 * pool_try_join - join a task that runs on a pool worker.
 *
 * Returns:
 *   0: handled, *ret carries the result
 *   1: handle is not a pool task, caller must use the legacy path
 */
static int pool_try_join(osal_task_t *hdl, osal_task_retval_t *retval, osal_retval_t *ret) {
    int idx;

    if (pool_ready == 0) {
        return 1;
    }

    pthread_mutex_lock(&pool_mtx);
    idx = pool_find_slot(hdl);
    if (idx < 0) {
        pthread_mutex_unlock(&pool_mtx);
        if (idx == POOL_SLOT_FINISHED) {
            /* double join: the legacy pthread_join() would report ESRCH */
            *ret = OSAL_ERR_NOT_FOUND;
            return 0;
        }
        return 1; /* not a pool task: use the legacy path */
    }

    {
        pool_slot_t *slot = &pool_slots[idx];
        osal_task_retval_t local_retval = NULL;

        if (pthread_equal(pthread_self(), slot->tid) != 0) {
            /* joining oneself: like the legacy pthread_join() EDEADLK */
            pthread_mutex_unlock(&pool_mtx);
            *ret = OSAL_ERR_DEAD_LOCK;
            return 0;
        }

        while ((slot->state != POOL_STATE_DONE) && (slot->worker_dead == 0)) {
            pthread_cond_wait(&slot->cv, &pool_mtx);
        }
        local_retval = slot->retval;
        slot->state = POOL_STATE_FREE;
        slot->owner = NULL;
        pthread_cond_broadcast(&slot->cv);
        pthread_mutex_unlock(&pool_mtx);

        if (retval != NULL) {
            *retval = local_retval;
        }
        *ret = OSAL_OK;
        return 0;
    }
}

/*
 * pool_task_active - check whether hdl refers to a pool worker thread
 * that still exists, for the get/set family of functions operating
 * on hdl->tid.
 *
 * The handle stays operable as long as the worker thread it points to
 * exists: while the job runs (ASSIGNED/RUNNING) and also after the
 * handler returned but before the join (DONE, worker alive). That
 * matches the legacy implementation, where the same window is a
 * terminated but unjoined (zombie) thread that the pthread calls
 * still accept.
 *
 * Returns:
 *   0: hdl refers to a live pool worker for this handle, the
 *      caller proceeds with the regular pthread operations on hdl->tid
 *   1: hdl is NULL (calling thread) or does not refer to a pool task,
 *      the caller proceeds with the regular path
 *   2: hdl refers to a pool thread whose worker is gone or no longer
 *      owned by this handle (joined, slot reused, worker cancelled),
 *      the caller must return OSAL_ERR_INVALID_PARAM, like the ESRCH
 *      a regular pthread call on a gone thread would report
 */
static int pool_task_active(osal_task_t *hdl) {
    int idx;
    int active = 1;

    if ((pool_ready == 0) || (hdl == NULL)) {
        return 1;
    }

    pthread_mutex_lock(&pool_mtx);
    idx = pool_find_slot(hdl);
    if (idx >= 0) {
        pool_slot_t *slot = &pool_slots[idx];
        if ((slot->state == POOL_STATE_DONE) && (slot->worker_dead != 0)) {
            /* worker cancelled or self-deleted: the thread is gone, a
             * regular pthread call on hdl->tid would report ESRCH */
            active = 2;
        } else {
            /* ASSIGNED/RUNNING, or DONE with the worker still alive
             * awaiting the join: the thread exists (like the unjoined
             * thread of the legacy implementation), operate on it */
            active = 0;
        }
    } else if (idx == POOL_SLOT_FINISHED) {
        active = 2;
    }
    pthread_mutex_unlock(&pool_mtx);

    return active;
}

#endif /* LIBOSAL_HAVE_TASK_POOL == 1 */

static void *posix_task_wrapper(void *args) {
    // cppcheck-suppress misra-c2012-11.5
    posix_start_args_t *start_args = (posix_start_args_t *)args;

    // copy all stuff to local stack-objects, they will be destroyed after 'start_args->running = 1;'
    osal_task_handler_t user_handler = start_args->user_handler;
    osal_task_handler_arg_t user_arg = start_args->user_arg;
    const osal_task_attr_t *user_attr = start_args->user_attr;

    if (user_attr != NULL) {
        if (user_attr->policy != 0u) {
            osal_retval_t local_ret = osal_task_set_policy(NULL, user_attr->policy);
            if (local_ret != OSAL_OK) {
                switch (local_ret) {
                    case OSAL_ERR_PERMISSION_DENIED:
                        fprintf(stderr, "unknown error occured setting policy to %d: PERMISSION DENIED\n", user_attr->policy);
                        break;
                    case OSAL_ERR_OPERATION_FAILED:
                        fprintf(stderr, "unknown error occured setting policy to %d: OPERATION FAILED\n", user_attr->policy);
                        break;
                    default:
                        fprintf(stderr, "unknown error occured setting policy to %d: %d\n", user_attr->policy, local_ret);
                        break;
                }
            }
        }

        if (user_attr->priority != 0u) {
            osal_retval_t local_ret = osal_task_set_priority(NULL, user_attr->priority);
            if (local_ret != OSAL_OK) {
                switch (local_ret) {
                    case OSAL_ERR_PERMISSION_DENIED:
                        fprintf(stderr, "unknown error occured setting priority to %d: PERMISSION DENIED\n", user_attr->priority);
                        break;
                    case OSAL_ERR_OPERATION_FAILED:
                        fprintf(stderr, "unknown error occured setting priority to %d: OPERATION FAILED\n", user_attr->priority);
                        break;
                    default:
                        fprintf(stderr, "unknown error occured setting priority to %d: %d\n", user_attr->priority, local_ret);
                        break;
                }
            }
        }

        if (user_attr->affinity > 0u) {
            osal_retval_t local_ret = osal_task_set_affinity(NULL, user_attr->affinity);
            if (local_ret != OSAL_OK) {
                switch (local_ret) {
                    case OSAL_ERR_INVALID_PARAM:
                        fprintf(stderr, "unknown error occured setting affinity to %d: INVALID PARAMETER\n", user_attr->affinity);
                        break;
                    default:
                        fprintf(stderr, "unknown error occured setting affinity to %d: %d\n", user_attr->affinity, local_ret);
                        break;
                }
            }
        }

#if LIBOSAL_HAVE_SYS_PRCTL_H == 1
        if (strlen(user_attr->task_name) > 0u) {
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
osal_retval_t osal_task_create(osal_task_t *hdl, const osal_task_attr_t *attr,
        osal_task_handler_t handler, osal_task_handler_arg_t arg) {
    assert(hdl != NULL);

#if LIBOSAL_HAVE_TASK_POOL == 1
    {
        osal_retval_t pool_ret = OSAL_OK;
        if (pool_try_create(hdl, attr, handler, arg, &pool_ret) == 0) {
            return pool_ret;
        }
    }
#endif

    osal_retval_t ret = OSAL_OK;
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
osal_retval_t osal_task_join(osal_task_t *hdl, osal_task_retval_t *retval) {
    osal_retval_t ret = OSAL_OK;
    int local_ret;

#if LIBOSAL_HAVE_TASK_POOL == 1
    {
        osal_retval_t pool_ret = OSAL_OK;
        if (pool_try_join(hdl, retval, &pool_ret) == 0) {
            return pool_ret;
        }
    }
#endif

    local_ret = pthread_join(hdl->tid, retval);

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
osal_retval_t osal_task_destroy(osal_task_t *hdl) {
    assert(hdl != NULL);

    osal_retval_t ret = OSAL_OK;

#if LIBOSAL_HAVE_TASK_POOL == 1
    if (pool_ready != 0) {
        pthread_mutex_lock(&pool_mtx);
        {
            int idx = pool_find_slot(hdl);
            if (idx >= 0) {
                pool_slot_t *slot = &pool_slots[idx];
                if ((slot->state != POOL_STATE_ASSIGNED) && (slot->state != POOL_STATE_RUNNING)) {
                    /* job already finished: the legacy thread would be
                     * gone here (ESRCH), and the idle worker must not
                     * be cancelled */
                    pthread_mutex_unlock(&pool_mtx);
                    return OSAL_ERR_NOT_FOUND;
                }
                /* active job: cancel the worker thread below */
            } else if (idx == POOL_SLOT_FINISHED) {
                pthread_mutex_unlock(&pool_mtx);
                return OSAL_ERR_NOT_FOUND;
            }
        }
        pthread_mutex_unlock(&pool_mtx);
    }
#endif

    if (pthread_cancel(hdl->tid) != 0) {
        ret = OSAL_ERR_NOT_FOUND;
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

#if LIBOSAL_HAVE_TASK_POOL == 1
    if (pool_task_active(hdl) == 2) {
        return OSAL_ERR_INVALID_PARAM;
    }
#endif

    osal_retval_t ret = OSAL_OK;
    int local_ret;

    struct sched_param param;
    param.sched_priority = attr->priority;
    local_ret = pthread_setschedparam(hdl->tid, attr->policy, &param);
    if (local_ret != 0) {
        if ((local_ret == ESRCH) || (local_ret == EINVAL)) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else if (local_ret == EPERM) {
            ret = OSAL_ERR_PERMISSION_DENIED;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    if (ret == OSAL_OK) {
#if LIBOSAL_HAVE_PTHREAD_SETAFFINITY_NP
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        for (uint32_t i = 0u; i < (sizeof(attr->affinity) * 8u); ++i) {
            if ((attr->affinity & ((uint32_t)1u << i)) != 0u) {
                CPU_SET(i, &cpuset);
            }
        }

        local_ret = pthread_setaffinity_np(hdl->tid, sizeof(cpu_set_t), &cpuset);
        if (local_ret != 0) {
            ret = OSAL_ERR_INVALID_PARAM;
        }
#endif
    }

    if (ret == OSAL_OK) {
#if LIBOSAL_HAVE_SYS_PRCTL_H == 1
        if (strlen(attr->task_name) > 0u) {
            prctl(PR_SET_NAME, attr->task_name, 0, 0, 0);
        }
#endif
    }

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
#if LIBOSAL_HAVE_TASK_POOL == 1
    if (pool_task_active(hdl) == 2) {
        return OSAL_ERR_INVALID_PARAM;
    }
#endif

    osal_retval_t ret = OSAL_OK;
    int local_ret;

    int policy;
    struct sched_param param;
    local_ret = pthread_getschedparam(hdl->tid, &policy, &param);
    if (local_ret == 0) {
        attr->policy = policy;
        attr->priority = param.sched_priority;
    } else {
        if ((local_ret == ESRCH) || (local_ret == EINVAL)) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else if (local_ret == EPERM) {
            ret = OSAL_ERR_PERMISSION_DENIED;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    if (ret == OSAL_OK) {
#if LIBOSAL_HAVE_PTHREAD_SETAFFINITY_NP
        attr->affinity = 0;

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);

        local_ret = pthread_getaffinity_np(hdl->tid, sizeof(cpuset), &cpuset);
        if (local_ret != 0) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else {
            for (osal_uint32_t j = 0; j < CPU_SETSIZE; j++) {
                if (CPU_ISSET(j, &cpuset) != 0u) {
                    attr->affinity |= (1u << j);
                }
            }
        }
#endif
    }

    if (ret == OSAL_OK) {
#if LIBOSAL_HAVE_SYS_PRCTL_H == 1
        prctl(PR_GET_NAME, attr->task_name, 0, 0, 0);
#endif
    }

    return ret;
}

//! \brief Change the policy of the specified thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *                      If <b> hdl is NULL, set policy for calling thread.
 * \param[in]   prio    The thread prio as member of osal_task_sched_policy_t
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_set_policy(osal_task_t *hdl, osal_task_sched_policy_t policy)
{
#if LIBOSAL_HAVE_TASK_POOL == 1
    if (pool_task_active(hdl) == 2) {
        return OSAL_ERR_INVALID_PARAM;
    }
#endif

    osal_retval_t ret = OSAL_OK;
    int local_ret;

    int tmp_policy;
    struct sched_param param;
    pthread_t tid = hdl != NULL ? hdl->tid : pthread_self();
    local_ret = pthread_getschedparam(tid, &tmp_policy, &param);

    if (local_ret != 0) {
        if ((local_ret == ESRCH) || (local_ret == EINVAL)) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else if (local_ret == EPERM) {
            ret = OSAL_ERR_PERMISSION_DENIED;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    if (ret == OSAL_OK) {
        if (policy == OSAL_SCHED_POLICY_FIFO) {
            tmp_policy = SCHED_FIFO;
        } else if (policy == OSAL_SCHED_POLICY_ROUND_ROBIN) {
            tmp_policy = SCHED_RR;
        } else {
            tmp_policy = SCHED_OTHER;
        }

        if (sched_get_priority_min(tmp_policy) > param.sched_priority) {
            param.sched_priority = sched_get_priority_min(tmp_policy);
        } else if (sched_get_priority_max(tmp_policy) < param.sched_priority) {
            param.sched_priority = sched_get_priority_max(tmp_policy);
        }

        local_ret = pthread_setschedparam(pthread_self(), tmp_policy, &param);
    
        if (local_ret != 0) {
            if ((local_ret == ESRCH) || (local_ret == EINVAL)) {
                ret = OSAL_ERR_INVALID_PARAM;
            } else if (local_ret == EPERM) {
                ret = OSAL_ERR_PERMISSION_DENIED;
            } else {
                ret = OSAL_ERR_OPERATION_FAILED;
            }
        }
    }

    return ret;
}

//! \brief Get the current policy of the specified thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *                      If <b> hdl is NULL, get policy for calling thread.
 * \param[out]  prio    The thread's current policy
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_get_policy(osal_task_t *hdl, osal_task_sched_policy_t *policy)
{
    assert(hdl != NULL);
    assert(policy != NULL);

#if LIBOSAL_HAVE_TASK_POOL == 1
    if (pool_task_active(hdl) == 2) {
        return OSAL_ERR_INVALID_PARAM;
    }
#endif

    osal_retval_t ret = OSAL_OK;
    int local_ret;

    int tmp_policy;
    struct sched_param param;
    pthread_t tid = hdl != NULL ? hdl->tid : pthread_self();
    local_ret = pthread_getschedparam(tid, &tmp_policy, &param);

    if (local_ret != 0) {
        if ((local_ret == ESRCH) || (local_ret == EINVAL)) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else if (local_ret == EPERM) {
            ret = OSAL_ERR_PERMISSION_DENIED;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    if (ret == OSAL_OK) {
        if (tmp_policy == SCHED_FIFO) {
            (*policy) = OSAL_SCHED_POLICY_FIFO;
        } else if (tmp_policy == SCHED_RR) {
            (*policy) = OSAL_SCHED_POLICY_ROUND_ROBIN;
        } else {
            (*policy) = OSAL_SCHED_POLICY_OTHER;
        }
    }

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
#if LIBOSAL_HAVE_TASK_POOL == 1
    if (pool_task_active(hdl) == 2) {
        return OSAL_ERR_INVALID_PARAM;
    }
#endif

    osal_retval_t ret = OSAL_OK;
    int local_ret;

    int policy;
    struct sched_param param;
    pthread_t tid = hdl != NULL ? hdl->tid : pthread_self();
    local_ret = pthread_getschedparam(tid, &policy, &param);

    if (local_ret != 0) {
        if ((local_ret == ESRCH) || (local_ret == EINVAL)) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else if (local_ret == EPERM) {
            ret = OSAL_ERR_PERMISSION_DENIED;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    if (ret == OSAL_OK) {
        param.sched_priority = prio;

        if (sched_get_priority_min(policy) > param.sched_priority) {
            param.sched_priority = sched_get_priority_min(policy);
        } else if (sched_get_priority_max(policy) < param.sched_priority) {
            param.sched_priority = sched_get_priority_max(policy);
        }

        local_ret = pthread_setschedparam(tid, policy, &param);

        if (local_ret != 0) {
            if ((local_ret == ESRCH) || (local_ret == EINVAL)) {
                ret = OSAL_ERR_INVALID_PARAM;
            } else if (local_ret == EPERM) {
                ret = OSAL_ERR_PERMISSION_DENIED;
            } else {
                ret = OSAL_ERR_OPERATION_FAILED;
            }
        }
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
#if LIBOSAL_HAVE_TASK_POOL == 1
    if (pool_task_active(hdl) == 2) {
        return OSAL_ERR_INVALID_PARAM;
    }
#endif

    osal_retval_t ret = OSAL_OK;
    int local_ret;

    int policy;
    struct sched_param param;
    pthread_t tid = hdl != NULL ? hdl->tid : pthread_self();
    local_ret = pthread_getschedparam(tid, &policy, &param);
    if (local_ret == 0) {
        *prio = param.sched_priority;
    } else {
        if ((local_ret == ESRCH) || (local_ret == EINVAL)) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else if (local_ret == EPERM) {
            ret = OSAL_ERR_PERMISSION_DENIED;
        } else {
            ret = OSAL_ERR_OPERATION_FAILED;
        }
    }

    return ret;
}

//! \brief Change the affinity of the specified thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *                      If <b> hdl is NULL, set affinity for calling thread.
 * \param[in]   prio    The thread affinity as member of osal_task_sched_priority_t
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_set_affinity(osal_task_t *hdl,
                                        osal_task_sched_affinity_t affinity)
{
#if LIBOSAL_HAVE_TASK_POOL == 1
    if (pool_task_active(hdl) == 2) {
        return OSAL_ERR_INVALID_PARAM;
    }
#endif

    osal_retval_t ret = OSAL_OK;

    if (affinity > 0u) {
#if LIBOSAL_HAVE_PTHREAD_SETAFFINITY_NP
        pthread_t tid = hdl != NULL ? hdl->tid : pthread_self();
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        for (uint32_t i = 0u; i < (sizeof(affinity) * 8u); ++i) {
            if ((affinity & ((uint32_t)1u << i)) != 0u) {
                CPU_SET(i, &cpuset);
            }
        }

        int local_ret = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
        if (local_ret != 0) {
            ret = OSAL_ERR_INVALID_PARAM;
        }
#endif
    }

    return ret;
}

//! \brief Change the affinity of the specified thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 *                      If <b> hdl is NULL, set affinity for calling thread.
 * \param[in]   prio    The thread affinity as member of osal_task_sched_priority_t
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_get_affinity(osal_task_t *hdl,
                                        osal_task_sched_affinity_t *affinity)
{
#if LIBOSAL_HAVE_TASK_POOL == 1
    if (pool_task_active(hdl) == 2) {
        return OSAL_ERR_INVALID_PARAM;
    }
#endif

    osal_retval_t ret = OSAL_OK;

#if LIBOSAL_HAVE_PTHREAD_SETAFFINITY_NP
    int local_ret;
    pthread_t tid = hdl != NULL ? hdl->tid : pthread_self();

    cpu_set_t cpuset;
    (*affinity) = 0;
    CPU_ZERO(&cpuset);

    local_ret = pthread_getaffinity_np(tid, sizeof(cpuset), &cpuset);
    if (local_ret != 0) {
        ret = OSAL_ERR_INVALID_PARAM;
    } else {
        for (osal_uint32_t j = 0; j < CPU_SETSIZE; j++) {
            if (CPU_ISSET(j, &cpuset) != 0u) {
                (*affinity) |= (1u << j);
            }
        }
    }
#endif
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

#if LIBOSAL_HAVE_TASK_POOL == 1
    if (pool_ready != 0) {
        pthread_mutex_lock(&pool_mtx);
        {
            int idx = pool_find_slot(hdl);
            if (idx >= 0) {
                pool_slot_t *slot = &pool_slots[idx];
                if ((slot->state != POOL_STATE_ASSIGNED) && (slot->state != POOL_STATE_RUNNING)) {
                    /* job finished: legacy pthread_kill() would report ESRCH */
                    pthread_mutex_unlock(&pool_mtx);
                    return OSAL_ERR_INVALID_PARAM;
                }
            } else if (idx == POOL_SLOT_FINISHED) {
                pthread_mutex_unlock(&pool_mtx);
                return OSAL_ERR_INVALID_PARAM;
            }
        }
        pthread_mutex_unlock(&pool_mtx);
    }
#endif

#if LIBOSAL_HAVE_SIGSTOP == 1 && LIBOSAL_HAVE_PTHREAD_KILL == 1
    int local_ret;

    local_ret = pthread_kill(hdl->tid, SIGSTOP);
    if (local_ret != 0) {
        ret = OSAL_ERR_INVALID_PARAM;
    }
#else 
    ret = OSAL_ERR_NOT_IMPLEMENTED;
#endif

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

#if LIBOSAL_HAVE_TASK_POOL == 1
    if (pool_ready != 0) {
        pthread_mutex_lock(&pool_mtx);
        {
            int idx = pool_find_slot(hdl);
            if (idx >= 0) {
                pool_slot_t *slot = &pool_slots[idx];
                if ((slot->state != POOL_STATE_ASSIGNED) && (slot->state != POOL_STATE_RUNNING)) {
                    /* job finished: legacy pthread_kill() would report ESRCH */
                    pthread_mutex_unlock(&pool_mtx);
                    return OSAL_ERR_INVALID_PARAM;
                }
            } else if (idx == POOL_SLOT_FINISHED) {
                pthread_mutex_unlock(&pool_mtx);
                return OSAL_ERR_INVALID_PARAM;
            }
        }
        pthread_mutex_unlock(&pool_mtx);
    }
#endif

#if LIBOSAL_HAVE_SIGCONT == 1 && LIBOSAL_HAVE_PTHREAD_KILL == 1
    int local_ret;
    local_ret = pthread_kill(hdl->tid, SIGCONT);
    if (local_ret != 0) {
        ret = OSAL_ERR_INVALID_PARAM;
    }
#else 
    ret = OSAL_ERR_NOT_IMPLEMENTED;
#endif

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

    void *retval = NULL;
    pthread_exit(retval);

    return ret;
}

//! \brief Get the current state of a created thread.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[out]  state   A thread state according to osal_task_State_t
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_task_get_state(osal_task_t *hdl,
                                      osal_task_state_t *state)
{
    assert(hdl != NULL);

    (void)hdl;
    (void)state;

    osal_retval_t ret = OSAL_ERR_NOT_IMPLEMENTED;

    return ret;
}

//! \brief Get the number of preallocated task pool threads.
/*!
 * \return The maximum number of threads in the preallocated task pool,
 *         or 0 if the task pool was not enabled in this library build
 *         (library built without --enable-task-pool).
 */
osal_uint32_t osal_task_pool_size(osal_void_t) {
#if LIBOSAL_HAVE_TASK_POOL == 1
    return pool_configured_size();
#else
    return 0u;
#endif
}
