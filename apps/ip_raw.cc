#include "socket.hh"

using namespace std;

class RawSocket : public DatagramSocket
{
public:
  RawSocket() : DatagramSocket( AF_INET, SOCK_RAW, IPPROTO_RAW ) {}
};

int main()
{
  // construct an Internet or user datagram here, and send using the RawSocket as in the Jan. 10 lecture
  string d;
  d += char (0b0100'0101);
  d += string(7,0);
  d += char (64);
  d += char(17); // UDP protocol. 
  d += string(6,0);
  d += char(10);
  d += char(144);
  d += char(0);
  d += char(231);
  // d += "Namaste";

  // UDP 
  d += char(0);
  d += char(1);
  d += char(4);
  d += char(0);
  string user_payload = "Heyyyy";
  d+= char(0);
  d += char(user_payload.length() + 8);
  d += string(2,0);
  d += user_payload;
  RawSocket{}.sendto(Address{"10.144.0.231"}, d);
  return 0;

  // UDPSocket sock;
  // sock.sendto(Address { "10.144.0.154", 1024}, "THIS WORKS FINALLY");
  // return 0; 

}
