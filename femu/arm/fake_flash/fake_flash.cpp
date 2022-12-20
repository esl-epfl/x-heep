#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
extern "C" {
#include <libxlnk_cma.h>
}
#include "OverlayControl.h"

const uint32_t MEM_IS_CACHEABLE = 0;

// Adjust the size of the mapping to cover all the peripherals, or use multiple mappings.
const uint32_t MAP_SIZE = 64*1024;
// Base address for the mapping of (all) peripherals
const uint32_t BASE_ADDR = 0x40000000;
 uint32_t * vector, * phyVector;

const uint32_t ALLOC_SIZE = 64 * 1024 * 128;
const uint32_t FIXED_VALUE = 0x1c;
static void stop(int)
{
signal(SIGTERM, SIG_DFL);
signal(SIGINT, SIG_DFL);

if(vector != NULL){
  FILE* dump = fopen("./dump.txt", "wb");
  fwrite(vector, ALLOC_SIZE, 1, dump);
  fclose(dump);
}


}

int main(int argc, char ** argv)
{
signal(SIGTERM, stop);
signal(SIGINT, stop);
  volatile uint8_t * device = NULL;

  printf("This program has to be run with sudo.\n");
  printf("Press ENTER to confirm that the bitstream is loaded (proceeding without it can crash the board).\n\n");
  getchar();

  // Obtain a pointer to access the peripherals in the address map.
  if ( (device = (uint8_t*) MapMemIO(BASE_ADDR, MAP_SIZE)) == NULL ) {
    printf("Error opening accelRegs!\n");
    return -1;
  }
  printf("Mmap done. Accelerator registers mapped at 0x%08X\n", (uint32_t)device);
  fflush(stdout);

  volatile uint32_t* HIJACKER = (uint32_t*)(device);

  printf("HIJACKER at 0x%08X\n", (uint32_t)HIJACKER);
  fflush(stdout);

  printf("Allocating DMA memory...\n");
  fflush(stdout);
  vector = (uint32_t *)cma_alloc(ALLOC_SIZE, MEM_IS_CACHEABLE);
  phyVector = (uint32_t*)((uint32_t)cma_get_phy_addr(vector));
  printf("DMA memory allocated.\n");
  fflush(stdout);
  printf("Vector: Virt: 0x%.8X (%u) // Phys: 0x%.8X (%u)\n", (uint32_t)vector, (uint32_t)vector, (uint32_t)phyVector, (uint32_t)phyVector);
  fflush(stdout);

  if (vector == NULL) {
    printf("Error allocating DMA memory for %u bytes.\n", ALLOC_SIZE);
    return -1;
  }

 
  // Fill the vector, randomly picking the correct value or a wrong (random) one.
  //srand(time(NULL));
  for (uint32_t ii = 0; ii < ALLOC_SIZE / sizeof(uint32_t); ++ii)
    *((uint8_t*)vector+ii)= ii;

  *HIJACKER = (uint32_t)phyVector;

  //printf("Written value: %x\n", (uint32_t)(*HIJACKER));
  //fflush(stdout);
  //printf("Written value: %x\n", (uint32_t)(*HIJACKER));
  //fflush(stdout);
  //printf("Written value: %x\n", (uint32_t)(*HIJACKER));
  //fflush(stdout);
  //printf("Written value: %x\n", (uint32_t)(*HIJACKER));
  //fflush(stdout);
  //
  //

  
  /*
  FILE* hex = fopen("./big_endian.hex", "rb");
  fseek(hex, 0L, SEEK_END);
  size_t hex_size = ftell(hex);
  rewind(hex);
  for(uint32_t i = 0; i < hex_size; ++i)
  {
	  uint8_t byte_val;
	  fread(&byte_val, sizeof(byte_val), 1, hex);
	  *((uint8_t*)vector + i) = byte_val;
	  if(i % 1000 == 0)
	  {
	  	printf("Read: %x\n", byte_val);
	  }
  }
  fclose(hex);
  */
//
  //FILE* dump = fopen("./dump.txt", "wb");
  //fwrite(vector, hex_size, 1, dump);
  //fclose(dump);

  getchar();

  FILE* dump = fopen("./dump.txt", "wb");
  fwrite(vector, ALLOC_SIZE, 1, dump);
  fclose(dump);



  //////////////////////////////////////////////  
  // DMA memory is a global variable. In this Linux version, it is allocated
  // by a device driver provided by Xilinx. This means that it is not freed
  // automatically when our process exits. We MUST free it or the board will
  // run out of DMA memory!!!
  if (vector != NULL)
    cma_free(vector);
  UnmapMemIO();

  return 0;
}

