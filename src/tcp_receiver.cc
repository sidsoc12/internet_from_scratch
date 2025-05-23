#include "tcp_receiver.hh"
#include "debug.hh"
#include <iostream>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  

  if ( message.RST ) { // handle RST
    rst = true;
    reader().set_error(); // set error
    return;
  }

  if (reassembler_.writer().is_closed()) {
    if (message.FIN || !message.payload.empty()) {
      std::cerr << "stream already closed — ignoring segment at seq "
                << message.seqno.unwrap(*ISN, checkpoint) << std::endl;
    }
    return;
  }
 
  if ( !received_SYN && message.SYN ) { // Check if there is a start sequence
    ISN = message.seqno;
    received_SYN = true;
    ackno = *ISN + 1;
  } else if ( received_SYN && message.SYN ) { // Ignoring duplicate SYN's
    return;
  }
  if ( !received_SYN ) { // If there is no start sequence, we cannot do anything
    return;
  }
  if ( message.SYN == false && message.payload.empty() && !message.FIN ) {
    return;
  }
  // No data to push yet => just update ISN and return
  if ( message.SYN && message.payload.empty() && !message.FIN ) {
    return;
  }
  // Steps for pushing data to reassembler
  uint64_t absolute_seqn = message.seqno.unwrap( *ISN, checkpoint );
  uint64_t reassembler_index = ( message.SYN ? 0 : absolute_seqn - 1 );
  // insert into reassembler
  // Debugging prints
  if (message.FIN) {
    std::cerr << "[receiver] Received FIN at absolute seq = " << absolute_seqn << std::endl;
  }
  std::cout << "Absolute Seq: " << absolute_seqn << ", Reassembler Index: " << reassembler_index
            << ", FIN: " << message.FIN << ", Payload Size: " << message.payload.size() << std::endl;
  reassembler_.insert( reassembler_index, message.payload, message.FIN );
  checkpoint = reassembler_index + message.payload.size();
  // FIN without payload should NOT increment checkpoint
  // if ( message.FIN && message.payload.empty() && !message.SYN ) {
  //   checkpoint -= 1;
  // }
  ackno = *ISN + reassembler_.writer().bytes_pushed() + 1;
  if ( reassembler_.writer().is_closed() ) {
    ackno = Wrap32( *ackno + 1 ); // FIN should only increase ackno when it can be assembled
  }
  // have to add 1 to add back the SYN byte
}

TCPReceiverMessage TCPReceiver::send() const
{
  uint16_t current_window_size = static_cast<uint16_t>(
    std::min( reassembler_.writer().available_capacity(), static_cast<size_t>( UINT16_MAX ) ) );
  return { !ackno ? std::nullopt : ackno, current_window_size, rst || reassembler_.writer().has_error() };
}
