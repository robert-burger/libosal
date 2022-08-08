#include <pthread.h>

typedef struct osal_mutex {
    pthread_mutex_t posix_mtx;
} osal_mutex_t;

