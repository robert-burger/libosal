
/**
 * \file timer.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL timer header.
 *
 * OSAL timer include header.
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

#ifndef LIBOSAL_POSIX_TIMER__H
#define LIBOSAL_POSIX_TIMER__H

#include <time.h>

#define LIBOSAL_CLOCK_MONOTONIC     CLOCK_MONOTONIC
#define LIBOSAL_CLOCK_REALTIME      CLOCK_REALTIME

extern int global_clock_id;

#endif /* LIBOSAL_POSIX_TIMER__H */
