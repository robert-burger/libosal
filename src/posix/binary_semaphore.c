#include <libosal/osal.h>
#include <assert.h>
#include <errno.h>

#define timespec_add(tvp, sec, nsec) { \
    (tvp)->tv_nsec += (nsec); \
    (tvp)->tv_sec += (sec); \
    if ((tvp)->tv_nsec > (long int)1E9) { \
        (tvp)->tv_nsec -= (long int)1E9; \
        (tvp)->tv_sec++; } }

//! \brief Initialize a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial semaphore attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 *
 * \return OK or ERROR_CODE.
 */
int osal_binary_semaphore_init(osal_binary_semaphore_t *sem, osal_binary_semaphore_attr_t *attr) {
    assert(sem != NULL);

    return 0;
}

//! \brief Post a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_binary_semaphore_post(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    pthread_mutex_lock(&sem->posix_mtx);

    if (sem->value == 0) {
        sem->value = 1;
        pthread_cond_signal(&sem->posix_cond);
    }

    pthread_mutex_unlock(&sem->posix_mtx);
    return 0;
}

//! \brief Wait for a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_binary_semaphore_wait(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    pthread_mutex_lock(&sem->posix_mtx);

    while (!sem->value) {
        pthread_cond_wait(&sem->posix_cond, &sem->posix_mtx);
    }

    sem->value = 0;
    
    pthread_mutex_unlock(&sem->posix_mtx);
    return 0;
}

//! \brief Wait for a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 * \param[in]   nsec    Timeout in nanoseconds.
 *
 * \return OK or ERROR_CODE.
 */
int osal_binary_semaphore_timedwait(osal_binary_semaphore_t *sem, osal_uint64_t nsec) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    timespec_add(&ts, nsec / (osal_uint64_t)1E9, nsec % (osal_uint64_t)1E9);

    pthread_mutex_lock(&sem->posix_mtx);

    while (!sem->value) {
        int ret = pthread_cond_timedwait(&sem->posix_cond, &sem->posix_mtx, &ts);
        if (ret == ETIMEDOUT) {
            break;
        }
    }

    sem->value = 0;
    
    pthread_mutex_unlock(&sem->posix_mtx);

    return 0;
}

//! \brief Destroys a binary_semaphore.
/*!
 * \param[in]   sem     Pointer to osal binary_semaphore structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_binary_semaphore_destroy(osal_binary_semaphore_t *sem) {
    assert(sem != NULL);

    pthread_mutex_destroy(&sem->posix_mtx);
    pthread_cond_destroy(&sem->posix_cond);

    return 0;
}

