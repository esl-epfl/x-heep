#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "x-heep.h"

#include "my_ip.h"
#include "my_ip_structs.h"

int main(void) {
    // Write test value
    uint32_t wval = 0x00000001;
    my_ip_peri->TEST_REG_W = wval;

    return 0;
}
