#include <pthread.h>

typedef struct osal_spinlock {
    pthread_spinlock_t posix_sl;
} osal_spinlock_t;

