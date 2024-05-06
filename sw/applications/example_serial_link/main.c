#include <stdio.h>
#include <stdlib.h>

#include "x-heep.h"
#include "core_v_mini_mcu.h"
#include "serial_link_regs.h"

int main(int argc, char *argv[])
{
    /*REG CONFIG*/ 

    int32_t *addr_p_reg =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_CTRL_REG_OFFSET); 
    //*addr_p_reg = (*addr_p_reg) & 0x11111101; // reset_n 
    *addr_p_reg = (*addr_p_reg)| 0x00000001; // clock enable
    
    // ROW MODE
    int32_t *addr_p_reg_RAW_MODE =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_RAW_MODE_EN_REG_OFFSET); 
    //*addr_p_reg = (*addr_p_reg)& 0x11111101;
    *addr_p_reg_RAW_MODE = (*addr_p_reg_RAW_MODE)| 0x00000001; // clock enable reg

    // AXI ISOLATE 
    // all channels are isolated by default
    //int32_t *addr_p_reg_RAW_MODE =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_CTRL_REG_OFFSET); 
    //*addr_p_reg_RAW_MODE = (*addr_p_reg_RAW_MODE)& 0x01111111; // axi_in_isolate

    //int32_t *addr_p_reg_RAW_MODE =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_CTRL_REG_OFFSET);
    //*addr_p_reg_RAW_MODE = (*addr_p_reg_RAW_MODE)| 0x00000001; // axi_out_isolate




    /* WRITING TO SL */
    printf("start!\n");
    int32_t *addr_p = 0x50000040; // bus serial link
    printf("writing to %p started\n", addr_p);
    *addr_p = 2345;
    
    printf("addr_p = %p -> %x\n", addr_p, *addr_p);

    



    printf("success!\n");
    return EXIT_SUCCESS;
}