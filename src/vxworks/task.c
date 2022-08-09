
int vxworks_task_entry(_Vx_usr_arg_t arg1, _Vx_usr_arg_t arg2) {
    osal_task_handler_t hdl = (osal_task_handler_t)arg1;
    osal_task_handler_arg_t arg = (osal_task_handler_arg_t)arg2;

    (*hdl)(arg);
}

//! \brief Create a task.
/*!
 * \param[in]   hdl     Pointer to osal task structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial task attributes. Can be NULL then
 *                      the defaults of the underlying task will be used.
 * \param[in]   hdl     Task handler to be executed.
 * \param[in]   arg     Pointer to argument passed to task handler.
 *
 * \return OK or ERROR_CODE.
 */
int osal_task_create(osal_task_t *hdl, const osal_task_attr_t *attr, 
        osal_task_handler_t hdl, osal_task_handler_arg_t arg) {
    int options = 0;
    size_t stackSize = 0x1000;

    TASK_ID tid = taskSpawn(attr->name, attr->priority, options, stackSize,
            vxworks_task_entry, hdl, arg, 3, 4, 5, 6, 7, 8, 9, 10);
}

