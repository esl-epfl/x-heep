#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "x-heep.h"

#include "my_ip.h"
#include "my_ip_structs.h"

#include "spi_host.h"

int main(void) {
    // Write test value
    uint32_t wval = 0x00000001;
    my_ip_peri->TEST_REG_W = wval;

    // Read control register of SPI
    wval = 0x80000000;
    spi_host_peri->CONTROL = wval;
    volatile uint32_t rval = spi_host_peri->CONTROL;
    // printf("SPI CR: 0x%08X\n", rval);

    return 0;
}
