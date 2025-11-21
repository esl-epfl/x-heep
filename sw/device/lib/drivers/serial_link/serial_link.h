
#include <stdio.h>
#include <stdlib.h>
#include "x-heep.h"
#include "core_v_mini_mcu.h"
#include "csr.h"


#define SL_INTERNAL_WRITE  (int32_t *)(SERIAL_LINK_START_ADDRESS)

#define SL_INTERNAL_READ   (int32_t *)(SERIAL_LINK_RECEIVER_FIFO_START_ADDRESS)

#define SL_EXTERNAL_WRITE  (int32_t *)(EXT_SLAVE_START_ADDRESS + 0x20000)

#define SL_EXTERNAL_READ   





void REG_CONFIG(void);
void REG_CONFIG_MULTI(void);
void RAW_MODE_EN(void);
void AXI_ISOLATE(void);
void EXTERNAL_BUS_SL_CONFIG(void);
void SIM_INIT(void);
