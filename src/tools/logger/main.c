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
            printf(msg);
        }
    }

    return 0;
}
