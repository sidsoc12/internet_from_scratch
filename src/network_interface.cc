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
    if( parse(dgram , frame.payload) == ParseResult::NoError ) { // prase payload
      datagrams_received_.push(dgram);
    }
  }
  // if inbound frame is ARP

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
  for (auto entry_it = arp_timestamps.begin(); entry_it != arp_timestamps.end();){
    entry_it->second += ms_since_last_tick;
  }
  
}
