#include <stdio.h>
#include <stdlib.h>

#include "x-heep.h"
#include "core_v_mini_mcu.h"
#include "serial_link_single_channel_regs.h" 
#include "serial_link_regs.h"
#include "csr.h"


int32_t NUM_TO_CHECK = 429496729;
int32_t NUM_TO_BE_CHECKED;
int main(int argc, char *argv[])
{

    //volatile int32_t *addr_p = 0x50000040;
    volatile int32_t *addr_p_external = (int32_t *)(EXT_SLAVE_START_ADDRESS + 0x20000);
    volatile int32_t *addr_p_recreg = (int32_t *)(SERIAL_LINK_RECEIVER_FIFO_START_ADDRESS);
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




void __attribute__ ((optimize("00"))) READ_SL(void){
    volatile int32_t *addr_p_external = 0xF0010000;
    while(1){
    if (*addr_p_external ==NUM_TO_CHECK){
        
        break;
    }}
}

void __attribute__ ((optimize("00"))) REG_CONFIG(void){
    volatile int32_t *addr_p_reg =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); 
    *addr_p_reg = (*addr_p_reg)| 0x00000001; // clock enable
    *addr_p_reg = (*addr_p_reg)& 0x11111101; // rst on
    *addr_p_reg = (*addr_p_reg)| 0x00000002; // rst oFF
}


void __attribute__ ((optimize("00"))) REG_CONFIG_MULTI(void){
    volatile int32_t *addr_p_reg =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_CTRL_REG_OFFSET); 
    *addr_p_reg = (*addr_p_reg)| 0x00000001; // clock enable
    *addr_p_reg = (*addr_p_reg)& 0x11111101; // rst on
    *addr_p_reg = (*addr_p_reg)| 0x00000002; // rst oFF
}

void __attribute__ ((optimize("00"))) RAW_MODE_EN(void){
    int32_t *addr_p_reg_RAW_MODE =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_EN_REG_OFFSET); 
    *addr_p_reg_RAW_MODE = (*addr_p_reg_RAW_MODE)| 0x00000001; // raw mode en

    int32_t *addr_p_RAW_MODE_IN_CH_SEL_REG =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_IN_CH_SEL_REG_OFFSET); 
    //*addr_p_RAW_MODE_IN_CH_SEL_REG = (*addr_p_RAW_MODE_IN_CH_SEL_REG)| 0x00000001; // raw mode select channel 

    int32_t *addr_p_RAW_MODE_OUT_CH_MASK_REG =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_OUT_CH_MASK_REG_OFFSET); 
    *addr_p_RAW_MODE_OUT_CH_MASK_REG= (*addr_p_RAW_MODE_OUT_CH_MASK_REG)| 0x00000008; // raw mode mask

    int32_t *addr_p_RAW_MODE_OUT_DATA_FIFO_REG =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_OUT_DATA_FIFO_REG_OFFSET); 
    *addr_p_RAW_MODE_OUT_DATA_FIFO_REG = (*addr_p_RAW_MODE_OUT_DATA_FIFO_REG)| 0x00000001;

    int32_t *addr_p_RAW_MODE_OUT_DATA_FIFO_CTRL_REG =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_OUT_DATA_FIFO_CTRL_REG_OFFSET); 
    *addr_p_RAW_MODE_OUT_DATA_FIFO_CTRL_REG = (*addr_p_RAW_MODE_OUT_DATA_FIFO_CTRL_REG)| 0x00000001;

    int32_t *addr_p_RAW_MODE_OUT_EN_REG =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_OUT_EN_REG_OFFSET); 
    *addr_p_RAW_MODE_OUT_EN_REG = (*addr_p_RAW_MODE_OUT_EN_REG)| 0x00000001; 

    int32_t *addr_p_RAW_MODE_IN_DATA_REG =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_IN_DATA_REG_OFFSET); 
    *addr_p_RAW_MODE_IN_DATA_REG = (*addr_p_RAW_MODE_IN_DATA_REG)| 0x00000001; 
}

void __attribute__ ((optimize("00"))) AXI_ISOLATE(void){
    int32_t *addr_p_reg_ISOLATE_IN =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); 

    //*addr_p_reg_ISOLATE_IN = (*addr_p_reg_ISOLATE_IN)& (8 << 0); // axi_in_isolate
    *addr_p_reg_ISOLATE_IN &= ~(1<<8);
    int32_t *addr_p_reg_ISOLATE_OUT =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET);
    *addr_p_reg_ISOLATE_OUT &= ~(1<<9); // axi_out_isolate
    }

void __attribute__ ((optimize("00"))) EXTERNAL_BUS_SL_CONFIG(void){
    // /*                     -------                     */
    // /*  SL TESTHARNESS EXTERNAL BUS X-heep system      */


    // /*  REG CONFIG                   */
    // /*  CTRL register                */
    volatile int32_t *addr_p_reg_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x06000 + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); //0x06000000 
    *addr_p_reg_ext = (*addr_p_reg_ext)| 0x00000001; // ctrl clock enable external

    *addr_p_reg_ext = (*addr_p_reg_ext)& 0x11111101; // rst on
    *addr_p_reg_ext = (*addr_p_reg_ext)| 0x00000002; // rst oFF


    // //int32_t *addr_p_reg_flow_ctrl_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x06000  + SERIAL_LINK_SINGLE_CHANNEL_FLOW_CONTROL_FIFO_CLEAR_REG_OFFSET); //0x06000000 
    // //*addr_p_reg_flow_ctrl_ext = (*addr_p_reg_flow_ctrl_ext)& 0x11111110;

    // /*  AXI ISOLATE                   */ 
    // all channels are isolated by default 
    int32_t *addr_p_reg_ISOLATE_IN_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x06000 + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); 
    *addr_p_reg_ISOLATE_IN_ext &= ~(1<<8);
    int32_t *addr_p_reg_ISOLATE_OUT_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x06000 + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET);
    *addr_p_reg_ISOLATE_OUT_ext &= ~(1<<9);


    }