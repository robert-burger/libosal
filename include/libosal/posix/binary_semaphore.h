/**
 * \file binary_semaphore.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL binary_semaphore header.
 *
 * OSAL binary_semaphore include header.
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

#ifndef LIBOSAL_POSIX_BINARY_SEMAPHORE__H
#define LIBOSAL_POSIX_BINARY_SEMAPHORE__H

#include <pthread.h>

typedef struct osal_binary_semaphore {
    pthread_mutex_t posix_mtx;
    pthread_cond_t posix_cond;
    int value;
} osal_binary_semaphore_t;

#endif /* LIBOSAL_POSIX_BINARY_SEMAPHORE__H */
