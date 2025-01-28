#include "wrapping_integers.hh"
#include "debug.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return Wrap32 {(n + zero_point.raw_value_) % 1UL << 32}; // 1 as a unsigned integer (1UL)
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // Calculate Base
  uint64_t base = (raw_value_ + ((uint64_t(1) << 32) - zero_point.raw_value_)) % (uint64_t(1) << 32);
  

  return {};
}
