#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  uint64_t left = output.bytes_pushed();
  uint64_t right = output.bytes_pushed() + output.available_capacity();
  if (first_index <= next_byte && next_byte < first_index + data.length()) {
    int l = next_byte - first_index;
    output.push(data.substr(l, data.length() - l));
    next_byte = output.bytes_pushed();
    if (is_last_substring) output.close();

    for (auto it = remain.begin(); it != remain.end(); it++) {
      string s = (*it).second;
      uint64_t index = (*it).first;
      if (index <= next_byte && next_byte < index + s.length()) {
        int l1 = next_byte - index;
        output.push(s.substr(l1, s.length() - l1));
        next_byte = output.bytes_pushed();
        // byte_pending -= s.length();
        if (mp[*it]) output.close();
        remain.erase(it);
      } else if (index + s.length() <= output.bytes_pushed()) {
        // byte_pending -= s.length();
        remain.erase(it);
      } else if (index > next_byte) break;
    }
  } else if (output.available_capacity() > 0 && first_index + data.length() > left && first_index < right) {
    remain.insert({first_index, data});
    if (is_last_substring) mp[{first_index, data}] = true;
    // byte_pending += data.length();
  } else if (data.length() == 0 && is_last_substring) {
    output.close();
  }

  uint64_t bound = 0;
  byte_pending = 0;
  for (auto it = remain.begin(); it != remain.end(); it++) {
    left = (*it).first;
    right = left + (*it).second.length();
    if (left < bound) {
      if (right > bound) {
        byte_pending += (right - bound);
        bound = right;
      }
    } else {
      byte_pending += (right - left);
      bound = right;
    }
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.

  return byte_pending;
}
