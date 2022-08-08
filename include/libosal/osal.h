

#define OSAL_OK                     0
#define OSAL_ERR_OPERATION_FAILED   -1

#include <libosal/config.h>

#ifdef BUILD_POSIX
#include <libosal/posix/mutex.h>
#endif

#include <stdint.h>

#define OSAL_MUTEX_ATTR__TYPE__MASK             0x00000003u
#define OSAL_MUTEX_ATTR__TYPE__NORMAL           0x00000000u
#define OSAL_MUTEX_ATTR__TYPE__ERRORCHECK       0x00000001u
#define OSAL_MUTEX_ATTR__TYPE__RECURSIVE        0x00000002u

#define OSAL_MUTEX_ATTR__ROBUST                 0x00000010u
#define OSAL_MUTEX_ATTR__PROCESS_SHARED         0x00000020u

#define OSAL_MUTEX_ATTR__PROTOCOL__MASK         0x00000300u
#define OSAL_MUTEX_ATTR__PROTOCOL__NONE         0x00000000u
#define OSAL_MUTEX_ATTR__PROTOCOL__INHERIT      0x00000100u
#define OSAL_MUTEX_ATTR__PROTOCOL__PROTECT      0x00000200u

#define OSAL_MUTEX_ATTR__PRIOCEILING__MASK      0xFFFF0000u
#define OSAL_MUTEX_ATTR__PRIOCEILING__SHIFT     16u

typedef uint32_t osal_mutex_attr_t;

//! \brief Initialize a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial mutex attributes. Can be NULL then
 *                      the defaults of the underlying mutex will be used.
 *
 * \return OK or ERROR_CODE.
 */
int osal_mutex_init(osal_mutex_t *mtx, const osal_mutex_attr_t *attr);

//! \brief Locks a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_mutex_lock(osal_mutex_t *mtx);

//! \brief Tries to lock a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_mutex_trylock(osal_mutex_t *mtx);

//! \brief Unlocks a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_mutex_unlock(osal_mutex_t *mtx);

//! \brief Destroys a mutex.
/*!
 * \param[in]   mtx     Pointer to osal mutex structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
int osal_mutex_destroy(osal_mutex_t *mtx);
