/**
 * \file shm.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL shm header.
 *
 * OSAL shm include header.
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

#ifndef LIBOSAL_SHM__H
#define LIBOSAL_SHM__H

#include <libosal/config.h>
#include <libosal/types.h>

#ifdef LIBOSAL_BUILD_POSIX
#include <libosal/posix/shm.h>
#endif

#ifdef LIBOSAL_BUILD_VXWORKS
#include <libosal/vxworks/shm.h>
#endif

#ifdef LIBOSAL_BUILD_PIKEOS
#include <libosal/pikeos/shm.h>
#endif

#define OSAL_SHM_ATTR__FLAG__MASK             0x0000003Fu
#define OSAL_SHM_ATTR__FLAG__RDONLY           0x00000001u
#define OSAL_SHM_ATTR__FLAG__RDWR             0x00000002u
#define OSAL_SHM_ATTR__FLAG__CREAT            0x00000004u
#define OSAL_SHM_ATTR__FLAG__EXCL             0x00000008u
#define OSAL_SHM_ATTR__FLAG__TRUNC            0x00000010u
#define OSAL_SHM_ATTR__FLAG__MAP              0x00000020u

#define OSAL_SHM_ATTR__MODE__MASK             0xFFFF0000u
#define OSAL_SHM_ATTR__MODE__SHIFT            16u

typedef osal_uint32_t osal_shm_attr_t;
typedef osal_uint32_t osal_shm_map_attr_t;

#ifdef __cplusplus
extern "C" {
#endif

//! \brief Initialize a shm.
/*!
 * \param[in]   shm     Pointer to osal shm structure. Content is OS dependent.
 * \param[in]   attr    Pointer to initial shm attributes. Can be NULL then
 *                      the defaults of the underlying shm will be used.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_shm_open(osal_shm_t *shm, const osal_char_t *name,  const osal_shm_attr_t *attr);

#define OSAL_SHM_MAP_ATTR__PROT_EXEC          0x00000001u
#define OSAL_SHM_MAP_ATTR__PROT_READ          0x00000002u
#define OSAL_SHM_MAP_ATTR__PROT_WRITE         0x00000004u
#define OSAL_SHM_MAP_ATTR__PROT_NONE          0x00000008u

#define OSAL_SHM_MAP_ATTR__SHARED             0x00000100u
#define OSAL_SHM_MAP_ATTR__PRIVATE            0x00000200u

//! \brief Map a shm.
/*!
 * \param[in]   shm     Pointer to osal shm structure. Content is OS dependent.
 * \param[in]   size    Size of memory to map.
 * \param[in]   attr    Pointer to map attributes.
 * \param[out]  ptr     Pointer where to returned mapped data pointer.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_shm_map(osal_shm_t *shm, const osal_size_t size, const osal_shm_map_attr_t *attr, osal_void_t **ptr);

//! \brief Closes an open shm.
/*!
 * \param[in]   shm     Pointer to osal shm structure. Content is OS dependent.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_shm_close(osal_shm_t *shm);

#ifdef __cplusplus
};
#endif

#endif /* LIBOSAL_SHM__H */


