#include <libosal/osal.h>
#include <libosal/task.h>

#include <errno.h>
#include <assert.h>

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
    pthread_attr_t pthr_attr;
    pthread_attr_t *ppthr_attr = NULL;
    struct sched_param sch_prm;

    if (attr != NULL) {
        pthread_attr_init(&pthr_attr);
        ppthr_attr = &pthr_attr;

        sch_prm.sched_priority = attr->priority;

        pthread_attr_setschedparam(ppthr_attr, &sch_prm);
    }

    local_ret = pthread_create(&hdl->tid, ppthr_attr, handler, arg);
    
    if (local_ret == EAGAIN) {
        ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
    } else if (local_ret == EPERM) {
        ret = OSAL_ERR_PERMISSION_DENIED;
    } else if (local_ret == EINVAL) {
        ret = OSAL_ERR_INVALID_PARAM;
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


