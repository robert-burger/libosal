/**
 * \file pikeos/shm.h
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

#ifndef LIBOSAL_PIKEOS_SHM__H
#define LIBOSAL_PIKEOS_SHM__H

#include <vm.h>

typedef struct osal_shm {
    vm_file_desc_t fd;
    osal_size_t size;
} osal_shm_t;

#endif /* LIBOSAL_PIKEOS_SHM__H */

