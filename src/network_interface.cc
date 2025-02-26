#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  uint32_t ip_32 = next_hop.ipv4_numeric(); // int 32 version of address 
  if (cache.find(ip_32) != cache.end()) { // ip add is in cache
    pair<EthernetAddress, size_t>& entry = cache[ip_32];
    EthernetAddress eth_addr = entry.first;
    size_t time = entry.second;
    EthernetFrame sent_frame;
    sent_frame.header.dst = eth_addr;
    sent_frame.header.src = ethernet_address_;
    sent_frame.header.type = EthernetHeader::TYPE_IPv4;
    sent_frame.payload = serialize(dgram); // serializing datagram 
    transmit(sent_frame);
    return;
  }
  // if ip_32 is not in cache, add it to queue
  datagram_q.push_back({ip_32, dgram});

  // have to send ARP request, but first check if one was sent in last 5 seconds

  if(arp_timestamps.find(ip_32) != arp_timestamps.end() || arp_timestamps[ip_32] >= 5000){ // address has never been requested yet or it has been longer than 5 seconds

    ARPMessage arp_request;
    arp_request.opcode = ARPMessage::OPCODE_REQUEST;
    arp_request.sender_ethernet_address = ethernet_address_;
    arp_request.sender_ip_address = ip_address_.ipv4_numeric();
    arp_request.target_ethernet_address = {};
    arp_request.target_ip_address = ip_32;

    EthernetFrame frame; // put the ARP request in the ethernet frame
    frame.header.dst = ETHERNET_BROADCAST;
    frame.header.src = ethernet_address_;
    frame.header.type = EthernetHeader::TYPE_ARP;
    frame.payload = serialize(arp_request);

    transmit(frame);

    arp_timestamps[ip_32] = 0; // reset timer to 0 
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( EthernetFrame frame )
{
  // first need to check if frame is for this interface
  if (frame.header.dst != ETHERNET_BROADCAST && frame.header.dst != ethernet_address_ ) {
        return;
  }
  // if inbound frame is IPV4
  if(frame.header.type == EthernetHeader::TYPE_IPv4){
    InternetDatagram dgram;
    if(parse(dgram , frame.payload)) { // prase payload
      datagrams_received_.push(dgram);
    }
  }

  // if inbound frame is ARP
  else if(frame.header.type == EthernetHeader::TYPE_ARP){
    ARPMessage arp_msg;
    if (!parse( arp_msg, frame.payload)){ // parse paylaod
      return;
    }
    //remember the mapping between the sender’s IP address and Ethernet address for 30 seconds
    uint32_t sender_ip_address = arp_msg.sender_ip_address;
    EthernetAddress sender_ethernet_address = arp_msg.sender_ethernet_address;
    cache[sender_ip_address] = {sender_ethernet_address, 30000}; // set 30 seconds for time 

    // check if any datagrams from queue can be sent using new mapping
    auto it = datagram_q.begin();
    while (it != datagram_q.end()){
      if(it->first == sender_ip_address){
        InternetDatagram dgram = it->second;
        // Package datagram into ethernet frame
        EthernetFrame send_frame;
        send_frame.header.dst = sender_ethernet_address;
        send_frame.header.src = ethernet_address_;
        send_frame.header.type = EthernetHeader::TYPE_IPv4;
        send_frame.payload = serialize(dgram);

        // Send Ethernet frame
        transmit(send_frame);

        it = datagram_q.erase(it);
      }
      else{
        it++; // have to manually increment itertor if nothing has been removed
      }
    }
    // send reply immediately if its a APR request
    if(arp_msg.opcode == ARPMessage::OPCODE_REQUEST && arp_msg.target_ip_address == ip_address_.ipv4_numeric()){
      ARPMessage arp_reply;
      arp_reply.sender_ip_address = ip_address_.ipv4_numeric();
      arp_reply.sender_ethernet_address = ethernet_address_;
      arp_reply.target_ethernet_address = sender_ethernet_address;
      arp_reply.target_ip_address = sender_ip_address;
      arp_reply.opcode = ARPMessage::OPCODE_REPLY;

      // put into frame
      EthernetFrame reply_frame;
      reply_frame.header.dst = sender_ethernet_address;
      reply_frame.header.src = ethernet_address_;
      reply_frame.header.type = EthernetHeader::TYPE_ARP;
      reply_frame.payload = serialize(arp_reply);

      transmit(reply_frame);
    }
  }

}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // update times for cache elements
  for (auto entry_it = cache.begin(); entry_it != cache.end();){
    entry_it->second.second += ms_since_last_tick;
    if(entry_it->second.second >= 30000){ // if time has been for greater than 30 seconds, forget mapping
      entry_it = cache.erase(entry_it);
    }
    else{
      entry_it++;
    }
  }

  // update ARP timestamp
  for (auto& entry : arp_timestamps) {
    entry.second += ms_since_last_tick;
  }
}
