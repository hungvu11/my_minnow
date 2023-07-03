#include "reassembler.hh"
#define low_bound -300

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  uint64_t len = data.length();
  if (size_ <= first_index + len) {
    for (uint64_t i=size_; i < first_index + len + 1; i++) {
      char_pending.push_back(low_bound);
      is_last.push_back(false);
    }
    size_ = first_index + len + 1;
  }

  if (output.available_capacity() > 0 && first_index <= next_byte && next_byte < first_index + len) {
    uint64_t capacity_ = output.available_capacity();
    string s = "";
    uint64_t start = next_byte-first_index;
    for (uint64_t i = start; i < len; i++) {
      if (capacity_ == 0) break;
      if (char_pending[i+first_index] != low_bound) byte_pending--;
      char_pending[i+first_index] = data[i];
      s += data[i];
      next_byte++;
      capacity_--;
      is_last[i+first_index] = is_last_substring;
    }
    
    // uint64_t start = next_byte;
    
    while(char_pending[next_byte] != low_bound && capacity_ > 0) {
      if (next_byte >= first_index + len) byte_pending--;
      s += char_pending[next_byte++];
      capacity_--;
    }
    output.push(s);
    // byte_pending -= (next_byte - start);

    if (is_last[next_byte-1]) output.close();
  } else if (output.available_capacity() > 0 && first_index + len > next_byte && first_index < next_byte + output.available_capacity()) {
    for (uint64_t i=first_index; i<min(first_index + len, next_byte + output.available_capacity()); i++) {
      if (char_pending[i] == low_bound) byte_pending++;
      char_pending[i] = data[i-first_index];
      is_last[i] = is_last_substring;
    }
  } else if (len == 0 && is_last_substring) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.

  return byte_pending;
}
