#include <stdio.h>

#include "csr_registers.h"
#include "csr.h"
#include "x_time.h"

static uint32_t start_time_val, stop_time_val;


void X_start_time(void)
{
    // Enable mcycle counter and read value
    CSR_CLEAR_BITS(CSR_REG_MCOUNTINHIBIT, 0x1);

    CSR_READ(CSR_REG_MCYCLE, &start_time_val);
}

void X_stop_time(void)
{
    CSR_READ(CSR_REG_MCYCLE, &stop_time_val);
}

uint32_t X_get_time(void)
{
    return (stop_time_val - start_time_val);
}

uint32_t X_time_in_secs(uint32_t ticks)
{
    return ticks*16E-6;  // Normalized to 16 MHz clock period
}

uint32_t X_time_in_msecs(uint32_t ticks)
{
    return ticks*16E-3;  // Normalized to 16 MHz clock period
}

void X_milli_delay(int n_milli_seconds)
{
    // Converting time into cycles
    //factor found for ZYNQ-Z2 through experimenation
    int cycles = 4*1000 * n_milli_seconds;
 
    for (int i=0; i<cycles; i++) asm volatile("nop;");

}

void X_micro_delay(int n_milli_seconds)
{
    // Converting time into cycles
    //factor found for ZYNQ-Z2 through experimenation
    int cycles = 4 * n_milli_seconds;
 
    for (int i=0; i<cycles; i++) asm volatile("nop;");

}