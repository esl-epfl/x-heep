#include <stdint.h>

#define DATA_LENGTH 0x14 //In bytes 
#define TARGET_ADDRESS 0x04


uint32_t test_data[DATA_LENGTH/4] = {
    0xFF00AA, 0xAABBCC, 0xFF1122CC, 0x2032AA, 0x0
};