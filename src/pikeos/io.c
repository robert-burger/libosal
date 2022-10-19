/**
 * \file pikeos/io.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 * \author Martin Stelzer <martin.stelzer@dlr.de>
 *
 * \date 07 Sep 2022
 *
 * \brief OSAL io pikeos source.
 *
 * OSAL io pikeos source.
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

#include <stand/string.h>
#include <assert.h>
#include <stdio.h>

#include <vm.h>
#ifdef PIKEOSDEBUG
/* init_gdbstub(), gdb_breakpoint(). */
#include <vm_debug.h>
#endif

static osal_mutex_t mutex_printf;
static int mutex_printf_init = 0;

//! \brief Get the current state of a created thread.
/*!
 * \param[in]   fmt     Print format.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_printf(const osal_char_t *fmt, ...) {
    assert(fmt != NULL);

    char buf[512];
    va_list va;
    osal_retval_t ret = OSAL_OK;

    if (mutex_printf_init == 0) {
        osal_mutex_init(&mutex_printf, NULL);
        mutex_printf_init = 1;
    }

    /* vm_cprintf is not reentrant */
    osal_mutex_lock(&mutex_printf);
    va_start(va, fmt);
    vsnprintf(buf, 512, fmt, va);
    va_end(va);

    vm_cprintf("%s", buf);
    osal_mutex_unlock(&mutex_printf);

    return ret;
}


