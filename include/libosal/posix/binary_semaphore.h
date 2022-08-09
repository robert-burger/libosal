#include <pthread.h>

typedef struct osal_binary_semaphore {
    pthread_mutex_t posix_mtx;
    pthread_cond_t posix_cond;
    int value;
} osal_binary_semaphore_t;

