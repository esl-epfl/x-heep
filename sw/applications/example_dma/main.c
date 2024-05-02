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

#ifdef TEST_2D_MODE

#define SIZE_IN_D1 30
#define SIZE_IN_D2 30
#define SIZE_OUT_D1 16
#define SIZE_OUT_D2 16
#define STRIDE_IN_D1 1
#define STRIDE_IN_D2 1
#define STRIDE_OUT_D1 1
#define STRIDE_OUT_D2 1
#define TOP_PAD 1
#define BOTTOM_PAD 1
#define LEFT_PAD 1
#define RIGHT_PAD 1
#define OUT_D1 (SIZE_OUT_D1 + LEFT_PAD + RIGHT_PAD)
#define OUT_D2 (SIZE_OUT_D2 + TOP_PAD + BOTTOM_PAD)
#define OUT_DIM ( OUT_D1 * OUT_D2 )
#define DMA_CSR_REG_MIE_MASK (( 1 << 19 ) | (1 << 11 ) )
#define DMA_DATA_TYPE DMA_DATA_TYPE_WORD

// Change the input datatype depending on the DMA_DATA_TYPE
typedef uint32_t dma_input_data_type;

dma_input_data_type test_data_2D[SIZE_IN_D1 * SIZE_IN_D2] = {253 ,207 ,382 ,390 ,301 ,174 ,86 ,342 ,313 ,367 ,384 ,322 ,164 ,403 ,312 ,110 ,464 ,245 ,461 ,249 ,280 ,198 ,376 ,374 ,55 ,23 ,111 ,42 ,265 ,141,180 ,157 ,65 ,302 ,15 ,18 ,256 ,79 ,237 ,270 ,313 ,232 ,384 ,493 ,124 ,72 ,284 ,485 ,50 ,407 ,109 ,251 ,175 ,283 ,258 ,77 ,113 ,301 ,292 ,212,319 ,58 ,379 ,266 ,419 ,290 ,350 ,4 ,351 ,206 ,423 ,311 ,242 ,301 ,3 ,45 ,486 ,485 ,10 ,349 ,311 ,1 ,142 ,170 ,411 ,247 ,128 ,297 ,428 ,498,207 ,288 ,137 ,52 ,388 ,291 ,475 ,45 ,16 ,354 ,464 ,139 ,153 ,318 ,227 ,115 ,312 ,292 ,438 ,78 ,385 ,72 ,321 ,196 ,459 ,376 ,413 ,206 ,438 ,33,477 ,472 ,112 ,324 ,477 ,319 ,289 ,137 ,368 ,107 ,342 ,243 ,326 ,231 ,84 ,431 ,301 ,428 ,200 ,228 ,290 ,61 ,297 ,115 ,462 ,76 ,318 ,267 ,335 ,258,354 ,488 ,420 ,479 ,331 ,352 ,254 ,255 ,433 ,128 ,147 ,77 ,446 ,265 ,165 ,65 ,370 ,47 ,93 ,33 ,149 ,35 ,188 ,278 ,309 ,376 ,213 ,311 ,441 ,139,488 ,129 ,403 ,464 ,98 ,417 ,442 ,277 ,407 ,13 ,419 ,309 ,6 ,414 ,226 ,8 ,240 ,19 ,101 ,126 ,422 ,443 ,491 ,87 ,376 ,319 ,490 ,61 ,495 ,73,120 ,458 ,2 ,29 ,428 ,415 ,72 ,20 ,181 ,395 ,352 ,66 ,452 ,429 ,475 ,229 ,186 ,62 ,442 ,250 ,205 ,282 ,63 ,176 ,181 ,185 ,218 ,51 ,316 ,191,146 ,29 ,270 ,224 ,316 ,383 ,394 ,207 ,424 ,411 ,19 ,211 ,25 ,410 ,12 ,49 ,66 ,445 ,18 ,333 ,216 ,106 ,68 ,129 ,290 ,211 ,276 ,376 ,167 ,94,466 ,383 ,90 ,161 ,312 ,80 ,291 ,480 ,115 ,242 ,267 ,130 ,15 ,459 ,51 ,335 ,454 ,280 ,4 ,384 ,178 ,211 ,22 ,366 ,305 ,390 ,493 ,59 ,245 ,392,100 ,468 ,61 ,290 ,53 ,452 ,166 ,454 ,339 ,159 ,83 ,340 ,338 ,374 ,163 ,483 ,103 ,22 ,172 ,161 ,468 ,270 ,397 ,356 ,480 ,295 ,165 ,374 ,384 ,53,101 ,493 ,126 ,321 ,476 ,197 ,21 ,103 ,62 ,361 ,276 ,224 ,468 ,277 ,79 ,490 ,228 ,19 ,365 ,147 ,241 ,406 ,375 ,2 ,75 ,488 ,378 ,357 ,458 ,296,153 ,146 ,470 ,73 ,32 ,92 ,192 ,118 ,450 ,308 ,89 ,186 ,383 ,342 ,489 ,176 ,422 ,29 ,108 ,317 ,412 ,248 ,299 ,15 ,434 ,143 ,240 ,307 ,26 ,190,271 ,243 ,74 ,439 ,170 ,298 ,349 ,115 ,138 ,126 ,273 ,475 ,375 ,137 ,334 ,453 ,26 ,281 ,348 ,392 ,198 ,355 ,295 ,280 ,252 ,316 ,12 ,463 ,129 ,495,32 ,102 ,239 ,370 ,67 ,4 ,237 ,248 ,126 ,81 ,266 ,21 ,298 ,124 ,375 ,20 ,240 ,67 ,58 ,387 ,176 ,390 ,402 ,360 ,72 ,320 ,304 ,63 ,41 ,444,189 ,495 ,142 ,333 ,206 ,203 ,106 ,183 ,39 ,345 ,145 ,498 ,63 ,56 ,22 ,381 ,94 ,178 ,63 ,78 ,235 ,326 ,412 ,339 ,431 ,333 ,483 ,175 ,83 ,320,330 ,103 ,135 ,284 ,134 ,163 ,84 ,280 ,56 ,83 ,168 ,294 ,446 ,151 ,191 ,371 ,127 ,74 ,474 ,358 ,44 ,9 ,231 ,427 ,469 ,183 ,12 ,220 ,469 ,95,31 ,350 ,246 ,119 ,158 ,402 ,494 ,235 ,241 ,112 ,79 ,339 ,472 ,394 ,125 ,282 ,349 ,270 ,91 ,333 ,214 ,365 ,138 ,144 ,370 ,178 ,410 ,317 ,108 ,205,35 ,276 ,125 ,186 ,143 ,259 ,385 ,303 ,427 ,290 ,241 ,484 ,293 ,209 ,264 ,211 ,124 ,351 ,441 ,432 ,258 ,80 ,480 ,287 ,387 ,429 ,162 ,476 ,187 ,401,382 ,488 ,312 ,100 ,194 ,398 ,365 ,60 ,474 ,120 ,357 ,399 ,306 ,13 ,255 ,446 ,174 ,489 ,166 ,110 ,155 ,273 ,262 ,351 ,179 ,99 ,244 ,409 ,243 ,490,89 ,322 ,265 ,392 ,117 ,384 ,104 ,231 ,475 ,442 ,333 ,485 ,62 ,203 ,248 ,103 ,130 ,21 ,320 ,312 ,239 ,144 ,259 ,110 ,118 ,462 ,175 ,83 ,90 ,175,410 ,334 ,265 ,117 ,497 ,277 ,400 ,1 ,140 ,410 ,197 ,289 ,58 ,286 ,471 ,137 ,363 ,221 ,60 ,341 ,391 ,181 ,478 ,377 ,203 ,12 ,51 ,399 ,286 ,428,371 ,477 ,243 ,326 ,20 ,126 ,159 ,251 ,248 ,500 ,275 ,483 ,490 ,365 ,451 ,145 ,104 ,66 ,99 ,66 ,176 ,328 ,60 ,222 ,394 ,120 ,108 ,218 ,287 ,437,492 ,82 ,2 ,30 ,404 ,482 ,187 ,474 ,137 ,122 ,161 ,30 ,491 ,145 ,426 ,487 ,91 ,285 ,142 ,304 ,431 ,488 ,144 ,492 ,129 ,98 ,409 ,349 ,134 ,391,321 ,128 ,238 ,494 ,497 ,377 ,247 ,64 ,428 ,191 ,105 ,423 ,402 ,434 ,47 ,277 ,278 ,87 ,150 ,41 ,228 ,311 ,47 ,33 ,69 ,411 ,165 ,101 ,393 ,365,174 ,287 ,463 ,462 ,214 ,417 ,256 ,116 ,267 ,406 ,140 ,159 ,494 ,119 ,468 ,274 ,361 ,53 ,224 ,267 ,382 ,419 ,75 ,462 ,492 ,367 ,375 ,204 ,276 ,78,48 ,300 ,140 ,350 ,480 ,122 ,316 ,282 ,470 ,495 ,376 ,244 ,250 ,401 ,488 ,375 ,53 ,221 ,186 ,308 ,448 ,393 ,316 ,115 ,406 ,198 ,296 ,442 ,399 ,40,27 ,148 ,430 ,392 ,450 ,385 ,111 ,345 ,273 ,61 ,109 ,411 ,408 ,3 ,94 ,485 ,210 ,424 ,295 ,274 ,436 ,84 ,213 ,35 ,438 ,432 ,80 ,321 ,19 ,355,482 ,444 ,361 ,268 ,210 ,109 ,480 ,415 ,406 ,350 ,141 ,38 ,28 ,334 ,313 ,230 ,494 ,153 ,410 ,363 ,370 ,292 ,190 ,457 ,97 ,26 ,415 ,77 ,67 ,412,126 ,216 ,231 ,485 ,298 ,113 ,141 ,122 ,289 ,89 ,194 ,413 ,390 ,71 ,207 ,82 ,421 ,316 ,404 ,134 ,451 ,477 ,176 ,128 ,470 ,360 ,467 ,345 ,292 ,453};
#endif


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

// This example tests the 2D operations of the DMA and its padding capabilites


PRINTF("\n\n\r===================================\n\n\r");
PRINTF("    TESTING 2D MODE   ");
PRINTF("\n\n\r===================================\n\n\r");


dma *peri = dma_peri;

int left_pad_cnt = 0;
int top_pad_cnt = 0;
dma_input_data_type copied_data_2D_DMA[OUT_DIM];
dma_input_data_type copied_data_2D_CPU[OUT_DIM];
uint32_t cycles_dma, cycles_cpu;
uint32_t size_dst_trans_d1;
uint32_t dst_stride_d1;
uint32_t dst_stride_d2;
uint32_t size_src_trans_d1;
uint32_t src_stride_d1;
uint32_t src_stride_d2;
char passed = 1;

// Reset the counter to evaluate the performance of the DMA

CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
CSR_WRITE(CSR_REG_MCYCLE, 0);

// Enable the interrupt

// Enable the DMA interrupt logic
write_register(  
                0x1,
                DMA_INTERRUPT_EN_REG_OFFSET,
                0x1,
                DMA_INTERRUPT_EN_TRANSACTION_DONE_BIT,
                peri );

// Enable global interrupts
CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
// Enable fast interrupts
CSR_SET_BITS(CSR_REG_MIE, DMA_CSR_REG_MIE_MASK );

// Now I need to set up the pointers

peri->SRC_PTR = &test_data_2D[0];
peri->DST_PTR = copied_data_2D_DMA;

// Now set up the dimensionality configuration

write_register(  
                0x1,
                DMA_DIM_CONFIG_REG_OFFSET,
                0x1,
                DMA_DIM_CONFIG_DMA_DIM_BIT,
                peri );

// Set the operation mode

write_register(  
                DMA_TRANS_MODE_SINGLE,
                DMA_MODE_REG_OFFSET,
                DMA_MODE_MODE_MASK,
                DMA_MODE_MODE_OFFSET,
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

// The d2 stride is not the tipical stride but rather the distance between 
// the first element of the next row and the first element of the current row
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

write_register( TOP_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ),
                DMA_PAD_REG_OFFSET,
                DMA_PAD_TOP_PAD_MASK,
                DMA_PAD_TOP_PAD_OFFSET,
                peri );

write_register( RIGHT_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ),
                DMA_PAD_REG_OFFSET,
                DMA_PAD_RIGHT_PAD_MASK,
                DMA_PAD_RIGHT_PAD_OFFSET,
                peri );

write_register( LEFT_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ),
                DMA_PAD_REG_OFFSET,
                DMA_PAD_LEFT_PAD_MASK,
                DMA_PAD_LEFT_PAD_OFFSET,
                peri );

write_register( BOTTOM_PAD * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ),
                DMA_PAD_REG_OFFSET,
                DMA_PAD_BOTTOM_PAD_MASK,
                DMA_PAD_BOTTOM_PAD_OFFSET,
                peri );

// Set the sizes

write_register( SIZE_OUT_D2 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ),
                DMA_SIZE_TR_D2_REG_OFFSET,
                DMA_SIZE_TR_D2_SIZE_MASK,
                DMA_SIZE_TR_D2_SIZE_OFFSET,
                peri );

write_register( SIZE_OUT_D1 * DMA_DATA_TYPE_2_SIZE( DMA_DATA_TYPE ),
                DMA_SIZE_TR_D1_REG_OFFSET,
                DMA_SIZE_TR_D1_SIZE_MASK,
                DMA_SIZE_TR_D1_SIZE_OFFSET,
                peri );

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

// Read the cycles count after the DMA run
CSR_READ(CSR_REG_MCYCLE, &cycles_dma);

// Reset the performance counter to evaluate the CPU performance
CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);
CSR_WRITE(CSR_REG_MCYCLE, 0);

int read_ptr = 0, write_ptr = 0;

// Run the same computation on the CPU
for (int i=0; i < OUT_D2; i++)
{
    for (int j=0; j < OUT_D1; j++)
    {
        read_ptr = i * STRIDE_OUT_D2 * OUT_D1 + j * STRIDE_OUT_D1;
        write_ptr = (i - top_pad_cnt ) * STRIDE_IN_D2 * SIZE_IN_D1 + (j - left_pad_cnt) * STRIDE_IN_D1;
        if (i < TOP_PAD || i >= SIZE_OUT_D2 + BOTTOM_PAD || j < LEFT_PAD || j >= SIZE_OUT_D1 + RIGHT_PAD)
        {
            copied_data_2D_CPU[read_ptr] = 0;
        }
        else
        {
            copied_data_2D_CPU[read_ptr] = test_data_2D[write_ptr];
        }
        if (j < LEFT_PAD && i >= TOP_PAD)
        {
            left_pad_cnt++;
        }
    }
    if (i < TOP_PAD)
    {
        top_pad_cnt++;
    }
    left_pad_cnt = 0;
}

// Read the cycles count after the CPU run
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

PRINTF("\n\r");

for (int i = 0; i < OUT_D2; i++) {
    for (int j = 0; j < OUT_D1; j++) {
        PRINTF("%d ", copied_data_2D_CPU[i * OUT_D1 + j]);
    }
    PRINTF("\n\r");
}

// Verify that the DMA and the CPU outputs are the same
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
