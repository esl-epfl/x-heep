#ifndef CACHE_H
#define CACHE_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>


// Target module representing a simple direct mapped cache
class CacheMemory
{

public:
  uint32_t cache_size_byte    = 4*1024;
  uint32_t number_of_blocks   = 256;

  uint32_t nbits_blocks       = 0;
  uint32_t nbits_tags         = 0;
  uint32_t nbits_index        = 0;
  uint32_t block_size_byte    = 0;

  enum { ARCHITECTURE_bits = 32 };

  std::ofstream cacheFile;

  typedef struct cache_line {
    uint32_t tag;
    bool    valid;
    uint8_t* data;
  } cache_line_t;

  cache_line_t* cache_array;


  CacheMemory(): cacheFile("cache_status.log")
  {
    cache_array = NULL;
  }

  void create_cache() {
      cache_array = new cache_line_t[number_of_blocks];
      this->block_size_byte = get_block_size();
      this->nbits_blocks    = log2(block_size_byte);
      this->nbits_index     = log2(number_of_blocks);
      this->nbits_tags      = ARCHITECTURE_bits - nbits_index - nbits_blocks;
      printf("bits block %d, index %d, tags %d\n",nbits_blocks, nbits_index, nbits_tags );
  }

  void create_cache(uint32_t cache_size_byte, uint32_t number_of_blocks) {
      this->cache_size_byte = cache_size_byte;
      this->number_of_blocks = number_of_blocks;
      cache_array = new cache_line_t[number_of_blocks];
      this->block_size_byte = get_block_size();
      this->nbits_blocks    = log2(block_size_byte);
      this->nbits_index     = log2(number_of_blocks);
      this->nbits_tags      = ARCHITECTURE_bits - nbits_index - nbits_blocks;
  }

  uint32_t initialize_cache() {
      if(cache_array == NULL) {
        return -1;
      }
      // Initialize memory with random data
      for (int i = 0; i < number_of_blocks; i++) {
        cache_array[i].valid = false;
        cache_array[i].tag   = 0;
        cache_array[i].data = new uint8_t[block_size_byte];
        for(int j = 0; j<block_size_byte;j++) {
          cache_array[i].data[j] = (uint8_t)(i*j);
        }
      }
      return 0;
  }

  uint32_t get_block_size() {
    return (uint32_t)(cache_size_byte / number_of_blocks);
  }

  uint32_t get_index(uint32_t address) {
    uint32_t mask_index = (1 << nbits_index) - 1;
    return (uint32_t)((address >> nbits_blocks) & mask_index );
  }

  uint32_t get_block_offset(uint32_t address) {
    uint32_t mask_block = (1 << nbits_blocks) - 1;
    return (uint32_t)(address & mask_block);
  }

  uint32_t get_base_address(uint32_t address) {
    return (uint32_t)((address >> nbits_blocks) << nbits_blocks);
  }

  uint32_t get_tag(uint32_t address) {
    return (uint32_t)(address >> (nbits_index+nbits_blocks));
  }

  uint32_t get_tag_from_index(uint32_t index) {
    return cache_array[index].tag;
  }


  bool cache_hit(uint32_t address) {
    uint32_t index = get_index(address);
    uint32_t tag   = get_tag(address);
    return ( cache_array[index].valid && tag == cache_array[index].tag);

  }

  void add_entry(uint32_t address, uint8_t* new_data) {
    uint32_t index = get_index(address);
    uint32_t tag   = get_tag(address);
    cache_array[index].valid = true;
    cache_array[index].tag   = tag;
    memcpy(cache_array[index].data, new_data, block_size_byte);
  }

  void get_data(uint32_t address, uint8_t* new_data) {
    uint32_t index = get_index(address);
    memcpy(new_data, cache_array[index].data, block_size_byte);
  }

  void get_data_at_index(uint32_t index, uint8_t* new_data) {
    memcpy(new_data, cache_array[index].data, block_size_byte);
  }

  uint32_t get_address(uint32_t address){
    uint32_t index = get_index(address);
    uint32_t tag   = cache_array[index].tag;
    uint32_t new_address = (tag << (nbits_index+nbits_blocks)) | (index<<nbits_blocks); //<<2 as words
    return new_address;
  }

  uint32_t get_address_at_index(uint32_t index){
    uint32_t tag   = cache_array[index].tag;
    uint32_t new_address = tag << (nbits_index+nbits_blocks) | (index<<nbits_blocks); //<<2 as words
    return new_address;
  }

  int32_t get_word(uint32_t address) {
    int32_t data_word = 0;
    uint32_t block_offset = this->get_block_offset(address);
    uint8_t* new_data = new uint8_t[block_size_byte];
    this->get_data(address, new_data);
    data_word = *((int32_t *)&new_data[block_offset]);
    delete new_data;
    return data_word;
  }

  void set_word(uint32_t address, int32_t data_word) {
    uint32_t block_offset = this->get_block_offset(address);
    uint8_t* new_data = new uint8_t[block_size_byte];
    this->get_data(address, new_data);
    *((int32_t *)&new_data[block_offset]) = data_word;
    for(int i=0;i<block_size_byte;i++)
    this->add_entry(address, new_data);
    delete new_data;
  }

  bool is_entry_valid(uint32_t address) {
    uint32_t index = get_index(address);
    return cache_array[index].valid;
  }

  bool is_entry_valid_at_index(uint32_t index) {
    return cache_array[index].valid;
  }

  void print_cache_status(uint32_t operation_id, std::string time_str) {
    if (cacheFile.is_open()) {
      std::string log_cache = "";
      std::ostringstream ss;

      log_cache+= std::to_string(operation_id) + "):  " + time_str + "\n";
      log_cache+= "INDEX | TAG | DATA BLOCK | VALID\n";

      for(int i=0;i<number_of_blocks;i++) {
        ss << "0x" << std::setw(this->nbits_index/4) << std::setfill('0') << std::hex << static_cast<uint32_t>(i);
        log_cache+= ss.str() + " | ";
        ss.str("");
        ss.clear();
        ss << "0x" << std::setw(this->nbits_tags/4) << std::setfill('0') << std::hex << cache_array[i].tag;
        log_cache+= ss.str() + " | 0x";
        ss.str("");
        ss.clear();
        for(int j = 0; j<block_size_byte; j++)
          ss << ":" << std::setw(2) << std::setfill('0') << std::hex << static_cast<uint16_t>(cache_array[i].data[j]);
        log_cache+= ss.str() + " | ";
        log_cache+= std::string( cache_array[i].valid ? "1" : "0" ) + "\n";

        cacheFile << log_cache;
        ss.str("");
        ss.clear();
        log_cache = std::string("");
      }
    } else {
      std::cout << "Failed to create the Cache file." << std::endl;
    }
  }

  /* main memory address
    0x7052 = 'b111_0000_0101_0010'

    cache size = 4KB,
    number_of_blocks = 256, thus index is on 8bit
    block_size_in_byte = 4KB/256 = 16bytes, i.e. 4 words

    111:       tag
    0000_0101: used as index
    0010:      used for block offset , 4bits as 16 bytes


      get_tag(0x7052) --> 0x7
      get_index(0x7052) --> 0x5
      get_block_offset(0x7052) --> 0x2


  */
};

#endif
