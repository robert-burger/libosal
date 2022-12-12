/**
 * \file pikeos/shm.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL shm pikeos source.
 *
 * OSAL shm pikeos source.
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

#include <assert.h>

#include <vm.h>
#include <p4ext/p4ext_vmem.h>

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
    P4_e_t local_retval;

    vm_file_access_mode_t oflags = 0;
    if (attr != 0) {
        if ((*attr & OSAL_SHM_ATTR__FLAG__RDONLY) != 0u) {
            oflags |= VM_O_RD;
        } 
        if ((*attr & OSAL_SHM_ATTR__FLAG__RDWR) != 0u) {
            oflags |= VM_O_RD | VM_O_WR;
        }
        if ((*attr & OSAL_SHM_ATTR__FLAG__CREAT) != 0u) {
            oflags |= VM_O_CREAT;
        }
        if ((*attr & OSAL_SHM_ATTR__FLAG__EXCL) != 0u) {
            oflags |= VM_O_EXCL;
        }
        if ((*attr & OSAL_SHM_ATTR__FLAG__TRUNC) != 0u) {
        }
        if ((*attr & OSAL_SHM_ATTR__FLAG__MAP) != 0u) {
            oflags |= VM_O_MAP;
        }
    }

    local_retval = vm_open(name, oflags, &shm->fd);
    if (local_retval != P4_E_OK) {
        switch (local_retval) {
            case P4_E_PERM:         // if the caller does not have oflags access rights to file name. if the caller tries to open a file on
                                    // a volume provider which is not yet mounted. if VM_O_WRLOCK was used and the file has already
                                    // been opened for writing. if writing was requested and the file has already been opened using another
                                    // descriptor in conjunction with VM_O_WRLOCK.
                                    // Note: for opening files in directories of a gate provider, VM_E_EXEC permissions are required, or else
                                    // P4_E_PERM will be returned. Also note that a gate with an empty name "" will be interpreted as the root di-
                                    // rectory, so any file name for which no explicit other gate is found will cause the root directory to be indexed.
            case P4_E_RESTRICTED:   // if VM_O_WR is given in oflags but the file system is write protected. This may be the
                                    // case if it was not mounted for writing or it resides on a read-only storage device.
                ret = OSAL_ERR_PERMISSION_DENIED;
                break;
            case P4_E_NOTIMPL:      // if the file system is not capable to access file name with the access rights oflags.
                ret = OSAL_ERR_NOT_IMPLEMENTED;
                break;
            case P4_E_NAME:         // if name is too long for the underlying provider.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case P4_E_NOENT:        // if a file with the name name does not exist.
                ret = OSAL_ERR_NOT_FOUND;
                break;
            case P4_E_INVAL:        // if a parameter is invalid.
                ret = OSAL_ERR_INVALID_PARAM;
                break;
            case P4_E_OOFILE:       // if no free file descriptor can be allocated from the partitions file descriptor pool.
            case P4_E_NOCONTAINER:  // if a component of the path prefix, except for the last component, of name is not a
                                    // volume or directory.
            case P4_E_CANCEL:       // if the call was canceled.
            case P4_E_LIMIT:        // if there is not enough space available on the volume.
                ret = OSAL_ERR_SYSTEM_LIMIT_REACHED;
                break;
            case P4_E_MISMATCH:     // if name is an existing directory.
            case P4_E_EXIST:        // if VM_O_CREAT and VM_O_EXCL was used and name is an existing file.
            case P4_E_IO:           // if the storage device containing the file reports a failure.
                ret = OSAL_ERR_OPERATION_FAILED;
                break;
            case P4_E_TIMEOUT:      // if VM_O_NONBLOCK was used, the driver needs to block, and the driver supports
                                    // VM_O_NONBLOCK.
                ret = OSAL_ERR_TIMEOUT;
                break;
        }
    }

    return ret;
}

//! \brief Map a shm.
/*!
 * \param[in]   shm     Pointer to osal shm structure. Content is OS dependent.
 * \param[in]   size    Size of memory to map.
 * \param[in]   attr    Pointer to map attributes.
 * \param[out]  ptr     Pointer where to returned mapped data pointer.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_shm_map(osal_shm_t *shm, const osal_size_t size, const osal_shm_map_attr_t *attr, osal_void_t **ptr) {
    assert(shm != NULL);
    assert(ptr != NULL);
    osal_retval_t ret = OSAL_OK;

    /* Allocate virtual memory to map the shared memory. Enforce an PAGE alignment */
    *ptr = (struct comm_str *)p4ext_vmem_alloc_aligned(size, (P4_phys_addr_t)P4_PAGESIZE);
    if (*ptr == NULL) {
        ret = OSAL_ERR_OUT_OF_MEMORY;
    }

    if (ret == OSAL_OK) {
        int local_retval = vm_map(&shm->fd, 0, size, VM_MEM_ACCESS_RD_WR, 0, (P4_address_t)*ptr);
        if (local_retval != P4_E_OK) {
            switch (local_retval) {
                case P4_E_PERM:     // if the partition does not have prot access rights to file fd.
                    ret = OSAL_ERR_PERMISSION_DENIED;
                    break;
                case P4_E_NOTIMPL:  // if the responsible provider does not support this operation.
                case P4_E_SIZE:     // if size is invalid
                case P4_E_OVERFLOW: // if offset causes an arithmetic overflow
                case P4_E_INVAL:    // if the file is not suited for memory mapping or
                                    // if a parameter is invalid, e.g., if fd is not a valid file descriptor. Note that uninitialized descriptors
                                    // cannot reliably be identified as invalid.
                    ret = OSAL_ERR_INVALID_PARAM;
                    break;
                case P4_E_TIMEOUT:  // if the file was opened with a VM_O_NONBLOCK flag, and the driver supports non-
                                    // blocking operation, and if the operation would need to block to be performed, then instead of blocking, 
                                    // P4_E_TIMEOUT will be returned.
                    ret = OSAL_ERR_TIMEOUT;
                    break;
                case P4_E_NOKMEM:   // if the file could not be mapped completely because of an insufficient. amount of free kernel
                                    // memory.
                    ret = OSAL_ERR_OUT_OF_MEMORY;
                    break;
                case P4_E_CANCEL:   // if the call was canceled.
                    ret = OSAL_ERR_OPERATION_FAILED;
                    break;
            }
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

    return ret;
}


