// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>


#include "dma.h"
#include "core_v_mini_mcu.h"
#include "x-heep.h"
#include "csr.h"
#include "rv_plic.h"

//#define TEST_SINGULAR_MODE
//#define TEST_PENDING_TRANSACTION
//#define TEST_WINDOW
//#define TEST_ADDRESS_MODE
//#define TEST_ADDRESS_MODE_EXTERNAL_DEVICE
#define TEST_2D_MODE

#define TEST_DATA_SIZE      16
#define TEST_DATA_LARGE     1024
#define TRANSACTIONS_N      3       // Only possible to perform transaction at a time, others should be blocked
#define TEST_WINDOW_SIZE_DU  1024    // if put at <=71 the isr is too slow to react to the interrupt

// Defines for low-level fw
#define DMA_REGISTER_SIZE_BYTES sizeof(int)
#define DMA_SELECTION_OFFSET_START 0


#if TEST_DATA_LARGE < 2* TEST_DATA_SIZE
    #errors("TEST_DATA_LARGE must be at least 2*TEST_DATA_SIZE")
#endif

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   1

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif


int32_t errors = 0;
int8_t cycles = 0;

static inline volatile void write_register( uint32_t  p_val,
                                uint32_t  p_offset,
                                uint32_t  p_mask,
                                uint8_t   p_sel,
                                dma* peri ) 
    {
        /*
        * The index is computed to avoid needing to access the structure
        * as a structure.
        */
        uint8_t index = p_offset / DMA_REGISTER_SIZE_BYTES;
        /*
        * An intermediate variable "value" is used to prevent writing twice into
        * the register.
        */
        uint32_t value  =  (( uint32_t * ) peri ) [ index ];
        value           &= ~( p_mask << p_sel );
        value           |= (p_val & p_mask) << p_sel;
        (( uint32_t * ) peri ) [ index ] = value;

    };

void dma_intr_handler_trans_done()
{
    cycles++;
}


#ifdef TEST_WINDOW

int32_t window_intr_flag;

void dma_intr_handler_window_done(void) {
    window_intr_flag ++;
}

uint8_t dma_window_ratio_warning_threshold()
{
    return 0;
}

#endif // TEST_WINDOW


int main(int argc, char *argv[])
{

    static uint32_t test_data_4B[TEST_DATA_SIZE] __attribute__ ((aligned (4))) = {
      0x76543210, 0xfedcba98, 0x579a6f90, 0x657d5bee, 0x758ee41f, 0x01234567, 0xfedbca98, 0x89abcdef, 0x679852fe, 0xff8252bb, 0x763b4521, 0x6875adaa, 0x09ac65bb, 0x666ba334, 0x55446677, 0x65ffba98};
    static uint32_t copied_data_4B[TEST_DATA_LARGE] __attribute__ ((aligned (4))) = { 0 };
    static uint32_t test_data_large[TEST_DATA_LARGE] __attribute__ ((aligned (4))) = { 0 };

     // this array will contain the even address of copied_data_4B
    uint32_t* test_addr_4B_PTR = &test_data_large[0];

    // The DMA is initialized (i.e. Any current transaction is cleaned.)
    dma_init(NULL);

    dma_config_flags_t res;

    dma_target_t tgt_src = {
                                .ptr        = test_data_4B,
                                .inc_du     = 1,
                                .size_du    = TEST_DATA_SIZE,
                                .trig       = DMA_TRIG_MEMORY,
                                .type       = DMA_DATA_TYPE_WORD,
                                };
    dma_target_t tgt_dst = {
                                .ptr        = copied_data_4B,
                                .inc_du     = 1,
                                .size_du    = TEST_DATA_SIZE,
                                .trig       = DMA_TRIG_MEMORY,
                                };

    dma_target_t tgt_addr = {
                                .ptr        = test_addr_4B_PTR,
                                .inc_du     = 1,
                                .size_du    = TEST_DATA_SIZE,
                                .trig       = DMA_TRIG_MEMORY,
                                };

    dma_trans_t trans = {
                                .src        = &tgt_src,
                                .dst        = &tgt_dst,
                                .src_addr   = &tgt_addr,
                                .mode       = DMA_TRANS_MODE_SINGLE,
                                .win_du     = 0,
                                .end        = DMA_TRANS_END_INTR,
                                };
    // Create a target pointing at the buffer to be copied. Whole WORDs, no skippings, in memory, no environment.

#ifdef TEST_SINGULAR_MODE
    /*
    PRINTF("\n\n\r===================================\n\n\r");
    PRINTF("    TESTING SINGLE MODE   ");
    PRINTF("\n\n\r===================================\n\n\r");
    */

    res = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    //PRINTF("tran: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res = dma_load_transaction(&trans);
    //PRINTF("load: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res = dma_launch(&trans);
    //PRINTF("laun: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");

    while( ! dma_is_ready()) {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready() == 0 ) {
            wait_for_interrupt();
            //from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }
    PRINTF(">> Finished transaction. \n\r");

    for(uint32_t i = 0; i < trans.size_b; i++ ) {
        if ( ((uint8_t*)copied_data_4B)[i] != ((uint8_t*)test_data_4B)[i] ) {
            PRINTF("ERROR [%d]: %04x != %04x\n\r", i, ((uint8_t*)copied_data_4B)[i], ((uint8_t*)test_data_4B)[i]);
            errors++;
        }
    }

    if (errors == 0) {
        PRINTF("DMA single mode success.\n\r");
    } else {
        PRINTF("DMA single mode failure: %d errors out of %d bytes checked\n\r", errors, trans.size_b );
        return EXIT_FAILURE;
    }

#endif // TEST_SINGULAR_MODE

#ifdef TEST_ADDRESS_MODE

    PRINTF("\n\n\r===================================\n\n\r");
    PRINTF("    TESTING ADDRESS MODE   ");
    PRINTF("\n\n\r===================================\n\n\r");

    // Prepare the data
    for (int i = 0; i < TEST_DATA_SIZE; i++) {
        test_addr_4B_PTR[i] = &copied_data_4B[i*2];
    }

    trans.mode = DMA_TRANS_MODE_ADDRESS;

    res = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF("tran: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res = dma_load_transaction(&trans);
    PRINTF("load: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res = dma_launch(&trans);
    PRINTF("laun: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");

    while( ! dma_is_ready()) {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready() == 0 ) {
            wait_for_interrupt();
            //from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }


    PRINTF(">> Finished transaction. \n\r");

    for(uint32_t i = 0; i < trans.size_b >> 2; i++ ) {
        if ( copied_data_4B[i*2] != test_data_4B[i] ) {
            PRINTF("ERROR [%d]: %04x != %04x\n\r", i, copied_data_4B[i*2], test_data_4B[i]);
            errors++;
        }
    }

    if (errors == 0) {
        PRINTF("DMA address mode success.\n\r");
    } else {
        PRINTF("DMA address mode failure: %d errors out of %d bytes checked\n\r", errors, trans.size_b );
        return EXIT_FAILURE;
    }

    trans.mode = DMA_TRANS_MODE_SINGLE;


#endif // TEST_ADDRESS_MODE

#if defined(TARGET_SIM) || defined(TARGET_SYSTEMC)

#ifdef TEST_ADDRESS_MODE_EXTERNAL_DEVICE

#pragma message ( "this application should not be ran in a system integrating x-heep as in the external \
    slave can be plugged something else than a slow memory as in our testbench" )

    uint32_t* ext_test_addr_4B_PTR = EXT_SLAVE_START_ADDRESS;
    uint32_t* ext_copied_data_4B;

    ext_copied_data_4B = &ext_test_addr_4B_PTR[TEST_DATA_SIZE+1];


    tgt_addr.ptr = ext_test_addr_4B_PTR;
    trans.src_addr = &tgt_addr;

    PRINTF("\n\n\r=====================================\n\n\r");
    PRINTF("    TESTING ADDRESS MODE IN EXTERNAL MEMORY  ");
    PRINTF("\n\n\r=====================================\n\n\r");

    // Prepare the data
    for (int i = 0; i < TEST_DATA_SIZE; i++) {
        ext_test_addr_4B_PTR[i] = &ext_copied_data_4B[i*2];
    }

    trans.mode = DMA_TRANS_MODE_ADDRESS;

    res = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF("tran: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res = dma_load_transaction(&trans);
    PRINTF("load: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res = dma_launch(&trans);
    PRINTF("laun: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");

    while( ! dma_is_ready()) {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        if ( dma_is_ready() == 0 ) {
            wait_for_interrupt();
            //from here we wake up even if we did not jump to the ISR
        }
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }


    PRINTF(">> Finished transaction. \n\r");

    for(uint32_t i = 0; i < trans.size_b >> 2; i++ ) {
        if ( ext_copied_data_4B[i*2] != test_data_4B[i] ) {
            PRINTF("ERROR [%d]: %04x != %04x\n\r", i, ext_copied_data_4B[i], test_data_4B[i]);
            errors++;
        }
    }

    if (errors == 0) {
        PRINTF("DMA address mode in external memory success.\n\r");
    } else {
        PRINTF("DMA address mode in external memory failure: %d errors out of %d bytes checked\n\r", errors, trans.size_b );
        return EXIT_FAILURE;
    }

    trans.mode = DMA_TRANS_MODE_SINGLE;

#endif //TEST_ADDRESS_MODE_EXTERNAL_DEVICE

#else
    #pragma message( "TEST_ADDRESS_MODE_EXTERNAL_DEVICE is not executed on target different than TARGET_SIM" )
#endif

#ifdef TEST_PENDING_TRANSACTION
    PRINTF("\n\n\r===================================\n\n\r");
    PRINTF("    TESTING MULTIPLE TRANSACTIONS   ");
    PRINTF("\n\n\r===================================\n\n\r");

    for (uint32_t i = 0; i < TEST_DATA_LARGE; i++) {
        test_data_large[i] = i;
    }


    tgt_src.ptr     = test_data_large;
    tgt_src.size_du = TEST_DATA_LARGE;

    // trans.end = DMA_TRANS_END_INTR_WAIT; // This option makes no sense, because the launch is blocking the program until the trans finishes.
    trans.end = DMA_TRANS_END_INTR;
    // trans.end = DMA_TRANS_END_POLLING;


    res = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF("tran: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    cycles = 0;
    uint8_t consecutive_trans = 0;

    for(  uint8_t i = 0; i < TRANSACTIONS_N; i++ ){
        res =  dma_load_transaction(&trans);
        res |= dma_launch(&trans);
        if( res == DMA_CONFIG_OK ) consecutive_trans++;
    }

    if( trans.end == DMA_TRANS_END_POLLING ){
        while( cycles < consecutive_trans ){
            while( ! dma_is_ready() );
            cycles++;
        }
    } else {
        while( cycles < consecutive_trans ){
            wait_for_interrupt();
        }
    }
    PRINTF(">> Finished %d transactions. That is %s.\n\r", consecutive_trans, consecutive_trans > 1 ? "bad" : "good");



    for(int i=0; i<TEST_DATA_LARGE; i++) {
        if (tgt_src.ptr[i] != tgt_dst.ptr[i]) {
            PRINTF("ERROR COPY [%d]: %08x != %08x : %04x != %04x\n\r", i, &tgt_src.ptr[i], &tgt_dst.ptr[i], tgt_src.ptr[i], tgt_dst.ptr[i]);
            errors++;
        }
    }

    if (errors == 0) {
        PRINTF("DMA multiple transactions success.\n\r");
    } else {
        PRINTF("DMA multiple transactions failure: %d errors out of %d words checked\n\r", errors, TEST_DATA_SIZE);
        return EXIT_FAILURE;
    }

#endif // TEST_PENDING_TRANSACTION


#ifdef TEST_WINDOW

    PRINTF("\n\n\r===================================\n\n\r");
    PRINTF("    TESTING WINDOW INTERRUPT   ");
    PRINTF("\n\n\r===================================\n\n\r");

    plic_Init();
    plic_irq_set_priority( DMA_WINDOW_INTR, 1);
    plic_irq_set_enabled(  DMA_WINDOW_INTR, kPlicToggleEnabled);

    window_intr_flag = 0;

    for (uint32_t i = 0; i < TEST_DATA_LARGE; i++) {
        test_data_large [i] = i;
        copied_data_4B  [i] = 0;
    }

    tgt_src.ptr     = test_data_large;
    tgt_src.size_du = TEST_DATA_LARGE;

    tgt_src.type    = DMA_DATA_TYPE_WORD;
    tgt_dst.type    = DMA_DATA_TYPE_WORD;

    trans.win_du     = TEST_WINDOW_SIZE_DU;
    trans.end       = DMA_TRANS_END_INTR;

    res = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );
    PRINTF("tran: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");
    res = dma_load_transaction(&trans);
    PRINTF("load: %u \t%s\n\r", res, res == DMA_CONFIG_OK ?  "Ok!" : "Error!");

    dma_launch(&trans);

    if( trans.end == DMA_TRANS_END_POLLING ){ //There will be no interrupts whatsoever!
        while( ! dma_is_ready() );
        PRINTF("?\n\r");
    } else {
        while( !dma_is_ready() ){
            wait_for_interrupt();
            PRINTF("i\n\r");
        }
    }

    PRINTF("\nWe had %d window interrupts.\n\r", window_intr_flag);

    for(uint32_t i = 0; i < TEST_DATA_LARGE; i++ ) {
        if (copied_data_4B[i] != test_data_large[i]) {
            PRINTF("[%d] %04x\tvs.\t%04x\n\r", i, copied_data_4B[i], test_data_large[i]);
            errors++;
        }
    }

    if (errors == 0) {
        PRINTF("DMA window success\n\r");
    } else {
        PRINTF("DMA window failure: %d errors out of %d words checked\n\r", errors, TEST_DATA_SIZE);
        return EXIT_FAILURE;
    }

#endif // TEST_WINDOW

#ifdef TEST_2D_MODE

#define SIZE_IN_D1 16
#define SIZE_IN_D2 16
#define SIZE_OUT_D1 14
#define SIZE_OUT_D2 14
#define STRIDE_IN_D1 1
#define STRIDE_IN_D2 1
#define STRIDE_OUT_D1 1
#define STRIDE_OUT_D2 1
#define TOP_PAD 4
#define BOTTOM_PAD 4
#define LEFT_PAD 4
#define RIGHT_PAD 4
#define OUT_D1 (SIZE_OUT_D1 + LEFT_PAD + RIGHT_PAD)
#define OUT_D2 (SIZE_OUT_D2 + TOP_PAD + BOTTOM_PAD)
#define OUT_DIM ( OUT_D1 * OUT_D2 )
#define DMA_CSR_REG_MIE_MASK (( 1 << 19 ) | (1 << 11 ) )
#define DMA_DATA_TYPE DMA_DATA_TYPE_HALF_WORD

typedef uint16_t dma_data_type_tmp; // Change the datatype depending on the DMA_DATA_TYPE

/*
PRINTF("\n\n\r===================================\n\n\r");
PRINTF("    TESTING 2D MODE   ");
PRINTF("\n\n\r===================================\n\n\r");
*/

dma *peri = dma_peri;

// Let's try to move a 2x2 window from a 4x4 matrix

dma_data_type_tmp test_data_2D[SIZE_IN_D1 * SIZE_IN_D2] = {
    12, 34, 85, 46, 95, 17, 58, 89, 23, 44, 68, 91, 14, 63, 24, 79,
    25, 80, 47, 56, 73, 89, 20, 31, 42, 57, 68, 92, 103, 210, 180, 150,
    45, 64, 23, 85, 95, 60, 20, 40, 55, 65, 75, 85, 120, 130, 140, 160,
    35, 70, 95, 100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160,
    165, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 220, 225, 230, 235, 240,
    245, 250, 255, 260, 265, 270, 275, 280, 285, 290, 295, 300, 305, 310, 315, 320,
    325, 330, 335, 340, 345, 350, 355, 360, 365, 370, 375, 380, 385, 390, 395, 400,
    405, 410, 415, 420, 425, 430, 435, 440, 445, 450, 455, 460, 465, 470, 475, 480,
    485, 490, 495, 500, 505, 510, 515, 520, 525, 530, 535, 540, 545, 550, 555, 560,
    565, 570, 575, 580, 585, 590, 595, 600, 605, 610, 615, 620, 625, 630, 635, 640,
    645, 650, 655, 660, 665, 670, 675, 680, 685, 690, 695, 700, 705, 710, 715, 720,
    725, 730, 735, 740, 745, 750, 755, 760, 765, 770, 775, 780, 785, 790, 795, 800,
    805, 810, 815, 820, 825, 830, 835, 840, 845, 850, 855, 860, 865, 870, 875, 880,
    885, 890, 895, 900, 905, 910, 915, 920, 925, 930, 935, 940, 945, 950, 955, 960,
    965, 970, 975, 980, 985, 990, 995, 1000, 1005, 1010, 1015, 1020, 1025, 1030, 1035, 1040,
    1045, 1050, 1055, 1060, 1065, 1070, 1075, 1080, 1085, 1090, 1095, 1100, 1105, 1110, 1115, 1120
};

int left_pad_cnt = 0;
int top_pad_cnt = 0;
dma_data_type_tmp copied_data_2D_DMA[OUT_DIM];
dma_data_type_tmp copied_data_2D_CPU[OUT_DIM];
uint32_t cycles_dma, cycles_cpu;
uint32_t size_dst_trans_d1;
uint32_t dst_stride_d1;
uint32_t dst_stride_d2;
uint32_t size_src_trans_d1;
uint32_t src_stride_d1;
uint32_t src_stride_d2;
char passed = 1;

//PRINTF("cpy_sz: %d\n\r", OUT_DIM);

// Testing the DMA

CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
CSR_WRITE(CSR_REG_MCYCLE, 0);

// Enable the interrupt

peri->INTERRUPT_EN = 0x1;
CSR_CLEAR_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK );
CSR_SET_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK );

// Now I need to set up the pointers

peri->SRC_PTR = &test_data_2D[0];
peri->DST_PTR = copied_data_2D_DMA;

// Now set up the dimensionality configuration

write_register(  0x1,
                DMA_DIM_CONFIG_REG_OFFSET,
                0x1,
                DMA_DIM_CONFIG_DMA_DIM_BIT,
                peri );

// Set the operation mode

peri->MODE = DMA_TRANS_MODE_SINGLE;

// Set the input dimensions

write_register(  SIZE_IN_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ),
                DMA_SIZE_IN_REG_OFFSET,
                DMA_SIZE_IN_D1_MASK,
                DMA_SIZE_IN_D1_OFFSET,
                peri );

write_register(  SIZE_IN_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ),
                DMA_SIZE_IN_REG_OFFSET,
                DMA_SIZE_IN_D2_MASK,
                DMA_SIZE_IN_D2_OFFSET,
                peri );

// Set the data type

write_register(  DMA_DATA_TYPE,
                DMA_DATA_TYPE_REG_OFFSET,
                DMA_DATA_TYPE_DATA_TYPE_MASK,
                DMA_SELECTION_OFFSET_START,
                peri );

// Set the source strides

size_src_trans_d1 = SIZE_OUT_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE );

src_stride_d1 = STRIDE_IN_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE );

// The d2 stride is not the tipitcal stride but rather the distance between the first element of the next row and the first element of the current row
src_stride_d2 = STRIDE_IN_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ) * SIZE_IN_D1 - (size_src_trans_d1 - DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ) + (src_stride_d1 - DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE )) * (SIZE_OUT_D1 - 1));

//PRINTF("src_s_d2: %d\n\r", src_stride_d2);

write_register(  src_stride_d1,
                DMA_SRC_PTR_INC_REG_OFFSET,
                DMA_SRC_PTR_INC_SRC_PTR_INC_D1_MASK,
                DMA_SRC_PTR_INC_SRC_PTR_INC_D1_OFFSET,
                peri );

write_register(  src_stride_d2,
                DMA_SRC_PTR_INC_REG_OFFSET,
                DMA_SRC_PTR_INC_SRC_PTR_INC_D2_MASK,
                DMA_SRC_PTR_INC_SRC_PTR_INC_D2_OFFSET,
                peri );

// Set the output strides
size_dst_trans_d1 = SIZE_OUT_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE );

dst_stride_d1 = STRIDE_OUT_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE );

// The d2 stride is not the tipitcal stride but rather the distance between the first element of the next row and the first element of the current row
dst_stride_d2 = STRIDE_OUT_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ) * (OUT_D1 - LEFT_PAD - RIGHT_PAD) - (size_dst_trans_d1 - DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ) + (dst_stride_d1 - DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE )) * (SIZE_OUT_D1 - 1));

write_register(  dst_stride_d1,
                DMA_DST_PTR_INC_REG_OFFSET,
                DMA_DST_PTR_INC_DST_PTR_INC_D1_MASK,
                DMA_DST_PTR_INC_DST_PTR_INC_D1_OFFSET,
                peri );

write_register(  dst_stride_d2,
                DMA_DST_PTR_INC_REG_OFFSET,
                DMA_DST_PTR_INC_DST_PTR_INC_D2_MASK,
                DMA_DST_PTR_INC_DST_PTR_INC_D2_OFFSET,
                peri );

// Set the padding

write_register(  TOP_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ),
                DMA_PAD_REG_OFFSET,
                DMA_PAD_TOP_PAD_MASK,
                DMA_PAD_TOP_PAD_OFFSET,
                peri );

write_register(  RIGHT_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ),
                DMA_PAD_REG_OFFSET,
                DMA_PAD_RIGHT_PAD_MASK,
                DMA_PAD_RIGHT_PAD_OFFSET,
                peri );

write_register(  LEFT_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ),
                DMA_PAD_REG_OFFSET,
                DMA_PAD_LEFT_PAD_MASK,
                DMA_PAD_LEFT_PAD_OFFSET,
                peri );

write_register(  BOTTOM_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ),
                DMA_PAD_REG_OFFSET,
                DMA_PAD_BOTTOM_PAD_MASK,
                DMA_PAD_BOTTOM_PAD_OFFSET,
                peri );

// Set the sizes
    
peri->SIZE_TR_D2 = SIZE_OUT_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ); 

peri->SIZE_TR_D1 = SIZE_OUT_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ); // Now the transaction should start

while( ! dma_is_ready()) {
    // disable_interrupts
    // this does not prevent waking up the core as this is controlled by the MIP register
    CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
    if ( dma_is_ready() == 0 ) {
        //wait_for_interrupt();
        //from here we wake up even if we did not jump to the ISR
    }
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
}

CSR_READ(CSR_REG_MCYCLE, &cycles_dma);

CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);

CSR_WRITE(CSR_REG_MCYCLE, 0);

// Now let's do the same with the CPU

for (int i=0; i < OUT_D2; i++)
{
    for (int j=0; j < OUT_D1; j++)
    {
        if (i < TOP_PAD || i >= SIZE_OUT_D2 + BOTTOM_PAD || j < LEFT_PAD || j >= SIZE_OUT_D1 + RIGHT_PAD)
        {
            copied_data_2D_CPU[i * STRIDE_OUT_D2 * OUT_D1 + j * STRIDE_OUT_D1] = 0;
        }
        else
        {
            copied_data_2D_CPU[i * STRIDE_OUT_D2 * OUT_D1 + j * STRIDE_OUT_D1] = test_data_2D[(i - top_pad_cnt ) * STRIDE_IN_D2 * SIZE_IN_D1 + (j - left_pad_cnt) * STRIDE_IN_D1 ];
        }
        if (j < LEFT_PAD && i >= TOP_PAD)
        {
            left_pad_cnt++;
        }
        //PRINTF("i: %d, j: %d, %d, %d, %d, %d\n\r", i, j, (i + STRIDE_OUT_D2 - 1) * OUT_D1 + j + STRIDE_OUT_D1 - 1, (i - top_pad_cnt + STRIDE_IN_D2 - 1) * SIZE_IN_D1 + j - left_pad_cnt + STRIDE_IN_D1 - 1, copied_data_2D_CPU[(i + STRIDE_OUT_D2 - 1) * OUT_D1 + j + STRIDE_OUT_D1 - 1], test_data_2D[(i - top_pad_cnt + STRIDE_IN_D2 - 1) * SIZE_IN_D1 + j - left_pad_cnt + STRIDE_IN_D1 - 1]);
        
    }
    if (i < TOP_PAD)
    {
        top_pad_cnt++;
    }
    left_pad_cnt = 0;
}

CSR_READ(CSR_REG_MCYCLE, &cycles_cpu);

PRINTF("DMA: %d\n\r", cycles_dma);
PRINTF("CPU: %d \n\r", cycles_cpu);

PRINTF("\n\r");


for (int i = 0; i < OUT_D2; i++) {
    for (int j = 0; j < OUT_D1; j++) {
        PRINTF("%d ", copied_data_2D_DMA[i * OUT_D1 + j]);
    }
    PRINTF("\n\r");
}
/*
PRINTF("\n\r");

for (int i = 0; i < OUT_D2; i++) {
    for (int j = 0; j < OUT_D1; j++) {
        PRINTF("%d ", copied_data_2D_CPU[i * OUT_D1 + j]);
    }
    PRINTF("\n\r");
}
*/

for (int i = 0; i < OUT_D2; i++) {
    for (int j = 0; j < OUT_D1; j++) {
        if (copied_data_2D_DMA[i * OUT_D1 + j] != copied_data_2D_CPU[i * OUT_D1 + j]) {
            passed = 0;
        }
    }
}

if (passed) {
    PRINTF("Success\n\r");
} else {
    PRINTF("Fail\n\r");
    return EXIT_FAILURE;
}

#endif // TEST_2D_MODE

return EXIT_SUCCESS;
}
