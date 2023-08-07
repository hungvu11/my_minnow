#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  uint64_t MAX = 1LL << 32;
  uint32_t n_ = uint32_t (n % MAX);
  return Wrap32 { zero_point + n_ };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  uint32_t offset = this->raw_value_ - zero_point.raw_value_;
  
  if (offset >= checkpoint) return offset;

  uint64_t MAX = 1LL << 32;
  uint64_t real_part = checkpoint - offset + (MAX >> 1);

  return real_part / MAX * MAX + offset;
}
