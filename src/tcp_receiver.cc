#include "tcp_receiver.hh"
#include "debug.hh"
#include <iostream>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if(message.RST){ // handle RST 
    rst = true;
    return;
  }
  if(!received_SYN && message.SYN){ // Check if there is a start sequence 
    ISN = message.seqno;
    received_SYN = true;
  }
  else if(received_SYN && message.SYN){ // Ignoring duplicate SYN's
    return; 
  }
  if(!received_SYN){ // If there is no start sequence, we cannot do anything
    return; 
  }
  if(message.SYN == false && message.payload.empty() && !message.FIN){
    return;
  }
  // No data to push yet => just update ISN and return
  if (message.SYN && message.payload.empty() && !message.FIN) {
    return;  
  }
  // Steps for pushing data to reassembler 
  uint64_t absolute_seqn = message.seqno.unwrap(*ISN, checkpoint);
  uint64_t reassembler_index = absolute_seqn - 1; // getting rid of SYN index

  // insert into reassembler 
  reassembler_.insert(reassembler_index, message.payload, message.FIN);
  checkpoint = reassembler_index + message.payload.size() - (message.FIN ? 1 : 0);
  // Update FIN flag 
  if(message.FIN){
    received_FIN = true;
  }
  ackno = *ISN + checkpoint + (received_FIN ? 1 : 0) + 1; //have to add 1 to add back the SYN byte
  window_size = static_cast<uint16_t>(std::min(reassembler_.writer().available_capacity(), static_cast<size_t>(UINT16_MAX)));
}

TCPReceiverMessage TCPReceiver::send() const
{
  if (rst) {
        return { std::nullopt, 0, true };
  }
  if (!ackno) {
        return { std::nullopt, 0, false };
  }
  return { ackno, window_size, false };
}
