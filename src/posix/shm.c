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
 * libosal is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * libosal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with libosal; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <libosal/shm.h>
#include <libosal/osal.h>
#include <libosal/config.h>

#include <assert.h>

#ifdef LIBOSAL_HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>
#include <unistd.h>

//! \brief Initialize a shm.
/*!
 * \param[in]   shm     Pointer to osal shm structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial shm attributes. Can be NULL then
 *                      the defaults of the underlying shm will be used.
 * \param[in]   size    Size for shm creation. Ignored in case shm already existed.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_shm_open(osal_shm_t *shm, const osal_char_t *name,  const osal_shm_attr_t *attr, const osal_size_t size) {
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

        struct stat buf;
        fstat(shm->fd, &buf);

        if (buf.st_size > 0) {
            shm->size = buf.st_size;
        } else {
            shm->size = size;
            local_retval = ftruncate(shm->fd, shm->size);
            if (local_retval != 0) {
                ret = OSAL_ERR_OPERATION_FAILED;
            }
        }
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
 * \param[in]   attr    Pointer to map attributes.
 * \param[out]  ptr     Pointer where to returned mapped data pointer.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_shm_map(osal_shm_t *shm, const osal_shm_map_attr_t *attr, osal_void_t **ptr) {
    assert(shm != NULL);
    assert(ptr != NULL);
    osal_retval_t ret = OSAL_OK;

    int prot = 0;
    int flags = 0;

    if (attr != NULL) {
        if ((*attr & OSAL_SHM_MAP_ATTR__PROT_EXEC) != 0u) {
            prot |= PROT_EXEC;
        }
        if ((*attr & OSAL_SHM_MAP_ATTR__PROT_READ) != 0u) {
            prot |= PROT_READ;
        }
        if ((*attr & OSAL_SHM_MAP_ATTR__PROT_WRITE) != 0u) {
            prot |= PROT_WRITE;
        }
        if ((*attr & OSAL_SHM_MAP_ATTR__PROT_NONE) != 0u) {
            prot |= PROT_NONE;
        }

        if ((*attr & OSAL_SHM_MAP_ATTR__SHARED) != 0u) {
            flags |= MAP_SHARED;
        }
        if ((*attr & OSAL_SHM_MAP_ATTR__PRIVATE) != 0u) {
            flags |= MAP_PRIVATE;
        }
    }

    *ptr = mmap(NULL, shm->size, prot, flags, shm->fd, 0);

    if (*ptr == (void *)-1) {
        switch (errno) {
            case EACCES:    // A file descriptor refers to a non-regular file.  Or a file mapping was requested, 
                            // but fd is not open for reading.  Or MAP_SHARED was requested and  PROT_WRITE is
                            // set, but fd is not open in read/write (O_RDWR) mode. Or PROT_WRITE is set, but 
                            // the file is append-only.
                ret = OSAL_ERR_PERMISSION_DENIED;
                break;
            case EAGAIN:    // The file has been locked, or too much memory has been locked (see setrlimit(2)).
                ret = OSAL_ERR_OPERATION_FAILED;
                break;
            case EBADF:     // fd is not a valid file descriptor (and MAP_ANONYMOUS was not set).
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case EEXIST:    // MAP_FIXED_NOREPLACE was specified in flags, and the range covered by addr and 
                            // length clashes with an existing mapping.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case EINVAL:    // We don't like addr, length, or offset (e.g., they are too large, or not aligned 
                            // on a page boundary). flags contained none of MAP_PRIVATE, MAP_SHARED or MAP_SHARED_VALIDATE.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case ENFILE:    // The system-wide limit on the total number of open files has been reached.
                ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
                break;
            case ENODEV:    // The underlying filesystem of the specified file does not support memory mapping.
                ret = OSAL_ERR_NOT_IMPLEMENTED;
                break;
            case ENOMEM:    // No memory is available. The process's maximum number of mappings would have been 
                            // exceeded. This error can also occur for munmap(), when unmapping a region in the 
                            // middle of an existing mapping, since this results in two smaller mappings on either 
                            // side of the region being unmapped.
                ret = OSAL_ERR_OUT_OF_MEMORY;
                break;
            case EOVERFLOW: // On 32-bit architecture together with the large file extension (i.e., using 64-bit off_t): 
                            // the number of pages used for length plus number of pages used for offset would 
                            // overflow unsigned long  (32 bits).
                ret = OSAL_ERR_OPERATION_FAILED;
                break;
            case EPERM:     // The prot argument asks for PROT_EXEC but the mapped area belongs to a file on a 
                            // filesystem that was mounted no-exec.
                            // The operation was prevented by a file seal; see fcntl(2).
                ret = OSAL_ERR_PERMISSION_DENIED;
                break;
            case ETXTBSY:   // MAP_DENYWRITE was set but the object specified by fd is open for writing.
                ret = OSAL_ERR_PERMISSION_DENIED;
                break;
            default:
                ret = OSAL_ERR_OPERATION_FAILED;
                break;
        }
    }

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

    close(shm->fd);

    return ret;
}


