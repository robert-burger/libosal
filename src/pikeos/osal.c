/**
 * \file pikeos/osal.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 * \author Martin Stelzer <martin.stelzer@dlr.de>
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

#ifdef HAVE_CONFIG_H
#include <libosal/config.h>
#endif

#include <libosal/osal.h>
#include <libosal/io.h>

#include <vm.h>

#if 0 // i think this only works on pikeos > 5

//! Initialize OSAL internals.
void __attribute__((constructor)) osal_init(void) {
    P4_thr_t thrno = -1;
    P4_e_t result;

#ifdef PIKEOSDEBUG
    vm_init();
    init_gdbstub("muxa:/%s/%s/dbg");
    gdb_breakpoint();
#else
    vm_init();
#endif

    result = p4ext_thr_num_alloc(&thrno);
    if (result != P4_E_OK) {
        osal_printf("Failed to allocate thread number for ANIS: %d %s\n", result, p4_strerror(result));
    } else {
        result = anis_init("anisfp", thrno);
    }

    if (result < 0) {
        osal_printf("ANIS init error: %d %s\n", result, p4_strerror(result));
    }
}

//! Destroy OSAL internals.
void __attribute__((destructor)) osal_destroy(void) {
    vm_shutdown(VM_RESPART_MYSELF);
}

#endif

