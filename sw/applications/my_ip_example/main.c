#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "x-heep.h"

#include "my_ip.h"
#include "my_ip_structs.h"

int main(void) {
    printf("===== my_ip test start =====\n");

    // Clear interrupt state
    my_ip_peri->INTR_STATE = 0xFFFFFFFF;

    // Enable interrupt
    my_ip_peri->INTR_ENABLE = 0x1;

    // Write test value
    uint32_t wval = 0xABCD1234;
    my_ip_peri->TEST_REG_W = wval;
    printf("Wrote  0x%08X\n", wval);

    // Read back to verify writing
    uint32_t read_back = my_ip_peri->TEST_REG_W;
    printf("Readback (W reg): 0x%08X\n", read_back);

    if (read_back == wval)
        printf("✔ Write verified OK\n");
    else
        printf("✘ Write mismatch (expected 0x%08X)\n", wval);

    // Read test value
    uint32_t rval = my_ip_peri->TEST_REG_R;
    printf("Read   0x%08X\n", rval);

    // Trigger software interrupt
    my_ip_peri->INTR_TEST = 0x1;
    printf("SW interrupt triggered\n");

    // Read interrupt state
    uint32_t intr = my_ip_peri->INTR_STATE;
    printf("Interrupt state: 0x%08X\n", intr);

    printf("=== done ===\n");
    return 0;
}
