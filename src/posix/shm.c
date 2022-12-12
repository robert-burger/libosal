/**
 * \file posix/shm.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL shm posix source.
 *
 * OSAL shm posix source.
 */

/*
 * This file is part of libosal.
 *
 * libosal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libosal is distributed in the hope that 
 * it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libosal. If not, see <http://www.gnu.org/licenses/>.
 */

#include <libosal/shm.h>
#include <libosal/osal.h>
#include <libosal/config.h>

#include <assert.h>

#ifdef LIBOSAL_HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>

//! \brief Initialize a shm.
/*!
 * \param[in]   shm     Pointer to osal shm structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial shm attributes. Can be NULL then
 *                      the defaults of the underlying shm will be used.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_shm_open(osal_shm_t *shm, const osal_char_t *name,  const osal_shm_attr_t *attr) {
    assert(shm != NULL);
    assert(name != NULL);
    
    osal_retval_t ret = OSAL_OK;
    mode_t mode = 0;
    int oflag = 0;
    if (attr != NULL) {
        mode = ((*attr) & OSAL_SHM_ATTR__MODE__MASK) >> OSAL_SHM_ATTR__MODE__SHIFT;

        osal_uint32_t attr_flags = (*attr) & OSAL_SHM_ATTR__FLAG__MASK;
        if ((attr_flags & OSAL_SHM_ATTR__FLAG__RDONLY) != 0u) {
            oflag |= O_RDONLY;
        }

        if ((attr_flags & OSAL_SHM_ATTR__FLAG__RDWR) != 0u) {
            oflag |= O_RDWR;
        }

        if ((attr_flags & OSAL_SHM_ATTR__FLAG__CREAT) != 0u) {
            oflag |= O_CREAT;
        }

        if ((attr_flags & OSAL_SHM_ATTR__FLAG__EXCL) != 0u) {
            oflag |= O_EXCL;
        }

        if ((attr_flags & OSAL_SHM_ATTR__FLAG__TRUNC) != 0u) {
            oflag |= O_TRUNC;
        }
    }

    int local_retval = shm_open(name, oflag, mode);
    if (local_retval > 0) {
        shm->fd = local_retval;
    } else {
        switch (errno) {
            case EACCES:        // Permission was denied to shm_open() name in the specified  mode,
                                // or O_TRUNC was specified and the caller does not have write per‐
                                // mission on the object.
                ret = OSAL_ERR_PERMISSION_DENIED;
                break;
            case EEXIST:        // Both O_CREAT and O_EXCL were specified  to  shm_open()  and  the
                                // shared memory object specified by name already exists.
                ret = OSAL_ERR_OPERATION_FAILED;
                break;
            case EINVAL:        // The name argument to shm_open() was invalid.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case EMFILE:        // The per-process limit on the number of open file descriptors has
                                // been reached.
                                // The system-wide limit on the total number of open files has been
                                // reached.
                ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
                break;
            case ENAMETOOLONG:  // The length of name exceeds PATH_MAX.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case ENOENT:        // An attempt was made to shm_open() a name that did not exist, and
                                // O_CREAT was not specified.
                                // An attempt was to made to shm_unlink() a name that does not  ex‐
                                // ist.
                ret = OSAL_ERR_NOT_FOUND;
                break;
        }
    }

    return ret;
}

//! \brief Map a shm.
/*!
 * \param[in]   shm     Pointer to osal shm structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_shm_map(osal_shm_t *shm, const osal_size_t size, osal_void_t **ptr) {
    assert(shm != NULL);
    assert(ptr != NULL);
    osal_retval_t ret = OSAL_OK;

    return ret;
}

//! \brief Closes an open shm.
/*!
 * \param[in]   shm     Pointer to osal shm structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_shm_close(osal_shm_t *shm) {
    assert(shm != NULL);
    osal_retval_t ret = OSAL_OK;

    return ret;
}


