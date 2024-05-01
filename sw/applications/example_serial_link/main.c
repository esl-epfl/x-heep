#include <stdio.h>
#include <stdlib.h>

#include "x-heep.h"
#include "core_v_mini_mcu.h"

int main(int argc, char *argv[])
{
    /* write something to stdout */
    
    int32_t *addr_p = 0x50000040;
    printf("writing to %p started\n", addr_p);
    *addr_p = 2345;
    
    printf("addr_p = %p -> %x\n", addr_p, *addr_p);

    printf("success!\n");
    return EXIT_SUCCESS;
}