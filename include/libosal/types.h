/**
 * \file types.h
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL types header.
 *
 * OSAL types include header.
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

#ifndef LIBOSAL_TYPES__H
#define LIBOSAL_TYPES__H

#ifdef HAVE_CONFIG_H
#include <libosal/config.h>
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

typedef uint32_t    osal_bool_t;        //!< \brief boolean type

#define OSAL_TRUE   ( 1u )              //!< \brief boolean true value.
#define OSAL_FALSE  ( 0u )              //!< \brief boolean false value.

typedef void        osal_void_t;        //!< \brief void type.

typedef int         osal_retval_t;      //!< \brief return value type.

typedef char        osal_char_t;        //!< \brief char (8-bit signed) type.

typedef uint64_t    osal_size_t;        //!< \brief size type.
typedef int64_t     osal_ssize_t;       //!< \brief signed size type.
typedef uint64_t    osal_off_t;         //!< \brief offset type.

typedef uint8_t     osal_uint8_t;       //!< \brief 8-bit unsigned type.
typedef uint16_t    osal_uint16_t;      //!< \brief 16-bit unsigned type.
typedef uint32_t    osal_uint32_t;      //!< \brief 32-bit unsigned type.
typedef uint64_t    osal_uint64_t;      //!< \brief 64-bit unsigned type.

typedef int8_t     osal_int8_t;         //!< \brief 8-bit signed type.
typedef int16_t    osal_int16_t;        //!< \brief 16-bit signed type.
typedef int32_t    osal_int32_t;        //!< \brief 32-bit signed type.
typedef int64_t    osal_int64_t;        //!< \brief 64-bit signed type.

typedef uint64_t   osal_mode_t;         //!< \brief mode type.

typedef FILE       osal_file_t;
typedef va_list    osal_va_list_t;

#endif /* LIBOSAL_TYPES__H */

