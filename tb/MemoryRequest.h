#ifndef MEMORYREQUEST_H
#define MEMORYREQUEST_H

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"


// MemoryRequest module generating generic payload transactions

SC_MODULE(MemoryRequest)
{
  // TLM-2 socket, defaults to 32-bits wide, base protocol
  tlm_utils::simple_initiator_socket<MemoryRequest> socket;
  bool                                          we_i;
  uint32_t                                      be_i;
  uint32_t                                      addr_i;
  uint32_t                                      rwdata_io;

  SC_CTOR(MemoryRequest)
  : socket("socket")  // Construct and name socket
  {
    SC_THREAD(thread_process);
  }

  void thread_process()
  {
    // TLM-2 generic payload transaction, reused across calls to b_transport
    tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
    sc_time delay_gnt = sc_time(1000, SC_NS);
    sc_time delay_rvalid = sc_time(1000, SC_NS);
    sc_time delay = sc_time(10, SC_NS);
    tlm::tlm_command cmd;

    while(true) {

      wait(obi_new_req);

      std::cout << "X-HEEP tlm_generic_payload REQ: { " << (we_i ? 'W' : 'R') << ", @0x" << hex << addr_i
                << " , DATA = 0x" << hex << rwdata_io << " BE = " << hex << be_i <<", at time " << sc_time_stamp() << " }" << std::endl;

      //wait some time before giving the gnt
      wait(delay_gnt);

      obi_new_gnt.notify();

      cmd = we_i ? tlm::TLM_WRITE_COMMAND : tlm::TLM_READ_COMMAND;

      trans->set_command( cmd );
      trans->set_address( addr_i & 0x000003FF );
      trans->set_data_ptr( reinterpret_cast<unsigned char*>(&rwdata_io) );
      trans->set_data_length( 4 );
      trans->set_streaming_width( 4 ); // = data_length to indicate no streaming
      trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
      trans->set_dmi_allowed( false ); // Mandatory initial value
      trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value

      socket->b_transport( *trans, delay );  // Blocking transport call

      // Initiator obliged to check response status and delay
      if ( trans->is_response_error() )
        SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

      //wait some time before giving the rvalid
      wait(delay_rvalid);

      std::cout << "X-HEEP tlm_generic_payload RESP: { DATA = 0x" << hex << rwdata_io <<", at time " << sc_time_stamp() << " }" << std::endl;

      obi_new_rvalid.notify();

    }
  }
};

#endif
