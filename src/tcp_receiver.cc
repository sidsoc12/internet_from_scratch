#include "tcp_receiver.hh"
#include "debug.hh"
#include <iostream>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  if(message.RST){ // handle RST 
    rst = true;
    return;
  }
  if(!received_SYN && message.SYN){ // Check if there is a start sequence 
    ISN = message.seqno;
    received_SYN = true;
  }
  if(!received_SYN){ // If there is no start sequence, we cannot do anything
    return; 
  }
  if(message.SYN == false && message.payload.empty() && !message.FIN){
    return;
  }
  // Steps for pushing data to reassembler 
  uint64_t absolute_seqn = message.seqno.unwrap(*ISN, checkpoint);
  uint64_t reassembler_index = absolute_seqn - (message.SYN ? 0 : 1); // getting rid of SYN index

  // insert into reassembler 
  reassembler_.insert(reassembler_index, message.payload, message.FIN);
  checkpoint = reassembler_index + message.payload.size();
  // Update FIN flag 
  if(message.FIN){
    received_FIN = true;
  }
  ackno = *ISN + checkpoint + (received_FIN ? 1 : 0);
  window_size = std::min(static_cast<uint16_t>(reassembler_.writer().available_capacity()), UINT16_MAX);
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  
  return {};
}
