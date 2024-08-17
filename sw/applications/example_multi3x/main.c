// Author: Ziyue Feng
// Date: 17.08.2024

#include "core_v_mini_mcu.h"
#include "multi3x.h"
// #include "multi3x_regs.h"
// #include "multi3x_structs.h"
#include "mmio.h"

uint32_t multi3x(uint32_t value_to_write){

  mmio_region_t multi3x_base_addr = mmio_region_from_addr((uintptr_t)MULTI3X_START_ADDRESS);      // Get the base address of the peripheral 
  mmio_region_write32(multi3x_base_addr, MULTI3X_DATAIN_REG_OFFSET, value_to_write);            // Write DATAIN
  uint32_t response = mmio_region_read32(multi3x_base_addr, MULTI3X_DATAOUT_REG_OFFSET);          // Read DATAOUT

  return response;
}

int main() {
    uint32_t value_to_write = 10;
    uint32_t response = multi3x(value_to_write);
    printf("Wrote to DATAIN: %u\n", value_to_write);
    printf("Read from DATAOUT: %u\n", response);
    return 0;
}
// make app PROJECT=example_multi3x
