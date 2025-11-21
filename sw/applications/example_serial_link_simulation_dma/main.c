#include <stdio.h>
#include <stdlib.h>
#include "serial_link_single_channel_regs.h"
#include "serial_link_regs.h"
#include "serial_link.h"


#define DMA_DATA_LARGE 8 
#define TEST_DATA_LARGE 30

static uint32_t to_be_sent_4B[TEST_DATA_LARGE] __attribute__((aligned(4))) = {0};
static uint32_t copied_data_4B[TEST_DATA_LARGE] __attribute__((aligned(4))) = {0};




int main(int argc, char *argv[]){
    
    SIM_INIT();
    
    for (int i = 0; i < TEST_DATA_LARGE; i++) {
        to_be_sent_4B[i] = i+1;
    }
    

    uint32_t chunks = TEST_DATA_LARGE / DMA_DATA_LARGE;
    uint32_t remainder = TEST_DATA_LARGE % DMA_DATA_LARGE;
    
    // DMA
    for (uint32_t i = 0; i < chunks; i++) {
        SL_DMA_TRANS(to_be_sent_4B + i * DMA_DATA_LARGE, copied_data_4B + i * DMA_DATA_LARGE, DMA_DATA_LARGE,SL_EXTERNAL_WRITE,SL_INTERNAL_READ );
    }
    if (remainder > 0) {
        SL_DMA_TRANS(to_be_sent_4B + chunks * DMA_DATA_LARGE, copied_data_4B + chunks * DMA_DATA_LARGE,SL_EXTERNAL_WRITE,SL_INTERNAL_READ,remainder);
    }
    printf("DMA DONE\n"); 

    // CPU
    for (uint32_t i = 0; i < chunks; i++) {
        SL_CPU_TRANS(to_be_sent_4B + i * DMA_DATA_LARGE, copied_data_4B + i * DMA_DATA_LARGE, DMA_DATA_LARGE,SL_EXTERNAL_WRITE,SL_INTERNAL_READ );
    }
    if (remainder > 0) {        
        SL_CPU_TRANS(to_be_sent_4B + chunks * DMA_DATA_LARGE, copied_data_4B + chunks * DMA_DATA_LARGE,SL_EXTERNAL_WRITE,SL_INTERNAL_READ,remainder);
    }

    printf("CPU DONE\n"); 
    printf("data saved:\n");
    for (int i = 0; i < TEST_DATA_LARGE; i++) {
        printf("%x\n", copied_data_4B[i]);
    }

    printf("DONE\n");  
    return EXIT_SUCCESS;
}





