#ifndef MEMORYREQUEST_H
#define MEMORYREQUEST_H

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"

#include "Cache.h"

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

  typedef struct cache_statistics
  {
    uint32_t number_of_transactions;
    uint32_t number_of_hit;
    uint32_t number_of_miss;
  } cache_statistics_t;

  cache_statistics_t cache_stat;

  SC_CTOR(MemoryRequest)
  : socket("socket")  // Construct and name socket
  {

    cache = new CacheMemory;
    cache->create_cache();
    cache->initialize_cache();
    cache_stat.number_of_transactions = 0;
    cache_stat.number_of_hit = 0;
    cache_stat.number_of_miss = 0;
    cache->print_cache_status(cache_stat.number_of_transactions++);

    SC_THREAD(thread_process);
  }




  void thread_process()
  {
    // TLM-2 generic payload transaction, reused across calls to b_transport
    tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
    sc_time delay_gnt = sc_time(1000, SC_NS);
    sc_time delay_rvalid = sc_time(1000, SC_NS);
    sc_time delay = sc_time(10, SC_NS);

    uint32_t cache_block_size_byte = cache->get_block_size();
    uint32_t cache_block_size_word = cache->get_block_size()/4;
    uint8_t*  cache_data = new uint8_t[cache_block_size_byte];
    int32_t* main_mem_data = new int32_t[cache_block_size_word];

    while(true) {

      wait(obi_new_req);

      std::cout << "X-HEEP tlm_generic_payload REQ: { " << (we_i ? 'W' : 'R') << ", @0x" << hex << addr_i
                << " , DATA = 0x" << hex << rwdata_io << " BE = " << hex << be_i <<", at time " << sc_time_stamp() << " }" << std::endl;

      if(be_i!=0xF) {
        SC_REPORT_ERROR("OBI External Memory SystemC", "ByteEnable different than 0xF is not supported");
      }

      // we use the cache only to read
      if(cache->cache_hit(addr_i)){

        cache_stat.number_of_hit++;
        obi_new_gnt.notify();
        main_mem_data[0] = cache->get_word(addr_i);
        //if Write, writes to cache
        if(we_i)
          cache->set_word(addr_i, rwdata_io);
        else
          rwdata_io = main_mem_data[0];
        //wait some time before giving the rvalid
        wait(delay_rvalid);
      }

      else { //miss case

        cache_stat.number_of_miss++;

        //wait some time before giving the gnt as we have a miss
        wait(delay_gnt);
        obi_new_gnt.notify();
        uint32_t addr_to_read = cache->get_base_address(addr_i);

        //first read block_size bytes from memory to place them in cache regardless of the cmd
        for(int i=0; i < cache_block_size_word; i++){
          trans->set_command( tlm::TLM_READ_COMMAND );
          trans->set_address( (addr_to_read + i*4) & 0x00007FFF ); //15bits
          trans->set_data_ptr( reinterpret_cast<unsigned char*>(&main_mem_data[i]) );
          trans->set_data_length( 4 );
          trans->set_streaming_width( 4 ); // = data_length to indicate no streaming
          trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
          trans->set_dmi_allowed( false ); // Mandatory initial value
          trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value
          socket->b_transport( *trans, delay );  // Blocking transport call

          printf("Reading from Mem[%x] %x\n", (addr_to_read + i*4), main_mem_data[i]);

          // Initiator obliged to check response status and delay
          if ( trans->is_response_error() )
            SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
        }

        //always write back what will be replace if valid as we do not have dirty bits for simplicity
        if (cache->is_entry_valid(addr_i)) {
          //if we are going to replace a valid entry
          cache->get_data(addr_i, cache_data);

          //write back
          for(int i=0; i < cache_block_size_word; i++){
            trans->set_command( tlm::TLM_WRITE_COMMAND );
            trans->set_address( (addr_to_read + i*4) & 0x00007FFF ); //15bits
            trans->set_data_ptr( reinterpret_cast<unsigned char*>(&cache_data[i]) );
            trans->set_data_length( 4 );
            trans->set_streaming_width( 4 ); // = data_length to indicate no streaming
            trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
            trans->set_dmi_allowed( false ); // Mandatory initial value
            trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value
            socket->b_transport( *trans, delay );  // Blocking transport call

            printf("Writing back to Mem[%x] %x\n", (addr_to_read + i*4), cache_data[i]);
            // Initiator obliged to check response status and delay
            if ( trans->is_response_error() )
              SC_REPORT_ERROR("TLM-2", "Response error from b_transport");
          }
        }

        //now replace the entry in cache
        cache->add_entry(addr_i, (uint8_t*)main_mem_data);

        //if Write, writes to cache
        if(we_i)
          cache->set_word(addr_i, rwdata_io);

        //now give back the rdata
        rwdata_io = main_mem_data[0];


        //wait some time before giving the rvalid
        wait(delay_rvalid);
      } //miss case

      std::cout << "X-HEEP tlm_generic_payload RESP: { DATA = 0x" << hex << rwdata_io <<", at time " << sc_time_stamp() << " }" << std::endl;
      cache->print_cache_status(cache_stat.number_of_transactions++);

      obi_new_rvalid.notify();

    }
  }
};

#endif
