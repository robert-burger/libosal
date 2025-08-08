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
#include <libosal/mutex.h>

#include <stand/string.h>
#include <assert.h>

// cppcheck-suppress misra-c2012-21.6
#include <stdio.h>

#include <vm.h>
#ifdef PIKEOSDEBUG
/* init_gdbstub(), gdb_breakpoint(). */
#include <vm_debug.h>
#endif

//! \brief Write message to stdout
/*!
 * \param[in]   msg     Message to be printed.
 *
 * \return OK or ERROR_CODE.
 */
osal_retval_t osal_puts(const osal_char_t *msg) {
    assert(fmt != NULL);

    osal_retval_t ret = OSAL_OK;

    static osal_mutex_t mutex_printf;
    static int mutex_printf_init = 0;
    if (mutex_printf_init == 0) {
        (void)osal_mutex_init(&mutex_printf, NULL);
        mutex_printf_init = 1;
    }

    /* vm_cprintf is not reentrant */
    if (osal_mutex_lock(&mutex_printf) == OSAL_OK) {
        vm_cprintf("%s", msg);
        (void)osal_mutex_unlock(&mutex_printf);
    }

    return ret;
}

osal_int32_t osal_vfprintf(osal_file_t *stream, const osal_char_t *format, osal_va_list_t ap) {
#if __GNUC__ == 5
    (void)stream;

    // there's no vfprintf on pikeos-4.2
    osal_int32_t ret = 0;
    /*static*/ char v_tmp_buf[256];
    ret = vsnprintf(&v_tmp_buf[0], 256, format, ap);
    (void)osal_puts(v_tmp_buf);

    return ret;
#else 
    return vfprintf(stream, format, ap);
#endif
}
