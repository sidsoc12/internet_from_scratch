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
  routes.push_back( { route_prefix, prefix_length, next_hop, interface_num } );
  // sort the routes based on prefix_length
  std::sort(
    routes.begin(), routes.end(), []( const Route& a, const Route& b ) { // sort based on higher prefix length
      return a.prefix_length > b.prefix_length;
    } );
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  if ( routes.empty() ) { // check if routing table is empty
    return;
  }
  // loop through the router's interfaces
  for ( auto& intr : interfaces_ ) {
    std::queue<InternetDatagram>& queue = intr->datagrams_received();
    while ( !queue.empty() ) { // go through all the datagrams in the queue
      InternetDatagram datagram = queue.front();
      queue.pop();
      if ( datagram.header.ttl <= 1 ) { // check ttl
        continue;
      }
      datagram.header.ttl -= 1;
      datagram.header.compute_checksum();
      const Route* match = nullptr;

      // Loop through all potential routes and find longest prefix match
      for ( const auto& route : routes ) {
        bool has_route_match = true;
        if ( route.prefix_length == 0 ) { // prevent >> 32
          has_route_match = true;
        } else {
          for ( int i = 31; i >= 32 - route.prefix_length; i-- ) {
            // Bit by Bit comparison until prefix_length
            if ( ( ( route.prefix >> i ) & 1 ) != ( ( datagram.header.dst >> i ) & 1 ) ) {
              has_route_match = false;
              break;
            }
          }
        }
        if ( has_route_match ) {
          match = &route;
          break; // since list is sorted, first match is best match
        }
      }
      if ( match == nullptr ) {
        continue;
      }
      if ( match->next_hop.has_value() ) {
        interface( match->interface_num )->send_datagram( datagram, match->next_hop.value() );
      } else {
        interface( match->interface_num )
          ->send_datagram( datagram, Address::from_ipv4_numeric( datagram.header.dst ) );
      }
    }
  }
}
