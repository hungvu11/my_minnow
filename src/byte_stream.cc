#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), stream() {}

void Writer::push( string data )
{
  // Your code here.
  for (char c : data) {
    if (size_ == capacity_) break;
    stream.push(c);
    size_++;
    total_++;
  }
}

void Writer::close()
{
  // Your code here.
  isClose = true;
}

void Writer::set_error()
{
  // Your code here.
  isError = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return isClose;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - size_;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return total_;
}

string_view Reader::peek() const
{
  // Your code here.
  return string_view(&stream.front(), 1);
}

bool Reader::is_finished() const
{
  // Your code here.
  return (isClose && (size_ == 0));
}

bool Reader::has_error() const
{
  // Your code here.
  return isError;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  for (uint64_t i=0; i<len; i++) {
    if (size_ == 0) break;
    stream.pop();
    size_--;
  }
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return size_;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return (total_ - size_);
}
