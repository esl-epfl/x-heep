/**
 * @file main.c
 * @brief SPI OBI slave IP functionality test
 * @author Fabian Aegerter
 *
*/


#include <stdint.h>
#include <stdlib.h>

#include "x-heep.h"
#include "spi_host.h"
#include "buffer.h"


/** Macros for operations. */
#define DIV_ROUND_UP(numerator, denominator) ((numerator + denominator - 1) / denominator)
#define LOWER_BYTE_16_BITS(bytes)(bytes & 0xFF)
#define UPPER_BYTE_16_BITS(bytes)((bytes & 0xFF00) >> 8)
#define LOWER_BYTES_32_BITS(bytes)(bytes & 0xFFFF)
#define UPPER_BYTES_32_BITS(bytes)((bytes & 0xFFFF0000) >> 16)

/** Macros for SPI SLAVE hardware. */
#define WRITE_SPI_SLAVE_REG_0 0x11
#define WRITE_SPI_SLAVE_REG_1 0x20
#define WRITE_SPI_SLAVE_REG_2 0x30
#define READ_SPI_SLAVE_CMD 0xB
#define WRITE_SPI_SLAVE_CMD 0x2
#define WORD_SIZE_IN_BYTES 4
#define MAX_DATA_SIZE 0x10000       //The values for Max data size and last valid address are arbitrary and aren't correct. 
                                    //They exist to highlight their importance for an actual implementation.

#define LAST_VALID_ADDRESS 0x40000  //This code assumes that the memory addresses start at 0. 
                                    //Hence the first valid address hasn't been included 

/** Enum for SPI operation status flags. */
typedef enum {
    // Everithing went well
    SPI_FLAG_SUCCESS                        = 0x0000,        
    //The SPI host was not properly initalized
    SPI_HOST_FLAG_NOT_INIT                  = 0x0001,   
    //The target address is invalid
    SPI_SLAVE_FLAG_ADDRESS_INVALID          = 0x0002,
    // The CSID was out of the bounds specified in SPI_HOST_PARAM_NUM_C_S 
    SPI_HOST_FLAG_CSID_INVALID              = 0x0003,    
    //The amount of data exceeds the memory capacity of the SPI SLAVE (X-HEEP)
    SPI_SLAVE_FLAG_SIZE_OF_DATA_EXCEEDED    = 0x0004, 
} spi_flags_e;


spi_host_t* spi_hst; 

/** Arrays used for testing. */
uint32_t target_data_1[DATA_1_LENGTH/4]; //The target destination of the dataset 1 in the RAM
uint32_t target_data_2[DATA_2_LENGTH/4]; //The target destination of the dataset 2 in the RAM
uint32_t target_data_3[DATA_3_LENGTH/4]; //The target destination of the dataset 3 in the RAM



/** Enums used for testing. */
typedef enum {
    OPERATION_WRITE,
    OPERATION_READ
} OperationType;

typedef enum{
    DATASET_1,
    DATASET_2,
    DATASET_3
} DatasetNumber;

/** Structs used for testing. */
typedef struct {
    uint8_t dataset_number;
    OperationType operation_type;
    uint8_t dummy_cycles;
} TestOperation;

typedef struct {
    uint32_t *test_data;
    uint32_t *target_data;
    uint16_t data_length;
} Dataset;

spi_flags_e spi_host_init(spi_host_t* host);
static spi_flags_e spi_slave_write(uint32_t addr, uint32_t *data, uint16_t length);
spi_flags_e spi_slave_read(uint32_t addr, void* data, uint16_t length, uint8_t dummy_cycles);
void print_array(const char *label, uint32_t *array, uint16_t size);
static void configure_spi();
void spi_slave_write_dummy_cycles(uint8_t cycles); 
void spi_host_wait(uint8_t cycles);
uint32_t make_word_compatible_for_spi_host(uint32_t word);
void make_compare_data_compatible(uint32_t *compare_data, uint16_t length);
void spi_slave_write_wrap_length(uint16_t length);
static void send_command_to_spi_host(uint32_t len, bool csaat, uint8_t direction);
bool test_process(Dataset *dataset, OperationType operation, uint8_t dummy_cycles);

int main(int argc, char *argv[])
{   
    spi_hst = spi_host1;
    spi_return_flags_e flags;
    flags = spi_host_init(spi_hst);
    if (flags != SPI_FLAG_SUCCESS){
        printf("Failure to initialize\n Error code: %d", flags);
        return EXIT_FAILURE;
    }

    Dataset all_test_datasets[] = { {test_data_1, target_data_1, DATA_1_LENGTH},
                                    {test_data_2, target_data_2, DATA_2_LENGTH},
                                    {test_data_3, target_data_3, DATA_3_LENGTH}
    };

    //Every entry of this array is a test that gets executed with the desired dataset, operation type and amount of dummy cycles
    TestOperation test_order[] =   {{DATASET_1, OPERATION_WRITE, 0},  //dummy_cycles = 0 (not used)
                                    {DATASET_1, OPERATION_READ, 32},
                                    {DATASET_2, OPERATION_WRITE, 0},  //dummy_cycles = 0 (not used)
                                    {DATASET_2, OPERATION_READ, 16},
                                    {DATASET_3, OPERATION_WRITE, 0},  //dummy_cycles = 0 (not used)
                                    {DATASET_3, OPERATION_READ, 8},
                                    {DATASET_1, OPERATION_READ, 6},
                                    {DATASET_2, OPERATION_READ, 6},   //The minimum amount of dummy cycles is 6.
                                    {DATASET_3, OPERATION_READ, 6}    //Otherwise it breaks.
    };


    for(uint32_t i = 0; i < sizeof(test_order) / sizeof(test_order[0]); i++){
        if(test_process(&all_test_datasets[test_order[i].dataset_number], test_order[i].operation_type, test_order[i].dummy_cycles) == EXIT_FAILURE){
            printf("Error in operation: %d", i);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

static void send_command_to_spi_host(uint32_t len, bool csaat, uint8_t direction){
    if(direction != SPI_DIR_DUMMY){
        len--; //The SPI HOST IP uses len-1 = amount of bytes to read and write. But also len = amount of dummy cycles
    }
    const uint32_t send_cmd_byte = spi_create_command((spi_command_t){
        .len        = len,                     
        .csaat      = csaat,                    // Command not finished e.g. CS remains low after transaction
        .speed      = SPI_SPEED_STANDARD,       // Single speed
        .direction  = direction
    });
    spi_set_command(spi_hst, send_cmd_byte);
    spi_wait_for_ready(spi_hst);
}

spi_flags_e spi_slave_read(uint32_t addr, void* data, uint16_t length, uint8_t dummy_cycles){
    /*  The  OBI SPI slave IP has no way of knowing or handling incorrect addresses. 
    *   Using an incorrect address leads to an abort of the simulation and  it is   
    *   unkown what happens when this issue occurs after physical implementation. 
    *   This is why it is absolutely necessary to include checks such as the following two lines. 
    */
    if(DIV_ROUND_UP(length, WORD_SIZE_IN_BYTES) > MAX_DATA_SIZE) return SPI_SLAVE_FLAG_SIZE_OF_DATA_EXCEEDED;
    if (addr % 4 != 0 || (addr + length) > LAST_VALID_ADDRESS) return SPI_SLAVE_FLAG_ADDRESS_INVALID;
    

    spi_slave_write_dummy_cycles(dummy_cycles);

    spi_slave_write_wrap_length(length);
 

    spi_write_byte(spi_hst, READ_SPI_SLAVE_CMD);
    spi_wait_for_ready(spi_hst);

    send_command_to_spi_host(1, true, SPI_DIR_TX_ONLY);

    //write address
    spi_write_word(spi_hst, make_word_compatible_for_spi_host(addr));
    spi_wait_for_ready(spi_hst); 
    send_command_to_spi_host(4, true, SPI_DIR_TX_ONLY);

    spi_host_wait(dummy_cycles);

    send_command_to_spi_host(length, false, SPI_DIR_RX_ONLY);

    /*
     * Set RX watermark to length. The watermark is in words.
     * If the length is not a multiple of 4, the RX watermark is set to length/4+1
     * to take into account the extra bytes.
     * If the length is higher then the RX FIFO depth, the RX watermark is set to
     * RX FIFO depth. In this case the flag is not set to 0, so the loop will
     * continue until all the data is read.
    */
    bool flag = 1;
    uint16_t to_read = 0;
    uint16_t i_start = 0;
    uint16_t length_original = length;
    uint32_t *data_32bit = (uint32_t *)data;
    while (flag) {
        if (length >= SPI_HOST_PARAM_RX_DEPTH) {
            spi_set_rx_watermark(spi_hst, SPI_HOST_PARAM_RX_DEPTH>>2);
            length -= SPI_HOST_PARAM_RX_DEPTH;
            to_read += SPI_HOST_PARAM_RX_DEPTH;
        }
        else {
            spi_set_rx_watermark(spi_hst, (length%4==0 ? length>>2 : (length>>2)+1));
            to_read += length;
            flag = 0;
        }
        // Wait till SPI Host RX FIFO is full (or I read all the data)
        spi_wait_for_rx_watermark(spi_hst);
        // Read data from SPI Host RX FIFO
        for (uint16_t i = i_start; i < to_read>>2; i++) {
            spi_read_word(spi_hst, &data_32bit[i]); // Writes a full word
        }
        // Update the starting index
        i_start += SPI_HOST_PARAM_RX_DEPTH>>2;
    }
    // Take into account the extra bytes (if any)
    if (length_original % 4 != 0) {
        uint32_t last_word = 0;
        spi_read_word(spi_hst, &last_word);
        memcpy(&data_32bit[length_original>>2], &last_word, length%4);
    }
    
    return SPI_FLAG_SUCCESS; // Success
}


spi_flags_e spi_host_init(spi_host_t* host) {
 
    // Set the global spi variable to the one passed as argument.
    spi_hst = host;

    // Enable spi host device
    spi_return_flags_e test = spi_set_enable(spi_hst, true);
    if(test != SPI_FLAG_SUCCESS){
        return SPI_HOST_FLAG_NOT_INIT;
    };

    // Enable spi output
    if(spi_output_enable(spi_hst, true) != SPI_FLAG_SUCCESS){
        return SPI_HOST_FLAG_NOT_INIT;
    };
    // Configure spi Master<->Slave connection on CSID 0
    configure_spi();

    // Set CSID
    if(spi_set_csid(spi_hst, 0) != SPI_FLAG_SUCCESS){
        return SPI_HOST_FLAG_CSID_INVALID;
    };

    return SPI_FLAG_SUCCESS; // Success
}



static spi_flags_e spi_slave_write(uint32_t addr, uint32_t *data, uint16_t length) {
    /*  The  OBI SPI slave IP has no way of knowing or handling incorrect addresses. 
     *  Using an incorrect address leads to an abort of the simulation and  it is   
     *  unkown what happens when this issue occurs after physical implementation. 
     *  This is why it is absolutely necessary to include checks such as the following two lines. 
    */
    if(DIV_ROUND_UP(length, WORD_SIZE_IN_BYTES) > MAX_DATA_SIZE) return SPI_SLAVE_FLAG_SIZE_OF_DATA_EXCEEDED;
    if (addr % 4 != 0 || (addr + length) > LAST_VALID_ADDRESS) return SPI_SLAVE_FLAG_ADDRESS_INVALID;


    spi_slave_write_wrap_length(length);

    spi_write_byte(spi_hst, WRITE_SPI_SLAVE_CMD);
    spi_wait_for_ready(spi_hst); 
    send_command_to_spi_host(1, true, SPI_DIR_TX_ONLY);

    ///write address
    spi_write_word(spi_hst, make_word_compatible_for_spi_host(addr));
    spi_wait_for_ready(spi_hst); 
    send_command_to_spi_host(4, true, SPI_DIR_TX_ONLY);

 
    /*
     * Place data in TX FIFO
     * In simulation it do not wait for the flash to be ready, so we must check
     * if the FIFO is full before writing.
    */
    uint16_t counter = 0;
    uint32_t *data_32bit = (uint32_t *)data;
    for (uint16_t i = 0; i < (length>>2 ); i++) {
        if(SPI_HOST_PARAM_TX_DEPTH == counter){
            send_command_to_spi_host(SPI_HOST_PARAM_TX_DEPTH*WORD_SIZE_IN_BYTES, true, SPI_DIR_TX_ONLY);
            counter = 0;
        }
        spi_wait_for_tx_not_full(spi_hst);
        spi_write_word(spi_hst, make_word_compatible_for_spi_host(data_32bit[i]));
        counter++;
    }
    if (length % 4 != 0) {
        uint32_t last_word = 0;
        memcpy(&last_word, &data[length - length % 4], length % 4);
        spi_wait_for_tx_not_full(spi_hst);
        spi_write_word(spi_hst, make_word_compatible_for_spi_host(last_word));
    }

    uint16_t last_words = length-((length/(SPI_HOST_PARAM_TX_DEPTH*4))*(SPI_HOST_PARAM_TX_DEPTH*4));

    if(last_words == 0){
        last_words = SPI_HOST_PARAM_TX_DEPTH*4;
    }

    send_command_to_spi_host(last_words, false, SPI_DIR_TX_ONLY);
    spi_wait_for_tx_empty(spi_hst);
    return SPI_FLAG_SUCCESS; // Success
}


static void configure_spi() {
    // Configure spi clock
    uint16_t clk_div = 0;

    // Spi Configuration
    // Configure chip 0 (slave)
    const uint32_t chip_cfg = spi_create_configopts((spi_configopts_t){
        .clkdiv     = clk_div,
        .csnidle    = 0xF,
        .csntrail   = 0xF,
        .csnlead    = 0xF,
        .fullcyc    = false,
        .cpha       = 0,
        .cpol       = 0            
    });
    spi_set_configopts(spi_hst, 0, chip_cfg);
}


/*
*   This function is optional since 32 are set by default.
*   Also when setting the dummy cycles too low it breaks the  
*   SPI OBI slave IP since the buffer is too slow to process 
*   the data.
*/
void spi_slave_write_dummy_cycles(uint8_t cycles){

    // Load command to TX FIFO
    spi_write_byte(spi_hst, WRITE_SPI_SLAVE_REG_0);
    spi_wait_for_ready(spi_hst);

    //The spi_write_byte function fills always the same byte in the TX FIFO of the SPI HOST IP
    //Hence it is not possible to write the reg 1 command and cycles at the same time without writing a word.
    send_command_to_spi_host(1, true, SPI_DIR_TX_ONLY);

    // Load command to TX FIFO
    spi_write_byte(spi_hst, cycles); 
    spi_wait_for_ready(spi_hst);

    send_command_to_spi_host(1, true, SPI_DIR_TX_ONLY);
}


void spi_slave_write_wrap_length(uint16_t length){

    uint32_t wrap_length_cmds = (WRITE_SPI_SLAVE_REG_1 << 24)               //Write register 1
                                + (LOWER_BYTE_16_BITS(length>>2) << 16)     //Wraplength low 
                                + (WRITE_SPI_SLAVE_REG_2 << 8)              //Write register 2
                                + (UPPER_BYTE_16_BITS(length>>2));          //Wraplength high

    spi_write_word(spi_hst, make_word_compatible_for_spi_host(wrap_length_cmds));
    spi_wait_for_ready(spi_hst);

    send_command_to_spi_host(4, true, SPI_DIR_TX_ONLY);
}

/*
* This function activates the SPI clock for the specified amount of cycles.
*/
void spi_host_wait(uint8_t cycles){
    if(cycles == 0){
        return;
    }
    send_command_to_spi_host(cycles, true, SPI_DIR_DUMMY);
}

void print_array(const char *label, uint32_t *array, uint16_t size) {
    printf("%s: [", label);
    for (uint16_t i = 0; i < size>>2; i++) {
        printf("%d", array[i]);
        if (i < size - 1) {
            printf(", ");
            printf("]\n");
        }
    }
    printf("]\n");
} 

/*
* The SPI Host IP changes the byte order. This helper function changes the byte order sent to the SPI Host IP 
* such that the correct byte order will be transmitted to the SPI slave
*/
uint32_t make_word_compatible_for_spi_host(uint32_t word){
    return (LOWER_BYTE_16_BITS(LOWER_BYTES_32_BITS(word)) << 24) 
        | (UPPER_BYTE_16_BITS(LOWER_BYTES_32_BITS(word)) << 16) 
        | (LOWER_BYTE_16_BITS(UPPER_BYTES_32_BITS(word)) << 8) 
        | UPPER_BYTE_16_BITS(UPPER_BYTES_32_BITS(word));
}

/*
* The SPI Host IP also shuffles the byte order when receiving data. 
* The following helper function corrects the byte order of the received data.
*/
void make_compare_data_compatible(uint32_t *compare_data, uint16_t length){
    for(uint16_t i = 0; i < length>>2; i++){
        compare_data[i] = make_word_compatible_for_spi_host(compare_data[i]);
    }    
}


bool test_process(Dataset *dataset, OperationType operation, uint8_t dummy_cycles){
    
    spi_return_flags_e flags;
    if(operation == OPERATION_WRITE){
        flags = spi_slave_write(dataset->target_data, dataset->test_data, dataset->data_length);
        if (flags != SPI_FLAG_SUCCESS){
            printf("Failure to write\n Error code: %d", flags);
            return EXIT_FAILURE;
        }
        if (memcmp(dataset->target_data, dataset->test_data, dataset->data_length/4) != 0) {
            printf("Failure to send correct data\n");
            return EXIT_FAILURE;
        }
    } 
    else{
        uint32_t *compare_data_read = (uint32_t *)calloc(dataset->data_length/4, sizeof(uint32_t));
        flags = spi_slave_read(dataset->target_data, compare_data_read, dataset->data_length, dummy_cycles);
        if (flags != SPI_FLAG_SUCCESS){
            printf("Failure to read\n Error code: %d", flags);
            free(compare_data_read);
            return EXIT_FAILURE;
        }
        make_compare_data_compatible(compare_data_read, dataset->data_length);
        if (memcmp(dataset->test_data, compare_data_read, dataset->data_length/4) != 0) {
            printf("Failure to retrieve correct data\n");
            free(compare_data_read);
            return EXIT_FAILURE;
        }
        free(compare_data_read);
    }



    return EXIT_SUCCESS;
}