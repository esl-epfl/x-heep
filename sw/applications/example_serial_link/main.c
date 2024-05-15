#include <stdio.h>
#include <stdlib.h>

#include "x-heep.h"
#include "core_v_mini_mcu.h"
#include "serial_link_single_channel_regs.h"

int main(int argc, char *argv[])
{
    /*REG CONFIG*/ 

    int32_t *addr_p_reg =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); 
    //*addr_p_reg = (*addr_p_reg) & 0x11111101; // reset_n 
    *addr_p_reg = (*addr_p_reg)| 0x00000001; // clock enable

    int32_t *addr_p_reg_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x04000 + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); //0x04000000 
    //*addr_p_reg = (*addr_p_reg) & 0x11111101; // reset_n 
    *addr_p_reg_ext = (*addr_p_reg_ext)| 0x00000001; // clock enable external
    
    // RAW MODE
    //int32_t *addr_p_reg_RAW_MODE =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_RAW_MODE_EN_REG_OFFSET); 
    //*addr_p_reg = (*addr_p_reg)& 0x11111101;
    //*addr_p_reg_RAW_MODE = (*addr_p_reg_RAW_MODE)| 0x00000001; // raw mode en

    // AXI ISOLATE 
    // all channels are isolated by default *( will work only with original occamy wrapper)
    //int32_t *addr_p_reg_ISOLATE_IN =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); 
    //*addr_p_reg_ISOLATE_IN = (*addr_p_reg_ISOLATE_IN)& (0 << 8); // axi_in_isolate
    //int32_t *addr_p_reg_ISOLATE_OUT =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET);
    //*addr_p_reg_ISOLATE_OUT = (*addr_p_reg_ISOLATE_OUT)& (0 << 9); // axi_out_isolate




    /* WRITING TO SL */
    printf("start!\n");
    int32_t num_to_check = 13;
    int32_t *addr_p = 0x50000040; // bus serial link from hjson
    *addr_p = num_to_check;
    printf("writing %d  to %p \n",num_to_check, addr_p);
    //int32_t *addr_p_reg_ISOLATE_OUT =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET);
    //*addr_p_reg_ISOLATE_OUT = (*addr_p_reg_ISOLATE_OUT)& (0 << 9); // axi_out_isolate
    
    //printf("addr_p = %p -> %x\n", addr_p, *addr_p);
    /* READING FROM SL EXT */
    //int32_t *addr_p_ext = EXT_SLAVE_START_ADDRESS + 0x10000; 
    
    //printf("addr_p_ext = %p -> %x\n", addr_p_ext, *addr_p_ext);

    //if (*addr_p_reg_ext == *addr_p){
        printf("success!\n");
    //} else {

        printf(" go go power ranger, make it work\n");
   // }
    
    return EXIT_SUCCESS;
}