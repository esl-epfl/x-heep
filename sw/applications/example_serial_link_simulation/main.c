#include <stdio.h>
#include <stdlib.h>

#include "x-heep.h"
#include "core_v_mini_mcu.h"
#include "serial_link_single_channel_regs.h" 
#include "serial_link_regs.h"
#include "serial_link.h"
#include "csr.h"



int32_t NUM_TO_CHECK = 429496729;
int32_t NUM_TO_BE_CHECKED;
int main(int argc, char *argv[])
{

    //volatile int32_t *addr_p = 0x50000040;
    volatile int32_t *addr_p_external = SL_EXTERNAL_WRITE;
    volatile int32_t *addr_p_recreg = SL_INTERNAL_READ;
    //*addr_p_recreg = NUM_TO_CHECK;
    //printf("START\n");
    REG_CONFIG();
    //printf("REG CONFIG DONE\n");
    AXI_ISOLATE();
    //printf("AXI ISOLATE DONE\n");
    EXTERNAL_BUS_SL_CONFIG();
    
    *addr_p_external = NUM_TO_CHECK;
    while(1){
    if (*addr_p_recreg ==NUM_TO_CHECK){
        
        break;
        }
    }

    printf("DONE\n");
    
    return EXIT_SUCCESS;
}

