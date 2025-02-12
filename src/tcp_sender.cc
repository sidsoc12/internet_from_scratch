#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"

using namespace std;

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  debug( "unimplemented sequence_numbers_in_flight() called" );
  return {};
}

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::consecutive_retransmissions() const
{
  debug( "unimplemented consecutive_retransmissions() called" );
  return {};
}

void TCPSender::push( const TransmitFunction& transmit )
{
  if(next_seqno == 0){ // first message 
    TCPSenderMessage msg;
    msg.SYN = true;
    msg.seqno = isn_;
    transmit(msg);
    next_seqno += 1;
    bytes_unack += 1;
    outstanding_segments.push_back(msg);
  }
  
  uint64_t available_window_size = window_size - bytes_unack; // how many bytes can we push to receiver
  if(window_size == 0){ // handle window size = 0 case 
    available_window_size = 1;
  }

  // Take all the packets of max MAX_PAYLOAD_SIZE out of buffer that we can
  while( available_window_size > 0){
    std::string payload = reader().read(std::min(available_window_size, static_cast<uint64_t>(TCPConfig::MAX_PAYLOAD_SIZE)));
  }




}

TCPSenderMessage TCPSender::make_empty_message() const
{
  debug( "unimplemented make_empty_message() called" );
  return {};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  debug( "unimplemented receive() called" );
  (void)msg;
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  debug( "unimplemented tick({}, ...) called", ms_since_last_tick );
  (void)transmit;
}
