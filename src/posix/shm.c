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

#include <assert.h>

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


