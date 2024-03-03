#ifndef CACHE_H
#define CACHE_H

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;


#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>


// Target module representing a simple direct mapped cache
class CacheMemory
{

public:
  uint32_t cache_size_byte  = 4*1024;
  uint32_t number_of_blocks = 256;
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
  }

  void create_cache(uint32_t cache_size_byte) {
      this->cache_size_byte = cache_size_byte;
      cache_array = new cache_line_t[number_of_blocks];
  }

  uint32_t initialize_cache() {
      if(cache_array == NULL) {
        return -1;
      }
      uint32_t block_size_byte = get_block_size();
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
    uint32_t block_size_byte = this->get_block_size();
    return (uint32_t)((address % number_of_blocks) / block_size_byte );
  }

  uint32_t get_block_offset(uint32_t address) {
    uint32_t block_size_byte = this->get_block_size();
    return (uint32_t)(address % block_size_byte);
  }

  uint32_t get_tag(uint32_t address) {
    uint32_t block_size_byte = this->get_block_size();
    return (uint32_t)(address / cache_size_byte);
  }

  bool cache_hit(uint32_t address) {
    uint32_t index = get_index(address);
    uint32_t tag   = get_tag(address);
    return ( cache_array[index].valid && tag == cache_array[index].tag);
  }

  void add_entry(uint32_t address, uint8_t* new_data) {
    uint32_t index = get_index(address);
    uint32_t tag   = get_tag(address);
    uint32_t block_size_byte = this->get_block_size();
    cache_array[index].valid = true;
    cache_array[index].tag   = tag;
    memcpy(cache_array[index].data, new_data, block_size_byte);
  }

  void get_data(uint32_t address, uint8_t* new_data) {
    uint32_t index = get_index(address);
    uint32_t block_size_byte = this->get_block_size();
    memcpy(new_data, cache_array[index].data, block_size_byte);
  }

  int32_t get_word(uint32_t address) {
    int32_t data_word = 0;
    uint32_t block_size_byte = this->get_block_size();
    uint32_t block_offset = this->get_block_offset(address);
    uint8_t* new_data = new uint8_t[block_size_byte];
    this->get_data(address, new_data);
    data_word = *((int32_t *)new_data[block_offset]);
    delete new_data;
    return data_word;
  }

  void set_word(uint32_t address, int32_t data_word) {
    uint32_t block_size_byte = this->get_block_size();
    uint32_t block_offset = this->get_block_offset(address);
    uint8_t* new_data = new uint8_t[block_size_byte];
    this->get_data(address, new_data);
    *((int32_t *)new_data[block_offset]) = data_word;
    this->add_entry(address, new_data);
    delete new_data;
  }

  bool is_entry_valid(uint32_t address) {
    uint32_t index = get_index(address);
    uint32_t tag   = get_tag(address);
    return cache_array[index].valid;
  }

  void print_cache_status(uint32_t operation_id) {
    if (cacheFile.is_open()) {
      std::string log_cache = "";
      std::ostringstream ss;

      log_cache+= std::to_string(operation_id) + "\n";
      log_cache+= "INDEX | TAG | DATA BLOCK | VALID\n";

      uint32_t block_size_byte = get_block_size();

      for(int i=0;i<number_of_blocks;i++) {
        ss << "0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<uint32_t>(i);
        log_cache+= ss.str() + " | ";
        ss.str("");
        ss.clear();
        ss << "0x" << std::hex << cache_array[i].tag;
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
