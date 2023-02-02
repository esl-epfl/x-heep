
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "OverlayControl.h"

extern "C" {
#include <libxlnk_cma.h>
}

const uint32_t MEM_IS_CACHEABLE = 0;
const uint32_t MAP_SIZE = 64*1024;
const uint32_t BASE_ADDR = 0x43C00000;
const uint32_t ALLOC_SIZE_BYTE = 64*1024*128;
const uint32_t FIXED_VALUE = 0x1c;

uint32_t *virtual_flash_buffer, *virtual_flash_buffer_phy;

static void stop(int)
{
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);

    if(virtual_flash_buffer != NULL){
        FILE* dump = fopen("./dump.txt", "wb");
        fwrite(virtual_flash_buffer, ALLOC_SIZE_BYTE, 1, dump);
        fclose(dump);
    }
}

int main(int argc, char **argv)
{
    signal(SIGTERM, stop);
    signal(SIGINT, stop);
    volatile uint8_t *pl_peripherals = NULL;
    volatile uint32_t* hijacker = NULL;

    printf("Press ENTER to confirm that the bitstream is loaded (proceeding without it can crash the board).\n");
    getchar();

    // Get the virtual address corresponding to the physical base address of the PL peripherals
    if ((pl_peripherals = (uint8_t *) MapMemIO(BASE_ADDR, MAP_SIZE)) == NULL) {
        printf("Error getting address!\n");
        return -1;
    }
    printf("PL peripherals mapped at 0x%08X.\n", (uint32_t)pl_peripherals);
    fflush(stdout);

    // Map hijacker PL peripheral
    hijacker = (uint32_t *)pl_peripherals;
    printf("hijacker mapped at 0x%08X.\n", (uint32_t)hijacker);

    // Allocate DDR memory buffer to virtualize a Flash memory and get its physical address
    fflush(stdout);
    virtual_flash_buffer = (uint32_t *)cma_alloc(ALLOC_SIZE_BYTE, MEM_IS_CACHEABLE);
    virtual_flash_buffer_phy = (uint32_t *)((uint32_t)cma_get_phy_addr(virtual_flash_buffer));
    printf("DDR memory buffer allocated.\n");
    fflush(stdout);
    printf("Virtual address: 0x%.8X.\n", (uint32_t)virtual_flash_buffer);
    fflush(stdout);
    printf("Physical address: Phys: 0x%.8X.\n\n", (uint32_t)virtual_flash_buffer_phy);
    fflush(stdout);
    if (virtual_flash_buffer == NULL) {
        printf("Error allocating DDR memory for %u bytes!\n", ALLOC_SIZE_BYTE);
        return -1;
    }

    // Initialize DDR memory buffer
    for (uint32_t i = 0; i < ALLOC_SIZE_BYTE / sizeof(uint32_t); i++)
        virtual_flash_buffer[0] = i;

    // Copy the physical address of the DDR memory buffer to hijacker PL peripheral
    *hijacker = (uint32_t)virtual_flash_buffer_phy;

    // Press ENTER to end the application
    printf("Press ENTER to end the application.\n");
    getchar();

    printf("Virtual flash read and content stored in file dump.txt.\n\n");

    // Read content of the DDR memory buffer and store it to a file
    FILE *dump = fopen("./dump.txt", "wb");
    fwrite(virtual_flash_buffer, ALLOC_SIZE_BYTE, 1, dump);
    fclose(dump);

    ///////////////////////////////////////////////////////////////////////////////////////
    // DDR memory is a global variable. In this Linux version, it is allocated
    // by a pl_peripherals driver provided by Xilinx. This means that it is not freed
    // automatically when our process exits. We MUST free it or the board will
    // run out of DMA memory!!!
    ///////////////////////////////////////////////////////////////////////////////////////
    if (virtual_flash_buffer != NULL)
        cma_free(virtual_flash_buffer);
    UnmapMemIO();

    return 0;
}
