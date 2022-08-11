#include <libosal/osal.h>
#include <p4.h>

int osal_mutex_lock(osal_mutex_t *mtx) {
    assert(mtx != NULL);
    assert(mtx->_hdl ! =NULL);

    P4_mutex_t *pikeos_mtx = (P4_mutex_t *)mtx->_hdl;

    int ret = p4_mutex_lock(pikeos_mtx, P4_TIMEOUT_NULL);

    return ret;
}


int osal_mutex_trylock(osal_mutex_t *mutex);

int osal_mutex_unlock(osal_mutex_t *mutex) {
    assert(mtx != NULL);
    assert(mtx->_hdl ! =NULL);

    P4_mutex_t *pikeos_mtx = (P4_mutex_t *)mtx->_hdl;

    int ret = p4_mutex_unlock(pikeos_mtx);
}

