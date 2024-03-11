#ifndef MEMORYREQUEST_H
#define MEMORYREQUEST_H

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"

#include "Cache.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

// MemoryRequest module generating generic payload transactions

SC_MODULE(MemoryRequest)
{
  // TLM-2 socket, defaults to 32-bits wide, base protocol
  tlm_utils::simple_initiator_socket<MemoryRequest> socket;
  bool                                          we_i;
  uint32_t                                      be_i;
  uint32_t                                      addr_i;
  uint32_t                                      rwdata_io;
  CacheMemory*                                  cache;
  std::ofstream                                 heep_mem_transactions;
  bool                                          bypass_state = false;

  typedef struct cache_statistics
  {
    uint32_t number_of_transactions;
    uint32_t number_of_hit;
    uint32_t number_of_miss;
  } cache_statistics_t;

  cache_statistics_t cache_stat;

  SC_CTOR(MemoryRequest)
  : socket("socket"),  // Construct and name socket
    heep_mem_transactions("heep_mem_transactions.log")
  {

    cache = new CacheMemory;
    cache->create_cache();
    cache->initialize_cache();
    cache_stat.number_of_transactions = 0;
    cache_stat.number_of_hit = 0;
    cache_stat.number_of_miss = 0;
    cache->print_cache_status(cache_stat.number_of_transactions++, sc_time_stamp().to_string());

    SC_THREAD(thread_process);
  }


  uint32_t memory_copy(uint32_t addr, int32_t* buffer_data, int N, bool write_enable, tlm::tlm_generic_payload* trans, sc_time delay) {

    tlm::tlm_command cmd = write_enable ? tlm::TLM_WRITE_COMMAND : tlm::TLM_READ_COMMAND;

    //first read block_size bytes from memory to place them in cache regardless of the cmd
    for(int i=0; i < N; i++){
      trans->set_command( cmd );
      trans->set_address( (addr + i*4) & 0x00007FFF ); //15bits
      trans->set_data_ptr( reinterpret_cast<unsigned char*>(&buffer_data[i]) );
      trans->set_data_length( 4 );
      trans->set_streaming_width( 4 ); // = data_length to indicate no streaming
      trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
      trans->set_dmi_allowed( false ); // Mandatory initial value
      trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value
      socket->b_transport( *trans, delay );  // Blocking transport call

      if(bypass_state){
        if(write_enable)
          heep_mem_transactions << "Writing to Mem[" << hex << ((addr + i*4) & 0x00007FFF) << "]: " << buffer_data[i] << " at time " << sc_time_stamp() <<std::endl;
        else
          heep_mem_transactions << "Reading from Mem[" << hex << ((addr + i*4) & 0x00007FFF) << "]: " << buffer_data[i] << " at time " << sc_time_stamp() <<std::endl;
      } else {
        if(write_enable)
          heep_mem_transactions << "Cache Writing to Mem[" << hex << ((addr + i*4) & 0x00007FFF) << "]: " << buffer_data[i] << " at time " << sc_time_stamp() <<std::endl;
        else
          heep_mem_transactions << "Cache Reading from Mem[" << hex << ((addr + i*4) & 0x00007FFF) << "]: " << buffer_data[i] << " at time " << sc_time_stamp() <<std::endl;
      }
      // Initiator obliged to check response status and delay
      if ( trans->is_response_error() )
        SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
    }
    return N;
  }


  void thread_process()
  {
    // TLM-2 generic payload transaction, reused across calls to b_transport
    tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;

    sc_time delay_gnt_miss = sc_time(100, SC_NS);
    sc_time delay_rvalid_miss = sc_time(100, SC_NS);

    sc_time delay_rvalid_hit = sc_time(20, SC_NS); //as of today, it must be >=20

    sc_time delay = sc_time(1, SC_NS);

    uint32_t cache_block_size_byte = cache->get_block_size();
    uint32_t cache_block_size_word = cache->get_block_size()/4;
    uint8_t* cache_data = new uint8_t[cache_block_size_byte];
    int32_t* main_mem_data = new int32_t[cache_block_size_word];
    uint32_t address_to_replace;
    uint32_t cache_flushed;

    while(true) {

      wait(obi_new_req);

      heep_mem_transactions << "X-HEEP tlm_generic_payload REQ: { " << (we_i ? 'W' : 'R') << ", @0x" << hex << addr_i
                << " , DATA = 0x" << hex << rwdata_io << " BE = " << hex << be_i <<", at time " << sc_time_stamp() << " }" << std::endl;

      if(be_i!=0xF) {
        SC_REPORT_ERROR("OBI External Memory SystemC", "ByteEnable different than 0xF is not supported");
      }

      //if we are writing 1 or 2 to last address, flush cache or bypass
      if(we_i && ((addr_i & 0x00007FFF) == 0x7FFC)){

        if(rwdata_io == 1){
          //FLUSH Cache
          heep_mem_transactions << "X-HEEP Flush Cache, at time " << sc_time_stamp() << " }" << std::endl;
          uint32_t cache_number_of_blocks = cache->number_of_blocks;
          heep_mem_transactions<<"Cache Flushing at time "<<sc_time_stamp()<<std::endl;
          cache_flushed=0;
          for(int i=0;i<cache_number_of_blocks;i++){
              if (cache->is_entry_valid_at_index(i)) {
                cache_flushed++;
                //if we are going to replace a valid entry
                cache->get_data_at_index(i, cache_data);
                address_to_replace = cache->get_address_at_index(i);
                //write back
                memory_copy(address_to_replace, (uint32_t *)cache_data, cache_block_size_word, true, trans, delay);
            }
          }
          heep_mem_transactions<<"Cache Flushed "<< dec << cache_flushed << " entries"<<std::endl;
        } else if (rwdata_io == 2){
          //ByPass Flash from next transaction
          bypass_state = true;
          heep_mem_transactions<<"Cache ByPass set at time "<<sc_time_stamp()<<std::endl;
          heep_mem_transactions << "X-HEEP Bypass Cache, at time " << sc_time_stamp() << " }" << std::endl;
        }
        obi_new_gnt.notify();
        wait(delay_rvalid_miss);
      }

      else{

        if (bypass_state) {
          heep_mem_transactions << "Cache in bypass state at time " << sc_time_stamp() <<std::endl;
          wait(delay_gnt_miss);
          obi_new_gnt.notify();
          memory_copy(addr_i, &rwdata_io, 1, we_i == true, trans, delay);
          wait(delay_rvalid_miss);
        } else {
          // we use the cache only to read
          if(cache->cache_hit(addr_i)){

            heep_mem_transactions << "Cache HIT on address " << hex << addr_i << " at time " << sc_time_stamp() <<std::endl;

            cache_stat.number_of_hit++;

            obi_new_gnt.notify();
            main_mem_data[0] = cache->get_word(addr_i);
            //if Write, writes to cache
            if(we_i)
              cache->set_word(addr_i, rwdata_io);
            else
              rwdata_io = main_mem_data[0];
            wait(delay_rvalid_hit);
          }

          else { //miss case

            cache_stat.number_of_miss++;

            heep_mem_transactions << "Cache MISS on address " << hex << addr_i << " at time " << sc_time_stamp() <<std::endl;

            //wait some time before giving the gnt as we have a miss
            wait(delay_gnt_miss);
            obi_new_gnt.notify();

            uint32_t addr_to_read = cache->get_base_address(addr_i);
            uint32_t addr_offset  = cache->get_block_offset(addr_i);

            //first read block_size bytes from memory to place them in cache regardless of the cmd
            memory_copy(addr_to_read, main_mem_data, cache_block_size_word, false, trans, delay);
            uint32_t index_to_add = cache->get_index(addr_i);
            uint32_t tag_to_add       = cache->get_tag(addr_i);

            heep_mem_transactions << "Adding to Cache TAG " << hex << tag_to_add << " and index " << hex << index_to_add <<std::endl;

            //always write back what will be replace if valid as we do not have dirty bits for simplicity
            if (cache->is_entry_valid(addr_i)) {
              //if we are going to replace a valid entry
              cache->get_data(addr_i, cache_data);
              address_to_replace = cache->get_address(addr_i);
              uint32_t index_to_replace = cache->get_index(addr_i);
              uint32_t tag_to_replace = cache->get_tag_from_index(index_to_replace);

              heep_mem_transactions << "Cache Replace address " << hex << addr_i << " with address " << hex << address_to_replace << " due to the MISS at time " << sc_time_stamp() <<std::endl;
              heep_mem_transactions << "Index to replace " << hex << index_to_replace << " Tag to replace " << tag_to_replace <<std::endl;

              //write back
              memory_copy(address_to_replace, (uint32_t *)cache_data, cache_block_size_word, true, trans, delay);
            }

            //now replace the entry in cache
            cache->add_entry(addr_i, (uint8_t*)main_mem_data);

            //if Write, writes to cache
            if(we_i)
              cache->set_word(addr_i, rwdata_io);

            //now give back the rdata
            rwdata_io = main_mem_data[addr_offset>>2]; //>>2 as addr_offset is for byte address, not words

            //wait some time before giving the rvalid
            wait(delay_rvalid_miss);

          }
        }
      }

      heep_mem_transactions << "X-HEEP tlm_generic_payload RESP: { DATA = 0x" << hex << rwdata_io <<", at time " << sc_time_stamp() << " }" << std::endl;
      cache->print_cache_status(cache_stat.number_of_transactions++, sc_time_stamp().to_string());

      obi_new_rvalid.notify();

    }
  }
};

#endif
