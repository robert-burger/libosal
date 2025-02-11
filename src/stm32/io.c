/**
 * \file posix/io.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Sep 2022
 *
 * \brief OSAL io posix source.
 *
 * OSAL io posix source.
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
#ifdef HAVE_CONFIG_H
#include <libosal/config.h>
#endif

#include <libosal/osal.h>
#include <libosal/io.h>

#include <assert.h>
        
// cppcheck-suppress misra-c2012-21.6
#include <stdio.h>
#include "usart.h"


//! \brief Write message to stdout
/*!
 * \param[in]   msg     Message to be printed.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_puts(const osal_char_t *msg) {
    assert(msg != NULL);
    HAL_UART_Transmit(&huart1, (const uint8_t *)msg, strlen(&msg[0]), 10);
    return OSAL_OK;
}


osal_int32_t osal_vfprintf(osal_file_t *stream, const osal_char_t *format, osal_va_list_t ap) {
    return vfprintf((FILE *)stream, (char *)format, ap);
}
