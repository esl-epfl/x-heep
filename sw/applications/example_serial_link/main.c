#include <stdio.h>
#include <stdlib.h>

#include "x-heep.h"
#include "core_v_mini_mcu.h"
#include "serial_link_regs.h"

int main(int argc, char *argv[])
{
    /* write something to stdout */
    printf("start!\n");
    int32_t *addr_p = 0x50000040; // bus serial link

    int32_t *addr_p_reg =SERIAL_LINK_START_ADDRESS + SERIAL_LINK_CTRL_REG_OFFSET; // register
    *addr_p_reg = (*addr_p_reg)| 0x00000001;
    printf("writing to %p started\n", addr_p);
    *addr_p = 2345;
    
    printf("addr_p = %p -> %x\n", addr_p, *addr_p);

    printf("success!\n");
    return EXIT_SUCCESS;
}