#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"

using namespace std;

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return bytes_unack;
}

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutive_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{

  if ( input_.writer().has_error() ) { // This is for managing error declared in stream.
    TCPSenderMessage rst_msg;
    rst_msg.seqno = isn_.wrap( next_seqno, isn_ );
    rst_msg.RST = true;
    transmit( rst_msg );
    return;
  }
  if ( !syn_sent_ ) { // first message
    TCPSenderMessage init_msg;
    init_msg.SYN = true;
    init_msg.seqno = isn_;
    syn_sent_ = true;

    // for reading before push
    uint64_t available_window_size = ( window_size > bytes_unack ) ? ( window_size - bytes_unack ) : 0;

    // reserve byte for SYN
    if ( available_window_size > 0 ) {
      available_window_size -= 1;
    }

    if ( available_window_size > 0 && reader().bytes_buffered() > 0 ) {
      uint64_t payload_size
        = std::min( { available_window_size, TCPConfig::MAX_PAYLOAD_SIZE, reader().bytes_buffered() } );
      std::string data = std::string( reader().peek().substr( 0, payload_size ) );
      reader().pop( payload_size );
      init_msg.payload = data;
    }
    // for SYN + FIN
    if ( reader().is_finished() && available_window_size > init_msg.sequence_length() ) {
      init_msg.FIN = true;
      fin_set = true;
    }
    transmit( init_msg );
    next_seqno += init_msg.sequence_length();
    bytes_unack += init_msg.sequence_length();
    outstanding_segments.push_back( init_msg );
    if ( !timer_active ) {
      timer_active = true;
      timer = 0;
    }
  }

  uint64_t available_window_size
    = ( window_size > bytes_unack ) ? ( window_size - bytes_unack ) : 0; // how many bytes can we push to receiver
  // Take all the packets of max MAX_PAYLOAD_SIZE out of buffer that we can
  while ( available_window_size > 0 && window_size != 0 && reader().bytes_buffered() > 0 ) {
    uint64_t payload_size
      = std::min( { available_window_size, TCPConfig::MAX_PAYLOAD_SIZE, reader().bytes_buffered() } );
    std::string data = std::string( reader().peek().substr( 0, payload_size ) );
    if ( payload_size > 0 ) {
      reader().pop( payload_size );
    }
    TCPSenderMessage msg;
    msg.seqno = isn_.wrap( next_seqno, isn_ );
    msg.payload = data;
    // Check if fin should be pushed
    if ( reader().is_finished() && available_window_size > payload_size ) {
      msg.FIN = true;
      fin_set = true;
    }
    transmit( msg );
    outstanding_segments.push_back( msg );
    uint64_t seq_len = msg.sequence_length();
    available_window_size -= seq_len;
    next_seqno += seq_len;
    bytes_unack += seq_len;

    if ( seq_len > 0 && !timer_active ) {
      timer_active = true;
      timer = 0;
    }
    if ( msg.FIN ) {
      break;
    }
  }
  // handle case where FIN couldnt fit but we still need to send it
  if ( !fin_set && reader().is_finished() && available_window_size > 0 ) {
    TCPSenderMessage fin_msg;
    fin_msg.seqno = isn_.wrap( next_seqno, isn_ );
    fin_msg.FIN = true;
    transmit( fin_msg );
    outstanding_segments.push_back( fin_msg );
    next_seqno += 1;
    bytes_unack += 1;
    fin_set = true;
    if ( !timer_active ) {
      timer_active = true;
      timer = 0;
    }
  }
  if ( fin_ready_ && !fin_set ) { // send fin flag if probe got acknowed and fin flag can be sent
    TCPSenderMessage fin_msg;
    fin_msg.seqno = isn_.wrap( next_seqno, isn_ );
    fin_msg.FIN = true;
    transmit( fin_msg );
    outstanding_segments.push_back( fin_msg );
    next_seqno += 1;
    bytes_unack += 1;
    fin_set = true;
    fin_ready_ = false;
    if ( !timer_active ) {
      timer_active = true;
      timer = 0;
    }
  }
  if ( window_size == 0 && !zero_window_probe_sent ) { // act as if window_size == 1
    uint64_t payload_size = std::min<uint64_t>( { 1, reader().bytes_buffered() } );
    if ( payload_size > 0 ) {
      std::string data = std::string( reader().peek().substr( 0, payload_size ) );
      reader().pop( payload_size );
      TCPSenderMessage msg;
      msg.seqno = isn_.wrap( next_seqno, isn_ );
      msg.payload = data;
      transmit( msg );
      outstanding_segments.push_back( msg );
      next_seqno += msg.sequence_length();
      bytes_unack += msg.sequence_length();
      zero_window_probe_sent = true;
      zero_window_probe_seqno = msg.seqno.unwrap( isn_, next_seqno );
      if ( !timer_active ) {
        timer_active = true;
        timer = 0;
      }
    }
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage empty_msg;
  empty_msg.seqno = isn_.wrap( next_seqno, isn_ );
  if ( writer().has_error() ) {
    empty_msg.RST = true;
  }
  return empty_msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{

  if ( msg.RST ) { // have to make sure to exit if there is an error
    input_.writer().set_error();
    outstanding_segments.clear();
    timer_active = false;
    return;
  }

  window_size = msg.window_size; // updating window

  if ( window_size > 0 ) { // update whether probe can be sent again
    zero_window_probe_sent = false;
  }

  if ( !msg.ackno.has_value() ) {
    return;
  }
  uint64_t ackno = msg.ackno->unwrap( isn_, next_seqno );
  if ( ackno <= next_seqno - bytes_unack )
    return;                   // this if for preventing duplicate and old Acks
  if ( ackno > next_seqno ) { // impossible ack
    return;
  }
  size_t old_size = outstanding_segments.size();
  while ( !outstanding_segments.empty() ) {
    // Go through oustanding segments from oldest to youngest
    TCPSenderMessage& oldest_unack_message = outstanding_segments.front();
    if ( ackno >= oldest_unack_message.seqno.unwrap( isn_, next_seqno ) + oldest_unack_message.sequence_length() ) {
      bytes_unack -= oldest_unack_message.sequence_length();
      // Check if probe was acknowledged, if so we have to allow another probe to be created when needed.
      if ( zero_window_probe_seqno.has_value()
           && zero_window_probe_seqno.value() == oldest_unack_message.seqno.unwrap( isn_, next_seqno ) ) {
        zero_window_probe_sent = false;
        zero_window_probe_seqno = std::nullopt;
        if ( !fin_set && reader().is_finished() && bytes_unack == 0 ) {
          fin_ready_ = true;
        }
      }
      outstanding_segments.pop_front();
    } else {
      break;
    }
  }
  // if new data was ack => reset transmission timer
  if ( old_size > outstanding_segments.size() ) {
    if ( !outstanding_segments.empty() ) {
      timer = 0; // reset timer if there are outstanding segments
      timer_active = true;
    } else {
      // stop timer if all oustanding have been acknowledged
      timer_active = false;
    }
    RTO = initial_RTO_ms_;
    consecutive_retransmissions_ = 0;
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  if ( outstanding_segments.empty() || !timer_active )
    return; // return if timer is inactive and there are no outstanding segments
  timer = timer + ms_since_last_tick;
  if ( timer >= RTO ) {
    transmit( outstanding_segments.front() ); // retransmit oldest unacknowledged segment
    if ( window_size > 0 ) {
      consecutive_retransmissions_++;
      RTO = RTO * 2;
    }
    timer = 0;
  }
}
