#include <vxWorks.h>

#include <pthread.h>

typedef struct osal_task {
    TASK_ID tid;
} osal_task_t;
