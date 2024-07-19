#include <stdio.h>
#include <stdlib.h>

#include "x-heep.h"
#include "core_v_mini_mcu.h"
#include "serial_link_single_channel_regs.h"
#include "serial_link_regs.h"
#include "csr.h"


int32_t NUM_TO_CHECK = 429496729;
int32_t NUM_TO_BE_CHECKED;
void WRITE_SL(void);

int main(int argc, char *argv[])
{

    volatile int32_t *addr_p = 0x50000040;
    volatile int32_t *addr_p_external = 0xF0010000;


    REG_CONFIG();
    AXI_ISOLATE();

    EXTERNAL_BUS_SL_CONFIG();
    
    unsigned int cycles1,cycles2;
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);

    *addr_p = NUM_TO_CHECK;
    CSR_WRITE(CSR_REG_MCYCLE, 0);
    while(1){
    if (*addr_p_external ==NUM_TO_CHECK){
        CSR_READ(CSR_REG_MCYCLE, &cycles1);
        
        break;
        }
    }
    *addr_p = 6550039;
     while(1){
    if (*addr_p_external ==6550039){
        //CSR_READ(CSR_REG_MCYCLE, &cycles1);
        
        break;
        }
    }
    *addr_p = 38734;
         while(1){
    if (*addr_p_external ==38734){
        //CSR_READ(CSR_REG_MCYCLE, &cycles1);
        
        break;
        }
    }
    *addr_p = 378518920;
             while(1){
    if (*addr_p_external ==378518920){
        //CSR_READ(CSR_REG_MCYCLE, &cycles1);
        //CSR_READ(CSR_REG_MCYCLE, &cycles1);
        break;
        }
    }
    printf("first write finished with  %d cycles\n\r", cycles1);
    printf("DONE\n");
    
    return EXIT_SUCCESS;
}

void __attribute__ ((optimize("00"))) WRITE_SL(void){

    volatile int32_t *addr_p = 0x50000040;
    *addr_p = NUM_TO_CHECK;

    //printf("asd\n");     
    
    //*addr_p = NUM_TO_CHECK;
    //*addr_p = 28;
    //*addr_p = 47;
    //*addr_p = 5;
    //*addr_p = 3;
    //printf("writing %d  to %p \n",*addr_p, addr_p);
}


void __attribute__ ((optimize("00"))) READ_SL(void){
    volatile int32_t *addr_p_external = 0xF0010000;// bus serial link from mcu_cfg.hjson
    //NUM_TO_BE_CHECKED= *addr_p_external ;
    while(1){
    if (*addr_p_external ==NUM_TO_CHECK){
        
        break;
    }}
    //printf("addr_p_ext = %p -> %d\n", addr_p_external, *addr_p_external);
}

void __attribute__ ((optimize("00"))) REG_CONFIG(void){
    volatile int32_t *addr_p_reg =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); 
    *addr_p_reg = (*addr_p_reg)| 0x00000001; // clock enable
     //printf("addr_p %x\n", *addr_p_reg);
    
    *addr_p_reg = (*addr_p_reg)& 0x11111101; // rst on
    *addr_p_reg = (*addr_p_reg)| 0x00000002; // rst oFF


    //int32_t *addr_p_reg_flow_ctrl =(int32_t *)(SERIAL_LINK_START_ADDRESS  + SERIAL_LINK_SINGLE_CHANNEL_FLOW_CONTROL_FIFO_CLEAR_REG_OFFSET); //0x04000000 
    //*addr_p_reg_flow_ctrl = (*addr_p_reg_flow_ctrl)& 0x00000001; //0x11111110;
}


void __attribute__ ((optimize("00"))) REG_CONFIG_MULTI(void){
    volatile int32_t *addr_p_reg =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_CTRL_REG_OFFSET); 
    *addr_p_reg = (*addr_p_reg)| 0x00000001; // clock enable
     //printf("addr_p %x\n", *addr_p_reg);
    
    *addr_p_reg = (*addr_p_reg)& 0x11111101; // rst on
    *addr_p_reg = (*addr_p_reg)| 0x00000002; // rst oFF


    //int32_t *addr_p_reg_flow_ctrl =(int32_t *)(SERIAL_LINK_START_ADDRESS  + SERIAL_LINK_SINGLE_CHANNEL_FLOW_CONTROL_FIFO_CLEAR_REG_OFFSET); //0x04000000 
    //*addr_p_reg_flow_ctrl = (*addr_p_reg_flow_ctrl)& 0x00000001; //0x11111110;
}

void __attribute__ ((optimize("00"))) RAW_MODE_EN(void){
    int32_t *addr_p_reg_RAW_MODE =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_EN_REG_OFFSET); 
    *addr_p_reg_RAW_MODE = (*addr_p_reg_RAW_MODE)| 0x00000001; // raw mode en

    int32_t *addr_p_RAW_MODE_IN_CH_SEL_REG =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_IN_CH_SEL_REG_OFFSET); 
    //*addr_p_RAW_MODE_IN_CH_SEL_REG = (*addr_p_RAW_MODE_IN_CH_SEL_REG)| 0x00000001; // raw mode select channel 

    int32_t *addr_p_RAW_MODE_OUT_CH_MASK_REG =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_OUT_CH_MASK_REG_OFFSET); 
    *addr_p_RAW_MODE_OUT_CH_MASK_REG= (*addr_p_RAW_MODE_OUT_CH_MASK_REG)| 0x00000008; // raw mode mask

    int32_t *addr_p_RAW_MODE_OUT_DATA_FIFO_REG =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_OUT_DATA_FIFO_REG_OFFSET); 
    *addr_p_RAW_MODE_OUT_DATA_FIFO_REG = (*addr_p_RAW_MODE_OUT_DATA_FIFO_REG)| 0x00000001;

    int32_t *addr_p_RAW_MODE_OUT_DATA_FIFO_CTRL_REG =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_OUT_DATA_FIFO_CTRL_REG_OFFSET); 
    *addr_p_RAW_MODE_OUT_DATA_FIFO_CTRL_REG = (*addr_p_RAW_MODE_OUT_DATA_FIFO_CTRL_REG)| 0x00000001;

    int32_t *addr_p_RAW_MODE_OUT_EN_REG =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_OUT_EN_REG_OFFSET); 
    *addr_p_RAW_MODE_OUT_EN_REG = (*addr_p_RAW_MODE_OUT_EN_REG)| 0x00000001; 

    int32_t *addr_p_RAW_MODE_IN_DATA_REG =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_IN_DATA_REG_OFFSET); 
    *addr_p_RAW_MODE_IN_DATA_REG = (*addr_p_RAW_MODE_IN_DATA_REG)| 0x00000001; 
}

void __attribute__ ((optimize("00"))) AXI_ISOLATE(void){
    int32_t *addr_p_reg_ISOLATE_IN =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); 

    //*addr_p_reg_ISOLATE_IN = (*addr_p_reg_ISOLATE_IN)& (8 << 0); // axi_in_isolate
    *addr_p_reg_ISOLATE_IN &= ~(1<<8);
    int32_t *addr_p_reg_ISOLATE_OUT =(int32_t *)(SERIAL_LINK_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET);
    *addr_p_reg_ISOLATE_OUT &= ~(1<<9); // axi_out_isolate
    }

void __attribute__ ((optimize("00"))) EXTERNAL_BUS_SL_CONFIG(void){
    // /*                     -------                     */
    // /*  SL TESTHARNESS EXTERNAL BUS X-heep system      */


    // /*  REG CONFIG                   */
    // /*  CTRL register                */
    volatile int32_t *addr_p_reg_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x04000 + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); //0x04000000 
    *addr_p_reg_ext = (*addr_p_reg_ext)| 0x00000001; // ctrl clock enable external

    *addr_p_reg_ext = (*addr_p_reg_ext)& 0x11111101; // rst on
    *addr_p_reg_ext = (*addr_p_reg_ext)| 0x00000002; // rst oFF


    // //int32_t *addr_p_reg_flow_ctrl_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x04000  + SERIAL_LINK_SINGLE_CHANNEL_FLOW_CONTROL_FIFO_CLEAR_REG_OFFSET); //0x04000000 
    // //*addr_p_reg_flow_ctrl_ext = (*addr_p_reg_flow_ctrl_ext)& 0x11111110;

    // /*  AXI ISOLATE                   */ 
    // all channels are isolated by default 
    int32_t *addr_p_reg_ISOLATE_IN_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x04000 + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); 
    *addr_p_reg_ISOLATE_IN_ext &= ~(1<<8);
    int32_t *addr_p_reg_ISOLATE_OUT_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x04000 + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET);
    *addr_p_reg_ISOLATE_OUT_ext &= ~(1<<9);

    // //int32_t *addr_p_reg_RAW_MODE_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x04000  + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_EN_REG_OFFSET); 
    // //*addr_p_reg_RAW_MODE_ext = (*addr_p_reg_RAW_MODE_ext)| 0x00000001; // raw mode en
    // //int32_t *addr_p_RAW_MODE_IN_CH_SEL_REG_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x04000 + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_IN_DATA_REG_OFFSET); 
    // //*addr_p_RAW_MODE_IN_CH_SEL_REG_ext = (*addr_p_RAW_MODE_IN_CH_SEL_REG_ext)| 0x00000001; // raw mode select channel 



    }