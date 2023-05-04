/**
 * \file osal.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 29 Mar 2023
 *
 * \brief OSAL init source.
 *
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

#include <libosal/osal.h>

//! Initialize OSAL internals.
void __attribute__((constructor, weak)) osal_init(void) {
}

//! Destroy OSAL internals.
void __attribute__((destructor, weak)) osal_destroy(void) {
}
