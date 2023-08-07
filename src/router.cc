#include "router.hh"

#include <iostream>
#include <limits>

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
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  // (void)route_prefix;
  // (void)prefix_length;
  // (void)next_hop;
  // (void)interface_num;

  route_table_.emplace_back(route_prefix, prefix_length, next_hop, interface_num);
}

void Router::route() 
{
 for (auto& interface_ : interfaces_) {
  while (interface_.is_empty() == false) 
  {
    auto dgram_recv = interface_.maybe_receive();
    if (dgram_recv.has_value()) {
      auto& dgram = dgram_recv.value();
      process_dgram(dgram);
    }
  }
 }
}

void Router::process_dgram(InternetDatagram& dgram)
{
  if (dgram.header.ttl <= 1) return;
  auto chosen_interface = route_table_.end();
  for (auto it = route_table_.begin(); it != route_table_.end(); ++it)
  {
    if (is_match(dgram.header.dst, it->route_prefix, it->prefix_length)) {
      if (chosen_interface == route_table_.end() || chosen_interface->prefix_length < it->prefix_length) {
        chosen_interface = it;
      }
    }
  }

  if (chosen_interface != route_table_.end()) 
  {
    dgram.header.ttl--;
    dgram.header.compute_checksum();
    if (chosen_interface->next_hop.has_value()) {
      interface(chosen_interface->interface_num).send_datagram( dgram, chosen_interface->next_hop.value() );
    } else {
      interface(chosen_interface->interface_num).send_datagram( dgram, Address::from_ipv4_numeric (dgram.header.dst) );
    }
    
  }
}

bool Router::is_match(uint32_t src_ip, uint32_t dst_ip, uint8_t prefix_length)
{
  if (prefix_length > 32) return false;

  uint32_t mask = (prefix_length != 0 ? 0xffffffff << (32 - prefix_length) : 0);
  return (src_ip & mask) == (dst_ip & mask);
}
