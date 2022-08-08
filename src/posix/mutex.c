#include <libosal/osal.h>

#include <pthread.h>
#include <assert.h>

int osal_mutex_init(osal_mutex_t *mtx, const osal_mutex_attr_t *attr) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
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
        }

        if (((*attr) & OSAL_MUTEX_ATTR__ROBUST) == OSAL_MUTEX_ATTR__ROBUST) {
            pthread_mutexattr_setrobust(&posix_attr, PTHREAD_MUTEX_ROBUST);
        }

        if (((*attr) & OSAL_MUTEX_ATTR__PROCESS_SHARED) == OSAL_MUTEX_ATTR__PROCESS_SHARED) {
            pthread_mutexattr_setpshared(&posix_attr, PTHREAD_PROCESS_SHARED);
        }
        
        if (((*attr) & OSAL_MUTEX_ATTR__PROTOCOL__MASK) == OSAL_MUTEX_ATTR__PROTOCOL__NONE) {
            pthread_mutexattr_setprotocol(&posix_attr, PTHREAD_PRIO_NONE);
        } else if (((*attr) & OSAL_MUTEX_ATTR__PROTOCOL__MASK) == OSAL_MUTEX_ATTR__PROTOCOL__INHERIT) {
            pthread_mutexattr_setprotocol(&posix_attr, PTHREAD_PRIO_INHERIT);
        } else if (((*attr) & OSAL_MUTEX_ATTR__PROTOCOL__MASK) == OSAL_MUTEX_ATTR__PROTOCOL__PROTECT) {
            pthread_mutexattr_setprotocol(&posix_attr, PTHREAD_PRIO_PROTECT);
        }

        if (((*attr) & OSAL_MUTEX_ATTR__PRIOCEILING__MASK) != 0u) {
            int prioceiling = (((*attr) & OSAL_MUTEX_ATTR__PRIOCEILING__MASK) >> OSAL_MUTEX_ATTR__PRIOCEILING__SHIFT);
            pthread_mutexattr_setprioceiling(&posix_attr, prioceiling);
        }

        pposix_attr = &posix_attr;
    }

    posix_ret = pthread_mutex_init(&mtx->posix_mtx, pposix_attr);
    if (posix_ret != 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}

int osal_mutex_lock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
    int posix_ret;

    posix_ret = pthread_mutex_lock(&mtx->posix_mtx);
    if (posix_ret != 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}

int osal_mutex_trylock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
    int posix_ret;

    posix_ret = pthread_mutex_trylock(&mtx->posix_mtx);
    if (posix_ret != 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}

int osal_mutex_unlock(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
    int posix_ret;

    posix_ret = pthread_mutex_unlock(&mtx->posix_mtx);
    if (posix_ret != 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}

int osal_mutex_destroy(osal_mutex_t *mtx) {
    assert(mtx != NULL);

    int ret = OSAL_OK;
    int posix_ret;

    posix_ret = pthread_mutex_destroy(&mtx->posix_mtx);
    if (posix_ret != 0) {
        ret = OSAL_ERR_OPERATION_FAILED;
    }

    return ret;
}
