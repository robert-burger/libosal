/**
 * \file posix/io.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 * \author Martin Stelzer <martin.stelzer@dlr.de>
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

#include <libosal/config.h>
#include <libosal/osal.h>
#include <libosal/io.h>

#include <assert.h>
        
// cppcheck-suppress misra-c2012-21.6
#include <stdio.h>
#include <stdarg.h>

//! \brief Get the current state of a created thread.
/*!
 * \param[in]   fmt     Print format.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_printf(const osal_char_t *fmt, ...) {
    assert(fmt != NULL);

    char buf[512];

    // cppcheck-suppress misra-c2012-17.1
    va_list va;
    osal_retval_t ret = OSAL_OK;

    // cppcheck-suppress misra-c2012-17.1
    va_start(va, fmt);

    (void)vsnprintf(buf, 512, fmt, va);
    
    // cppcheck-suppress misra-c2012-17.1
    va_end(va);

    (void)fprintf(stdout, "%s", buf);

    return ret;
}



