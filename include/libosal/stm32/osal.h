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

#ifndef LIBOSAL_STM32_OSAL__H
#define LIBOSAL_STM32_OSAL__H

#include <cmsis_compiler.h>

#define DECLARE_CRITICAL_SECTION() uint32_t __primask
#define ENTER_CRITICAL_SECTION()    \
    __primask = __get_PRIMASK();    \
    __disable_irq();

#define LEAVE_CRITICAL_SECTION()    \
    if (__primask == 0) {           \
        __enable_irq();             \
    }

#endif /* LIBOSAL_STM32_OSAL__H */


