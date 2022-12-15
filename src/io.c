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
#include <libosal/shm.h>
#include <libosal/mutex.h>
#include <libosal/semaphore.h>

#include <assert.h>
        
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define LIBOSAL_IO_SHM_SIZE         0x100000
#define LIBOSAL_IO_SHM_MAGIC        0x00AFFE00
#define LIBOSAL_IO_SHM_MAX_MSG_SIZE 512
#define LIBOSAL_IO_SHM_MAX_MSGS     100

typedef struct osal_io_shm {
	osal_mutex_t mtx;
	osal_semaphore_t sem;
	osal_bool_t new_msg;
	osal_uint32_t magic;

    osal_uint32_t act_printed;
    osal_uint32_t act_written;
	char msg[LIBOSAL_IO_SHM_MAX_MSGS][LIBOSAL_IO_SHM_MAX_MSG_SIZE];
} osal_io_shm_t;

static osal_shm_t osal_io_shm;
static osal_io_shm_t *osal_io_shm_buffer = NULL;

// Get next message printed to shm.
osal_retval_t osal_io_shm_get_message(osal_char_t msg[LIBOSAL_IO_SHM_MAX_MSG_SIZE],
        const osal_timer_t *to)
{
    osal_retval_t ret = OSAL_ERR_UNAVAILABLE;

    if (osal_io_shm_buffer->act_printed == osal_io_shm_buffer->act_written) {
        if (to != NULL) {
            (void)osal_semaphore_timedwait(&osal_io_shm_buffer->sem, to);
        }
    }

    if (osal_io_shm_buffer->act_printed != osal_io_shm_buffer->act_written) {
        osal_io_shm_buffer->act_printed = (osal_io_shm_buffer->act_printed + 1) % LIBOSAL_IO_SHM_MAX_MSGS;
        strncpy(msg, osal_io_shm_buffer->msg[osal_io_shm_buffer->act_printed], LIBOSAL_IO_SHM_MAX_MSG_SIZE);
        ret = OSAL_OK;
    }

    return ret;
}

osal_retval_t osal_io_shm_setup(const osal_char_t *shm_name) {
    osal_shm_attr_t shm_attr_msr = OSAL_SHM_ATTR__FLAG__RDWR | OSAL_SHM_ATTR__FLAG__MAP | 
        OSAL_SHM_ATTR__FLAG__CREAT;
    shm_attr_msr |= 0666 << OSAL_SHM_ATTR__MODE__SHIFT;
    osal_retval_t local_retval = osal_shm_open(&osal_io_shm, shm_name, &shm_attr_msr);
    if (local_retval != OSAL_OK) {
        osal_printf("osal_shm_open(%p, %s, %p) returned error: %d\n", 
                &osal_io_shm, shm_name, &shm_attr_msr, local_retval);
    } else {
        osal_void_t *tmp = NULL;
        osal_shm_map_attr_t map_attr;
        map_attr = OSAL_SHM_MAP_ATTR__PROT_WRITE | OSAL_SHM_MAP_ATTR__PROT_READ | OSAL_SHM_MAP_ATTR__SHARED;
        local_retval = osal_shm_map(&osal_io_shm, LIBOSAL_IO_SHM_SIZE, &map_attr, (osal_void_t **)&tmp);
        if (local_retval != OSAL_OK) {
            osal_printf("osal_shm_map(%p, %d, %p) returned error: %d\n", &osal_io_shm, LIBOSAL_IO_SHM_SIZE, &tmp, local_retval);
        } else {
            osal_printf("SHM opened and mapped successfully!\n");
            osal_io_shm_buffer = (osal_io_shm_t *)tmp;
    
            osal_printf("%X, %d %d\n", osal_io_shm_buffer->magic, osal_io_shm_buffer->act_printed, osal_io_shm_buffer->act_written);
            if (osal_io_shm_buffer->magic == LIBOSAL_IO_SHM_MAGIC) {
                osal_printf("found LIBOSAL_IO_SHM_MAGIC, skipping initialization.\n");
            } else {
                osal_io_shm_buffer->act_printed = 0;
                osal_io_shm_buffer->act_written = 0;

                osal_mutex_attr_t tmp_mutex_attr = OSAL_MUTEX_ATTR__ROBUST | OSAL_MUTEX_ATTR__PROCESS_SHARED;
                osal_mutex_init(&osal_io_shm_buffer->mtx, &tmp_mutex_attr);

                osal_semaphore_attr_t tmp_semaphore_attr = OSAL_SEMAPHORE_ATTR__PROCESS_SHARED;
                osal_semaphore_init(&osal_io_shm_buffer->sem, &tmp_semaphore_attr, 0);

                osal_io_shm_buffer->magic = LIBOSAL_IO_SHM_MAGIC;
                osal_io_shm_buffer->new_msg = 0;

                for (int i = 0; i < LIBOSAL_IO_SHM_MAX_MSGS; ++i) {
                    osal_io_shm_buffer->msg[i][0] = '\0';
                }
            }
        }
    }

    return OSAL_OK;
}

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

    if (osal_io_shm_buffer != NULL) {
        osal_uint32_t next_write = (osal_io_shm_buffer->act_written + 1) % LIBOSAL_IO_SHM_MAX_MSGS;
        char *tmp = &osal_io_shm_buffer->msg[next_write][0];

        if (next_write == osal_io_shm_buffer->act_printed) {
            osal_io_shm_buffer->act_printed = (osal_io_shm_buffer->act_printed + 1) % LIBOSAL_IO_SHM_MAX_MSGS;
        }

        strncpy(tmp, buf, LIBOSAL_IO_SHM_MAX_MSG_SIZE);
        osal_io_shm_buffer->act_written = next_write;
        osal_semaphore_post(&osal_io_shm_buffer->sem);
    } else {
        (void)osal_puts(buf);
    }

    return ret;
}

