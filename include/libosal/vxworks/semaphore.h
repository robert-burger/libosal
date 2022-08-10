/**
 * \file vxworks/semaphore.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL semaphore header.
 *
 * OSAL semaphore include header.
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

#ifndef LIBOSAL_VXWORKS_SEMAPHORE__H
#define LIBOSAL_VXWORKS_SEMAPHORE__H

#include <vxWorks.h>
#include <semLib.h>

typedef struct osal_semaphore {
    SEM_ID vx_sem;
} osal_semaphore_t;

#endif /* LIBOSAL_VXWORKS_SEMAPHORE__H */


