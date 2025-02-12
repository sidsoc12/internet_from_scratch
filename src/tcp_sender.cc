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
  if(!syn_sent_){ // first message 
    TCPSenderMessage init_msg;
    init_msg.SYN = true;
    init_msg.seqno = isn_;
    syn_sent_ = true;
    transmit(init_msg);
    next_seqno += 1;
    bytes_unack += 1;
    outstanding_segments.push_back(init_msg);
  }
  
  uint64_t available_window_size = (window_size > bytes_unack) ? (window_size - bytes_unack) : 0; // how many bytes can we push to receiver
  // Take all the packets of max MAX_PAYLOAD_SIZE out of buffer that we can
  bool fin_set = false;
  while( available_window_size > 0 && window_size != 0){
    uint64_t payload_size = std::min({available_window_size, TCPConfig::MAX_PAYLOAD_SIZE, reader().bytes_buffered()});
    std::string data = std::string(reader().peek().substr(0, payload_size));
    reader().pop(payload_size);
    TCPSenderMessage msg;
    msg.seqno = isn_.wrap(next_seqno, isn_);
    msg.payload = data;

    if(reader().is_finished() && available_window_size > payload_size && outstanding_segments.empty()){
      msg.FIN = true;
      fin_set = true;
      //should next_seqno be reset here?
    }
    transmit(msg);
    outstanding_segments.push_back(msg);
    uint64_t seq_len = msg.sequence_length();
    available_window_size -= seq_len;
    next_seqno += seq_len;
    bytes_unack += seq_len;

    if(msg.FIN){
      break;
    }
  }
  //handle case where FIN couldnt fit but we still need to send it
  if (!fin_set && reader().is_finished() && outstanding_segments.empty()) {
        TCPSenderMessage fin_msg;
        fin_msg.seqno = isn_.wrap(next_seqno, isn_);
        fin_msg.FIN = true;
        transmit(fin_msg);
        outstanding_segments.push_back(fin_msg);
        next_seqno += 1;
        bytes_unack += 1;
  }
  if(window_size == 0){ // act as if window_size == 1
    TCPSenderMessage probe;
    probe.seqno = isn_.wrap(next_seqno , isn_);
    probe.payload = "?"; // dummy 1 byte payload
    transmit(probe);
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage empty_msg;
  empty_msg.seqno = isn_.wrap(next_seqno, isn_);
  return empty_msg; 
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
