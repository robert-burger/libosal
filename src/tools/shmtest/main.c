/**
 * \file main.c
 *
 * \author Robert Burger <robert.burger@dlr.de>
 *
 * \date 07 Aug 2022
 *
 * \brief OSAL main.
 *
 * OSAL main.
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
#include <libosal/io.h>
#include <libosal/shm.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
    
extern int main(int argc, char **argv) {
    osal_retval_t local_ret;
    osal_shm_t shm;
    osal_shm_attr_t shm_attr;
    osal_shm_map_attr_t shm_map_attr;
    osal_void_t *shm_mem;

    if (argc < 2) {
        osal_printf("usage: %s <shm_name>\n", argv[0]);
        return 0;
    }

    shm_attr = OSAL_SHM_ATTR__FLAG__CREAT | OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__MAP;
    shm_attr |= 0666 << OSAL_SHM_ATTR__MODE__SHIFT;
    local_ret = osal_shm_open(&shm, argv[1], &shm_attr, 0x1000);

    osal_printf("opened shm: fd %d, local_ret %d\n", shm.fd, local_ret);

    shm_map_attr = OSAL_SHM_MAP_ATTR__SHARED | OSAL_SHM_MAP_ATTR__PROT_READ | OSAL_SHM_MAP_ATTR__PROT_WRITE;
    local_ret = osal_shm_map(&shm, &shm_map_attr, &shm_mem);

    osal_printf("mapped shared memory: %p, local_ret %d\n", shm_mem, local_ret);
    osal_printf("content: %s\n", shm_mem);
    sprintf(shm_mem, "Hallo shared memory!");
    while (0) {
        osal_timer_t to;
        (void)osal_timer_init(&to, 10000000);
    }

    (void)osal_shm_close(&shm);

    return 0;
}

