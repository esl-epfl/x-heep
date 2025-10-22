#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "x-heep.h"

#include "my_ip.h"
#include "my_ip_structs.h"

/* Inline function to write to registers */
static inline void write_register( uint32_t   p_val,
                                  uint32_t    p_offset,
                                  uint32_t    p_mask,
                                  uint8_t     p_sel,
                                  uint32_t*    p_src)
{
    /*
     * The index is computed to avoid needing to access the structure
     * as a structure.
     */
    uint8_t index = p_offset / sizeof(uint32_t);
    /*
     * An intermediate variable "value" is used to prevent writing twice into
     * the register.
     */
    uint32_t value  =  p_src[ index ];
    value           &= ~( p_mask << p_sel );
    value           |= (p_val & p_mask) << p_sel;
    p_src[ index ] = value;

}



int main(void) {
    // Start read operation
    write_register( 0x1,
                    MY_IP_CONTROL_REG_OFFSET,
                    0x1,
                    MY_IP_CONTROL_START_BIT,
                    MY_IP_START_ADDRESS
                );

    return EXIT_SUCCESS;
}

// Write manually a specific value into FLASH to verify read (like old examples)
