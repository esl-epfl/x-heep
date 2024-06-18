#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "gpio.h"
#include "pad_control.h"
#include "pad_control_regs.h"  // Generated.
#include "x-heep.h"
#include "bitfield.h"

#include "i2c.h"
#include "i2c_regs.h"

#include "i2c_structs.h"

// #include <rv_plic_structs.h>

void test_config();
void test_set_watermark();
void test_reset();
void test_host_enable();
void test_override();
void test_write_byte();
// void test_read_byte();
void test_irq_enable();
void test_irq_snapshot_restore();

void print_FSM_status();

void test_read();
void test_transmit();

void handler_irq_i2c_fmtOverflow(void)
{
    printf("Ecco!\n");
}

uint8_t plic_intr_flag = 0;

int main()
{
    // printf("\n------------------------------------\n");
    // printf("\nStarting i2c test!\n");

    test_config();
    test_reset();
    test_set_watermark();
    test_host_enable();
    // test_override();
    // test_write_byte();
    // test_read_byte();
    // test_irq_enable();
    // test_irq_snapshot_restore();

    // print_FSM_status();

    // test_read();
    test_transmit();

    printf("Done!\n");

    return 1;
}


void test_config()
{
    printf("\n\n>> Config...\n");

    i2c_config_t config;

    i2c_timing_config_t time_config = {
        .lowest_target_device_speed = 100000,
        .clock_period_nanos = 1000000000,
        .sda_rise_nanos = 500,
        .sda_fall_nanos = 500,
        .scl_period_nanos = 1500
    };


    i2c_compute_timing(time_config, &config);


    /* This is the hardcoded version of doing it
       it allows to directly see the numbers in the
       register when printing */

    // i2c_config_t config = {
    //     // TIMING 0
    //     .scl_time_high_cycles = 1,
    //     .scl_time_low_cycles = 1,
        
    //     // TIMING 1
    //     .rise_cycles = 1,
    //     .fall_cycles = 1,

    //     // TIMING 2
    //     .start_signal_setup_cycles = 1,
    //     .start_signal_hold_cycles = 1,

    //     // TIMING 3
    //     .data_signal_setup_cycles = 1,
    //     .data_signal_hold_cycles = 1,

    //     // TIMING 4
    //     .stop_signal_setup_cycles = 1,
    //     .stop_signal_hold_cycles = 1
    // };

    i2c_configure(config);

    // printf("TIMING0:\t%d\n", i2c_peri->TIMING0);
    // printf("TIMING1:\t%d\n", i2c_peri->TIMING1);
    // printf("TIMING2:\t%d\n", i2c_peri->TIMING2);
    // printf("TIMING3:\t%d\n", i2c_peri->TIMING3);
    // printf("TIMING4:\t%d\n", i2c_peri->TIMING4);
}

void test_set_watermark()
{
    printf(">> Set watermark...\n");

    i2c_level_t rx_level = kI2cLevel4Byte;

    i2c_level_t fmt_level = kI2cLevel16Byte;

    i2c_set_watermarks(rx_level, fmt_level);

    // printf("FIFO_CTRL:\t%d\n", i2c_peri->FIFO_CTRL);
    // printf("\tRX_LVL:\t\t%d\n", bitfield_read(i2c_peri->FIFO_CTRL, 0x3, I2C_FIFO_CTRL_RXILVL_OFFSET));
    // printf("\tFMT_LVL:\t%d\n", bitfield_read(i2c_peri->FIFO_CTRL, 0x3, I2C_FIFO_CTRL_FMTILVL_OFFSET));

}

void test_reset()
{   
    printf(">> Reset...\n");

    i2c_reset_rx_fifo();
    i2c_reset_fmt_fifo();
    i2c_reset_acq_fifo();
    i2c_reset_tx_fifo();

    int32_t rx_rst = bitfield_read(i2c_peri->FIFO_CTRL, BIT_MASK_1, I2C_FIFO_CTRL_RXRST_BIT);
    int32_t fmt_rst = bitfield_read(i2c_peri->FIFO_CTRL, BIT_MASK_1, I2C_FIFO_CTRL_FMTRST_BIT);
    int32_t rx_lvl = bitfield_read(i2c_peri->FIFO_CTRL, 0x3, I2C_FIFO_CTRL_RXILVL_OFFSET);
    int32_t fmt_lvl = bitfield_read(i2c_peri->FIFO_CTRL, 0x3, I2C_FIFO_CTRL_FMTILVL_OFFSET);

    // printf("\tFIFO_CTRL:\t%d\n", i2c_peri->FIFO_CTRL);
    // printf("\tRx rst:\t\t%d\n", rx_rst);
    // printf("\tFmt rst:\t%d\n", fmt_rst);
    // printf("\tRX_LVL:\t\t%d\n", rx_lvl);
    // printf("\tFMT_LVL:\t%d\n", fmt_lvl);
}


void test_host_enable(){

    printf(">> Host enable\n");

    i2c_result_t res = i2c_host_set_enabled(kI2cToggleEnabled);
    if(res != kI2cOk){
        printf("ERROR (host_set_enabled)\n");
    }

}



void test_override()
{
    printf(">> Test override...\n");

    /* This test is to check that I cannot
       modify the pins if first I don't enable 
       the override mode */

    // NOTE: this test is not properly asserted!!!
    // The values in the registers are always written, but they are used by
    // the HW only if the override mode is enabled (check it with gtkwave, it works!!)
    i2c_override_set_enabled(kI2cToggleDisabled);
    printf("\ten?:\t%d\n", bitfield_read(i2c_peri->OVRD, BIT_MASK_1, I2C_OVRD_TXOVRDEN_BIT));
    i2c_override_drive_pins(0, 1);
    printf("\tOVRD:\t%d\n", i2c_peri->OVRD);

    /* This is the one in which I also enable the override */
    i2c_override_set_enabled(kI2cToggleEnabled);
    printf("\ten?:\t%d\n", bitfield_read(i2c_peri->OVRD, BIT_MASK_1, I2C_OVRD_TXOVRDEN_BIT));
    i2c_override_drive_pins(1, 1);
    printf("\tOVRD:\t%d\n", i2c_peri->OVRD);

}

void test_write_byte()
{
    printf(">> Write byte...\n");

    i2c_fmt_t code = kI2cFmtStart;
    uint8_t fmt_lvl = 0;
    uint8_t rx_lvl = 0;

    i2c_get_fifo_levels(&fmt_lvl, &rx_lvl);

    printf("\tfmt_lvl:\t%d\n", fmt_lvl);
    printf("\trx_lvl:\t\t%d\n", rx_lvl);

    i2c_result_t res = i2c_write_byte(42, code, false);

    if(res != kI2cOk){
        printf("\t--|| ERROR! ||--\n");
    } else {
        printf("\tByte written!\n");
    }

    i2c_get_fifo_levels(&fmt_lvl, &rx_lvl);

    printf("\tfmt_lvl:\t%d\n", fmt_lvl);
    printf("\trx_lvl:\t\t%d\n", rx_lvl);

    // Try resetting the FMT fifo
    res  = i2c_reset_fmt_fifo();
    if(res != kI2cOk){
        printf("\t--|| ERROR! ||--\n");
    } else {
        printf("\tFMT fifo reset!\n");
    }
    i2c_get_fifo_levels(&fmt_lvl, &rx_lvl);
    printf("\tfmt_lvl:\t%d\n", fmt_lvl);
    printf("\trx_lvl:\t\t%d\n", rx_lvl);

}


void test_read_byte()
{
    printf(">> Read byte...\n");

    uint8_t byte;

    i2c_read_byte(&byte);

    printf("Read byte:\t%d\n", byte);
}


void test_irq_enable()
{
    printf(">> Enable IRQ...\n");

    // Interrupt type we want to test
    i2c_irq_t irq = kI2cIrqFmtFifoOverflow;

    // SETUP PLIC
    plic_result_t r = plic_Init();
    if(r != kPlicOk) {
        printf("PLIC init failed!\n");
        return -1;
    }

    r = plic_irq_set_priority(I2C_IRQ_ID_START + irq, 1);  
    if(r != kPlicOk){
        printf("Prio setting failed!\n");
    }
    
    r = plic_irq_set_enabled(I2C_IRQ_ID_START + irq, kPlicToggleEnabled);
    if (r != kPlicOk) {
        printf("Failed\n");
        return -1;
    }

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    const uint32_t mask = 1 << 11;
    CSR_SET_BITS(CSR_REG_MIE, mask);
    // END SETUP PLIC

    i2c_toggle_t state = -1;
    i2c_result_t res = 0;

    // Should be disabled
    i2c_irq_get_enabled(irq, &state);
    printf("\tIRQ [%d] en?\t%d\n", irq, state);

    // Enable the irq
    res = i2c_irq_set_enabled(irq, kI2cToggleEnabled);
    if (res != kI2cOk)
    {
        printf("\t--|| Enabling ERROR! ||--\n");
    } else {
        printf("\tIRQ [%d] enabled!\n", irq);
    }

    i2c_irq_get_enabled(irq, &state);
    printf("\tIRQ [%d] en?\t%d\n", irq, state);

    // If a irq is pending
    bool is_pending = false;

    /*
        NB To make this work you must disable the interrupt.
        This will cause the interrupt not to be serviced but the app
        will not get stuck.
        Easy way: in the plic disable the intr line just to test.
        Add something like:
        plic_irq_set_enabled(I2C_ID_START + irq, kPlicToggleDisabled);

        If you want to test the interrupt event keep the interrupt enabled and
        overwrite the weak i2c-handler. The app will get stuck because 
        there is no way of deasserting the interrupt source (it's simulated 
        by writing into a register), so the irq_src_i of the PLIC will 
        stay high and the IRQ will be serviced infinitely.
    */
    plic_irq_set_enabled(I2C_IRQ_ID_START + irq, kPlicToggleDisabled);
    
    plic_intr_flag = 0;
    while (plic_intr_flag == 0){
        res = i2c_irq_force(irq);

        // wait_for_interrupt();
        plic_intr_flag = 1;
    }

    res = i2c_irq_force(irq);
    if (res == kI2cOk)
    {
        printf("\tForced!\n");
    } 
    else {
        printf("\tForcing error!\n");
    }

    printf("\tTest:\t%d\n", i2c_peri->INTR_TEST);

    res = i2c_irq_is_pending(irq, &is_pending);
    if(res != kI2cOk){
        printf("\t--|| is_pending ERROR! ||--\n");
    } 
    else {
        printf("\tRead is_pending!\n");
    }

    printf("\tIs pending:\t%d\n", is_pending);

    res = i2c_irq_acknowledge(irq);
    if(res != kI2cOk){
        printf("\t--|| ACK ERROR! ||--\n");
    } 
    else {
        printf("\tIRQ acknowledged!\n");
    }

    res = i2c_irq_is_pending(irq, &is_pending);
    if(res != kI2cOk){
        printf("\t--|| is_pending ERROR! ||--\n");
    } 
    else {
        printf("\tRead is_pending!\n");
    }

    printf("\tIs pending:\t%d\n", is_pending);

}


void test_irq_snapshot_restore()
{
    /* Here I test the restoring of the INTR enable states
       after resetting them all! */

    printf(">> Snapshot restore...\n");

    i2c_result_t res = 0;

    i2c_irq_set_enabled(kI2cIrqRxWatermarkOverflow, kI2cToggleEnabled);
    i2c_irq_set_enabled(kI2cIrqRxFifoOverflow, kI2cToggleEnabled);
    i2c_irq_set_enabled(kI2cIrqSclInterference, kI2cToggleEnabled);

    printf("\tEnable reg:\t%d\n", i2c_peri->INTR_ENABLE);
    
    i2c_irq_snapshot_t snapshot = 0;

    res = i2c_irq_disable_all(&snapshot);
    if(res != kI2cOk){
        printf("\t--|| disable_all ERROR! ||--\n");
    } else {
        printf("\tDisabled all the IRQ\n");
    }

    printf("\tEnable reg:\t%d\n", i2c_peri->INTR_ENABLE);
    printf("\tSnapshot:\t%d\n", snapshot);

    res = i2c_irq_restore_all(&snapshot);
    if(res != kI2cOk){
        printf("\t--|| restore_all ERROR! ||--\n");
    } else {
        printf("\tRestored the IRQ\n");
    }

    printf("\tEnable reg:\t%d\n", i2c_peri->INTR_ENABLE);
}


// void print_FSM_status()
// {
//     i2c_fsm_t fsm = i2c_state;

//     printf("----------- FSM -----------\n");
//     printf("\tFMT_LVL:\t%d\n", i2c_state.fmt_fifo_level);
//     printf("\tTX_LVL:\t%f\n", i2c_state.tx_fifo_level);
//     printf("\tBYTES_RD:\t%d\n", i2c_state.bytes_read);
//     printf("\tBYTES_WR:\t%d\n", i2c_state.bytes_written);
//     printf("---------------------------\n");
// }


void test_read(){

    printf(">> Read...\n");

    int8_t addr = 0x34;     // hardcoded address of the audio sensor

    int32_t data = 42;

    i2c_read(&data, addr);

}


void test_transmit(){

    printf(">> Transmit...\n");


    uint32_t data_len = 5;
    int8_t *data = (int8_t*)malloc(data_len * sizeof(int8_t));

    for(uint16_t i=0; i<data_len; i++){
        data[i] = i;
    }

    i2c_transfer(data, data_len, false);
    // i2c_transfer_test(data, data_len, false);

    uint8_t fmt_lvl = 0;
    uint8_t rx_lvl = 0;

    i2c_get_fifo_levels(&fmt_lvl, &rx_lvl);
    printf("\tfmt_lvl:\t%d\n", fmt_lvl);

    printf("\tFSM status:\n");
    printf("\t\tbusy:\t\t%d\n", i2c_state.busy);
    printf("\t\tbuffer[3]:\t%d\n", i2c_state.buffer[3]);
    printf("\t\tbuffer_len:\t%d\n", i2c_state.buffer_length);


    i2c_get_fifo_levels(&fmt_lvl, &rx_lvl);
    printf("\tfmt_lvl:\t%d\n", fmt_lvl);

    i2c_get_fifo_levels(&fmt_lvl, &rx_lvl);
    printf("\tfmt_lvl:\t%d\n", fmt_lvl);

    i2c_get_fifo_levels(&fmt_lvl, &rx_lvl);
    printf("\tfmt_lvl:\t%d\n", fmt_lvl);
}

