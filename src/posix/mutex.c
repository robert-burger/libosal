/**
 * \file vxworks/mutex.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL mutex vxworks source.
 *
 * OSAL mutex vxworks source.
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

#include <libosal/osal.h>

#include <errno.h>
#include <pthread.h>
#include <assert.h>

//! \brief Initialize a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial mutex attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_init(osal_mutex_t *mtx, const osal_mutex_attr_t *attr) {
    assert(mtx != NULL);

    osal_retval_t ret = OSAL_OK;
    int posix_ret;

    pthread_mutexattr_t posix_attr;
    pthread_mutexattr_t *pposix_attr = NULL;

    if (attr != NULL) {
        pthread_mutexattr_init(&posix_attr);

        if (((*attr) & OSAL_MUTEX_ATTR__TYPE__MASK) == OSAL_MUTEX_ATTR__TYPE__NORMAL) {
            pthread_mutexattr_settype(&posix_attr, PTHREAD_MUTEX_NORMAL);
        } else if (((*attr) & OSAL_MUTEX_ATTR__TYPE__MASK) == OSAL_MUTEX_ATTR__TYPE__ERRORCHECK) {
            pthread_mutexattr_settype(&posix_attr, PTHREAD_MUTEX_ERRORCHECK);
        } else if (((*attr) & OSAL_MUTEX_ATTR__TYPE__MASK) == OSAL_MUTEX_ATTR__TYPE__RECURSIVE) {
            pthread_mutexattr_settype(&posix_attr, PTHREAD_MUTEX_RECURSIVE);
        } else  {}

#if LIBOSAL_HAVE_PTHREAD_MUTEXATTR_SETROBUST == 1
        if (((*attr) & OSAL_MUTEX_ATTR__ROBUST) == OSAL_MUTEX_ATTR__ROBUST) {
            pthread_mutexattr_setrobust(&posix_attr, PTHREAD_MUTEX_ROBUST);
        }
#endif

        if (((*attr) & OSAL_MUTEX_ATTR__PROCESS_SHARED) == OSAL_MUTEX_ATTR__PROCESS_SHARED) {
            pthread_mutexattr_setpshared(&posix_attr, PTHREAD_PROCESS_SHARED);
        }
        
        if (((*attr) & OSAL_MUTEX_ATTR__PROTOCOL__MASK) == OSAL_MUTEX_ATTR__PROTOCOL__NONE) {
            pthread_mutexattr_setprotocol(&posix_attr, PTHREAD_PRIO_NONE);
        } else if (((*attr) & OSAL_MUTEX_ATTR__PROTOCOL__MASK) == OSAL_MUTEX_ATTR__PROTOCOL__INHERIT) {
            pthread_mutexattr_setprotocol(&posix_attr, PTHREAD_PRIO_INHERIT);
        } else if (((*attr) & OSAL_MUTEX_ATTR__PROTOCOL__MASK) == OSAL_MUTEX_ATTR__PROTOCOL__PROTECT) {
            pthread_mutexattr_setprotocol(&posix_attr, PTHREAD_PRIO_PROTECT);
        } else {}

        if (((*attr) & OSAL_MUTEX_ATTR__PRIOCEILING__MASK) != 0u) {
            int prioceiling = (((*attr) & OSAL_MUTEX_ATTR__PRIOCEILING__MASK) >> OSAL_MUTEX_ATTR__PRIOCEILING__SHIFT);
            pthread_mutexattr_setprioceiling(&posix_attr, prioceiling);
        }

        pposix_attr = &posix_attr;
    }

    posix_ret = pthread_mutex_init(&mtx->posix_mtx, pposix_attr);

    if (posix_ret != 0) {
        if (posix_ret == EAGAIN) {
            ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
        } else if (posix_ret == ENOMEM) {
            ret = OSAL_ERR_OUT_OF_MEMORY;
        } else if (posix_ret == EPERM) {
            ret = OSAL_ERR_PERMISSION_DENIED;
        } else if (posix_ret == EINVAL) {
            ret = OSAL_ERR_INVALID_PARAM;
        } else {
            ret = OSAL_ERR_UNAVAILABLE;
        }
    } else {
        ret = OSAL_OK;
    }

    return ret;
}

//! \brief Locks a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_lock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    osal_retval_t ret;
    int posix_ret;

    posix_ret = pthread_mutex_lock(&mtx->posix_mtx);
    if (posix_ret != 0) {
        if (posix_ret == EAGAIN) {
            ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
        } else if (posix_ret == EINVAL) {
            ret = OSAL_ERR_INVALID_PARAM; 
#if LIBOSAL_HAVE_ENOTRECOVERABLE == 1
        } else if (posix_ret == ENOTRECOVERABLE) {
            ret = OSAL_ERR_NOT_RECOVERABLE;
#endif
        } else if (posix_ret == EOWNERDEAD) {
            ret = OSAL_ERR_OWNER_DEAD;
        } else if (posix_ret == EDEADLK) {
            ret = OSAL_ERR_DEAD_LOCK;
        } else {
            ret = OSAL_ERR_UNAVAILABLE;
        }
    } else {
        ret = OSAL_OK;
    }

    return ret;
}

//! \brief Tries to lock a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_trylock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    osal_retval_t ret;
    int posix_ret;

    posix_ret = pthread_mutex_trylock(&mtx->posix_mtx);
    if (posix_ret != 0) {
        if (posix_ret == EAGAIN) {
            ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
        } else if (posix_ret == EINVAL) {
            ret = OSAL_ERR_INVALID_PARAM; 
#if LIBOSAL_HAVE_ENOTRECOVERABLE == 1
        } else if (posix_ret == ENOTRECOVERABLE) {
            ret = OSAL_ERR_NOT_RECOVERABLE;
#endif
        } else if (posix_ret == EOWNERDEAD) {
            ret = OSAL_ERR_OWNER_DEAD;
        } else if (posix_ret == EBUSY) {
            ret = OSAL_ERR_BUSY;
        } else {
            ret = OSAL_ERR_UNAVAILABLE;
        }
    } else {
        ret = OSAL_OK;
    }

    return ret;
}

//! \brief Unlocks a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_unlock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    osal_retval_t ret;
    int posix_ret;

    posix_ret = pthread_mutex_unlock(&mtx->posix_mtx);
    if (posix_ret != 0) {
        if (posix_ret == EPERM) {
            ret = OSAL_ERR_PERMISSION_DENIED;
        } else {
            ret = OSAL_ERR_UNAVAILABLE;
        }
    } else {
        ret = OSAL_OK;
    }

    return ret;
}

//! \brief Destroys a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_mutex_destroy(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    osal_retval_t ret = OSAL_OK;
    int posix_ret;

    posix_ret = pthread_mutex_destroy(&mtx->posix_mtx);
    if (posix_ret != 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}
