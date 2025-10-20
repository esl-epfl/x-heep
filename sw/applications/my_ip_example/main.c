#include <stdio.h>
#include <stdlib.h>

#include "core_v_mini_mcu.h"
#include "x-heep.h"

#include "my_ip.h"
#include "my_ip_structs.h"

#include "spi_host.h" // SPI flash is an instantiation of spi_host

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
    // Setting test conditions (values to write read from)
    write_register( 0xAB,
                    SPI_HOST_CONTROL_REG_OFFSET,
                    SPI_HOST_CONTROL_RX_WATERMARK_MASK,
                    SPI_HOST_CONTROL_RX_WATERMARK_OFFSET,
                    SPI_FLASH_START_ADDRESS
                );

    write_register( 0x1,
                    SPI_HOST_CONTROL_REG_OFFSET,
                    0x1,
                    SPI_HOST_CONTROL_SPIEN_BIT,
                    SPI_FLASH_START_ADDRESS
                );
    
    // Reading set values
        // Set address to read from
    write_register( SPI_FLASH_START_ADDRESS + SPI_HOST_CONTROL_REG_OFFSET,
                    MY_IP_ADDRESS_REG_OFFSET,
                    0xffffffff,
                    0,
                    MY_IP_START_ADDRESS
                );
    
        // Start read operation
    write_register( 0x1,
                    MY_IP_CONTROL_REG_OFFSET,
                    0x1,
                    MY_IP_CONTROL_START_BIT,
                    MY_IP_START_ADDRESS
                );

    // while ((my_ip_peri->STATUS & 0x1) == 0) {
    //     // While not finished with a transaction
    // }

    // // Now READY == 1 → perform the action
    // write_register(
    //     SPI_FLASH_START_ADDRESS + SPI_HOST_STATUS_REG_OFFSET,
    //     MY_IP_ADDRESS_REG_OFFSET,
    //     0xffffffff,
    //     0,
    //     MY_IP_START_ADDRESS
    // );

    while ((my_ip_peri->STATUS & 0x1) == 0) {
        // While not finished with a transaction
    }

    // TRY WRITING
        // Set write enable
    write_register( 0x1,
                    MY_IP_CONTROL_REG_OFFSET,
                    0x1,
                    MY_IP_CONTROL_WRITE_BIT,
                    MY_IP_START_ADDRESS
                );
        // Set data to write
    write_register( 0xffffffff,
                    MY_IP_DATA_REG_OFFSET,
                    0xffffffff,
                    0,
                    MY_IP_START_ADDRESS
                );


    return EXIT_SUCCESS;
}

// Issues:
// Compile reorders? Produces erros see screenshot
// Trying to limit write/read to once (not keep going as long as start set to 1)
// Failed to do so in HW hence was exploring hw/sw solutions (ready bit)
// However came across the fact that my if fails and that write_registers which follow each other, overwrite each other

// How to write to register in my_ip
// uint32_t wval = 0x00000001; // XXX VERIFY SW WRITES
// my_ip_peri->TEST_REG_W = wval; // Start OBI FSM

// How to write to a sequence of bits in a register (REGISTER MUST BE WRITABLE IN SW!)
    // write_register( 0xAB,
    //                 SPI_HOST_CONTROL_REG_OFFSET,
    //                 SPI_HOST_CONTROL_RX_WATERMARK_MASK,
    //                 SPI_HOST_CONTROL_RX_WATERMARK_OFFSET,
    //                 SPI_FLASH_START_ADDRESS
    //             );

// How to write to a specific bit in a register (REGISTER MUST BE WRITABLE IN SW!)
    // write_register( 0x1,
    //             SPI_HOST_CONTROL_REG_OFFSET,
    //             0x1,
    //             SPI_HOST_CONTROL_SPIEN_BIT,
    //             SPI_FLASH_START_ADDRESS
    //         );

// Alternative to write to a register
//spi_host_t* spi = spi_flash;
//SPI_HW(spi)->CONTROL = wval;

// printf("T\n");