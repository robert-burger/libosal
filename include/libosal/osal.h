/**
 * \file osal.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL main header.
 *
 * OSAL main include header.
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

#ifndef LIBOSAL_OSAL__H
#define LIBOSAL_OSAL__H

#define OSAL_OK                         0
#define OSAL_ERR_OPERATION_FAILED       -1
#define OSAL_ERR_INVALID_PARAM          -2
#define OSAL_ERR_PERMISSION_DENIED      -3
#define OSAL_ERR_SYSTEM_LIMIT_REACHED   -4

#include <libosal/task.h>
#include <libosal/mutex.h>
#include <libosal/semaphore.h>
#include <libosal/binary_semaphore.h>

#endif /* LIBOSAL_OSAL__H */

