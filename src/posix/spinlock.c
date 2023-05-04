/**
 * \file vxworks/spinlock.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL spinlock vxworks source.
 *
 * OSAL spinlock vxworks source.
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

#include <libosal/osal.h>

#include <errno.h>
#include <pthread.h>
#include <assert.h>

//! \brief Initialize a spinlock.
/*!
 * \param[in]   mtx     Pointer to osal spinlock structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial spinlock attributes. Can be NULL then
 *                      the defaults of the underlying spinlock will be used.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_spinlock_init(osal_spinlock_t *mtx, const osal_spinlock_attr_t *attr) {
    assert(mtx != NULL);

    osal_retval_t ret = OSAL_OK;
    int posix_ret;

    (void)attr;
/*
    pthread_spinlockattr_t posix_attr;
    pthread_spinlockattr_t *pposix_attr = NULL;

    if (attr != NULL) {
        pthread_spinlockattr_init(&posix_attr);

        if (((*attr) & OSAL_SPINLOCK_ATTR__TYPE__MASK) == OSAL_SPINLOCK_ATTR__TYPE__NORMAL) {
            pthread_spinlockattr_settype(&posix_attr, PTHREAD_SPINLOCK_NORMAL);
        } else if (((*attr) & OSAL_SPINLOCK_ATTR__TYPE__MASK) == OSAL_SPINLOCK_ATTR__TYPE__ERRORCHECK) {
            pthread_spinlockattr_settype(&posix_attr, PTHREAD_SPINLOCK_ERRORCHECK);
        } else if (((*attr) & OSAL_SPINLOCK_ATTR__TYPE__MASK) == OSAL_SPINLOCK_ATTR__TYPE__RECURSIVE) {
            pthread_spinlockattr_settype(&posix_attr, PTHREAD_SPINLOCK_RECURSIVE);
        }

        if (((*attr) & OSAL_SPINLOCK_ATTR__ROBUST) == OSAL_SPINLOCK_ATTR__ROBUST) {
            pthread_spinlockattr_setrobust(&posix_attr, PTHREAD_SPINLOCK_ROBUST);
        }

        if (((*attr) & OSAL_SPINLOCK_ATTR__PROCESS_SHARED) == OSAL_SPINLOCK_ATTR__PROCESS_SHARED) {
            pthread_spinlockattr_setpshared(&posix_attr, PTHREAD_PROCESS_SHARED);
        }
        
        if (((*attr) & OSAL_SPINLOCK_ATTR__PROTOCOL__MASK) == OSAL_SPINLOCK_ATTR__PROTOCOL__NONE) {
            pthread_spinlockattr_setprotocol(&posix_attr, PTHREAD_PRIO_NONE);
        } else if (((*attr) & OSAL_SPINLOCK_ATTR__PROTOCOL__MASK) == OSAL_SPINLOCK_ATTR__PROTOCOL__INHERIT) {
            pthread_spinlockattr_setprotocol(&posix_attr, PTHREAD_PRIO_INHERIT);
        } else if (((*attr) & OSAL_SPINLOCK_ATTR__PROTOCOL__MASK) == OSAL_SPINLOCK_ATTR__PROTOCOL__PROTECT) {
            pthread_spinlockattr_setprotocol(&posix_attr, PTHREAD_PRIO_PROTECT);
        }

        if (((*attr) & OSAL_SPINLOCK_ATTR__PRIOCEILING__MASK) != 0u) {
            int prioceiling = (((*attr) & OSAL_SPINLOCK_ATTR__PRIOCEILING__MASK) >> OSAL_SPINLOCK_ATTR__PRIOCEILING__SHIFT);
            pthread_spinlockattr_setprioceiling(&posix_attr, prioceiling);
        }

        pposix_attr = &posix_attr;
    }*/

    posix_ret = pthread_spin_init(&mtx->posix_sl, 0);//pposix_attr);

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

//! \brief Locks a spinlock.
/*!
 * \param[in]   mtx     Pointer to osal spinlock structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_spinlock_lock(osal_spinlock_t *mtx) {
    assert(mtx != NULL);

    osal_retval_t ret;
    int posix_ret;

    posix_ret = pthread_spin_lock(&mtx->posix_sl);
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

//! \brief Unlocks a spinlock.
/*!
 * \param[in]   mtx     Pointer to osal spinlock structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_spinlock_unlock(osal_spinlock_t *mtx) {
    assert(mtx != NULL);

    osal_retval_t ret;
    int posix_ret;

    posix_ret = pthread_spin_unlock(&mtx->posix_sl);
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

//! \brief Destroys a spinlock.
/*!
 * \param[in]   mtx     Pointer to osal spinlock structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_spinlock_destroy(osal_spinlock_t *mtx) {
    assert(mtx != NULL);

    osal_retval_t ret = OSAL_OK;
    int posix_ret;

    posix_ret = pthread_spin_destroy(&mtx->posix_sl);
    if (posix_ret != 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}
