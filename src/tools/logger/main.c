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

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
    
extern int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: %s <shm_name>\n", argv[0]);
        return 0;
    }

    printf("SHM logger\n");
    //printf("max messages: %d\n", LIBOSAL_IO_SHM_MAX_MSGS);
    //printf("max message size: %d\n", LIBOSAL_IO_SHM_MAX_MSG_SIZE);

    osal_io_shm_setup(argv[1], 1000, 512);

    osal_char_t msg[LIBOSAL_IO_SHM_MAX_MSG_SIZE];

    while (1) {
        osal_timer_t to;
        (void)osal_timer_init(&to, 10000000);
        osal_retval_t ret = osal_io_shm_get_message(msg, &to);
        if (ret == OSAL_OK) {
            printf("%s\n", msg);
        }
    }

    return 0;
}
