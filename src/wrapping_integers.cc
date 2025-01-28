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
   // Calculate base
  uint64_t base = (raw_value_ + ((uint64_t(1) << 32) - zero_point.raw_value_)) % (uint64_t(1) << 32);
  uint64_t before_checkpoint = base + ((checkpoint - base) / (1UL << 32)) * (1UL << 32);
  uint64_t after_checkpoint = before_checkpoint + (1UL << 32);
  // find candidates right before and after checkpoint
  uint64_t diff_before = checkpoint - before_checkpoint;
  uint64_t diff_after = after_checkpoint - checkpoint;
  // Compare candidates 
  if(diff_before > diff_after){
    return after_checkpoint;
  }
  return before_checkpoint;
}
