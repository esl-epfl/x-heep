

#include <stdio.h>
#include "serial_link.h"
#include "serial_link_single_channel_regs.h" 
#include "serial_link_regs.h"

// Adapted to be used with single channel


void __attribute__ ((optimize("00"))) SIM_INIT(void){
    REG_CONFIG();
    AXI_ISOLATE();
    EXTERNAL_BUS_SL_CONFIG();
}

void __attribute__ ((optimize("00"))) REG_CONFIG(void){
    volatile uint32_t * const ctrl = (volatile uint32_t *)CTRL_REG_ADDR;
    // Step 1: clock enabled, reset asserted (RESET_N = 0)
    *ctrl = CTRL_CLK_EN_MASK;
    // Step 2: clock enabled, reset de-asserted (RESET_N = 1)
    *ctrl = CTRL_CLK_EN_MASK | CTRL_RESET_N_MASK;
}


void __attribute__ ((optimize("00"))) REG_CONFIG_MULTI(void){
    volatile uint32_t * const ctrl = (volatile uint32_t *)CTRL_REG_ADDR_MULTI;
    // Step 1: clock enabled, reset asserted (RESET_N = 0)
    *ctrl = CTRL_CLK_EN_MASK;
    // Step 2: clock enabled, reset de-asserted (RESET_N = 1)
    *ctrl = CTRL_CLK_EN_MASK | CTRL_RESET_N_MASK;
}

void __attribute__ ((optimize("00"))) RAW_MODE_EN(void){
    int32_t *addr_p_reg_RAW_MODE =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_EN_REG_OFFSET); 
    *addr_p_reg_RAW_MODE = (*addr_p_reg_RAW_MODE)| 0x00000001; // raw mode en

    int32_t *addr_p_RAW_MODE_IN_CH_SEL_REG =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_RAW_MODE_IN_CH_SEL_REG_OFFSET); 

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
    *addr_p_reg_ISOLATE_IN &= ~(1<<8);
    int32_t *addr_p_reg_ISOLATE_OUT =(int32_t *)(SERIAL_LINK_REG_START_ADDRESS + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET);
    *addr_p_reg_ISOLATE_OUT &= ~(1<<9); // axi_out_isolate
    }

void __attribute__ ((optimize("00"))) EXTERNAL_BUS_SL_CONFIG(void){

    volatile int32_t *addr_p_reg_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x06000 + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); //0x06000000 
    *addr_p_reg_ext = (*addr_p_reg_ext)| 0x00000001; // ctrl clock enable external
    *addr_p_reg_ext = (*addr_p_reg_ext)& 0x11111101; // rst on
    *addr_p_reg_ext = (*addr_p_reg_ext)| 0x00000002; // rst oFF

    int32_t *addr_p_reg_ISOLATE_IN_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x06000 + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET); 
    *addr_p_reg_ISOLATE_IN_ext &= ~(1<<8);
    int32_t *addr_p_reg_ISOLATE_OUT_ext =(int32_t *)(EXT_PERIPHERAL_START_ADDRESS + 0x06000 + SERIAL_LINK_SINGLE_CHANNEL_CTRL_REG_OFFSET);
    *addr_p_reg_ISOLATE_OUT_ext &= ~(1<<9);


    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void __attribute__ ((optimize("00"))) SL_CPU_TRANS(uint32_t *src_d, uint32_t *dst_d,uint32_t *src, uint32_t *dst,  uint32_t large ){

    for (int i = 0; i < large; i++) {
        *src = *(src_d + i);
    }
    
    for (int i = 0; i < large; i++) {
        *(dst_d + i) = *dst;
    }

}

// parameter "large" should equal to or less than FIFO size (default 8)
void __attribute__ ((optimize("00"))) SL_DMA_TRANS(uint32_t *src_d, uint32_t *dst_d, uint32_t *src, uint32_t *dst,uint32_t large){
    volatile static dma_config_flags_t res;
    volatile static dma_target_t tgt_src_d;
    volatile static dma_target_t tgt_dst_d;
    volatile static dma_trans_t trans;


        dma_init(NULL);
        tgt_src_d.ptr = (uint32_t *)src_d;
        tgt_src_d.inc_d1_du = 1;
        tgt_src_d.trig = DMA_TRIG_MEMORY;
        tgt_src_d.type = DMA_DATA_TYPE_WORD;

        tgt_dst_d.ptr = (uint32_t *)src;
        tgt_dst_d.inc_d1_du = 0;
        tgt_dst_d.trig = DMA_TRIG_MEMORY;
        tgt_dst_d.type = DMA_DATA_TYPE_WORD;

        trans.src = &tgt_src_d;
        trans.dst = &tgt_dst_d;
        trans.size_d1_du = large;
        trans.mode = DMA_TRANS_MODE_SINGLE;
        trans.win_du = 0;
        trans.sign_ext = 0;
        trans.end = DMA_TRANS_END_INTR;

        res |= dma_validate_transaction(&trans, false, false);
        res |= dma_load_transaction(&trans);
        res |= dma_launch(&trans);
        
        if(!dma_is_ready(0)) {
            CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
                    if (!dma_is_ready(0)) {
                        wait_for_interrupt();
                    }
            CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
        }


        dma_init(NULL);
        tgt_src_d.ptr = (uint32_t *)dst;
        tgt_src_d.inc_d1_du = 0;
        tgt_src_d.trig = DMA_TRIG_MEMORY;
        tgt_src_d.type = DMA_DATA_TYPE_WORD;

        tgt_dst_d.ptr = (uint32_t *)dst_d;
        tgt_dst_d.inc_d1_du = 1;
        tgt_dst_d.trig = DMA_TRIG_MEMORY;
        tgt_dst_d.type = DMA_DATA_TYPE_WORD;

        trans.src = &tgt_src_d;
        trans.dst = &tgt_dst_d;
        trans.size_d1_du = large;
        trans.mode = DMA_TRANS_MODE_SINGLE;
        trans.win_du = 0;
        trans.sign_ext = 0;
        trans.end = DMA_TRANS_END_INTR;

        res |= dma_validate_transaction(&trans, false, false);
        res |= dma_load_transaction(&trans);
        res |= dma_launch(&trans);

        if(!dma_is_ready(0)) {
            CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
                    if (!dma_is_ready(0)) {
                        wait_for_interrupt();
                    }
            CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
        }
}