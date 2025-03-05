#include "router.hh"
#include "debug.hh"
#include <algorithm> // Add this line

#include <iostream>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  routes.push_back({route_prefix, prefix_length, next_hop, interface_num});
  // sort the routes based on prefix_length
  std::sort(routes.begin(), routes.end(), [](const Route &a, const Route &b) { // sort based on higher prefix length
        return a.prefix_length > b.prefix_length;  
  });
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  // loop through the router's interfaces 
  for (auto &intr: interfaces_){
    std::queue<InternetDatagram> queue = intr->datagrams_received();
    while(!queue.empty()){ // go through all the datagrams in the queue
      InternetDatagram datagram = queue.front();
      queue.pop();
      if(datagram.header.ttl == 0 || datagram.header.ttl - 1 == 0){
        
      }

    }
  }
}
