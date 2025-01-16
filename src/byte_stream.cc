#include "byte_stream.hh"
#include <iostream>


using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  uint64_t available_space = capacity_ - stream.size();
  uint64_t available_write = std::min(available_space, data.size());
  if(is_closed || available_space == 0){
    std::cerr << "ERROR: Writer is closed" << std::endl;
    return;
  }
  stream.insert(stream.end(), data.begin(), data.begin() + available_write);
  total_bytes_pushed += available_write;
}

void Writer::close()
{
  is_closed_ = true;
}

bool Writer::is_closed() const
{
  return is_closed_; // Your code here.
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - stream.size(); // Your code here.
}

uint64_t Writer::bytes_pushed() const
{
  return {}; // Your code here.
}

string_view Reader::peek() const
{
  return {}; // Your code here.
}

void Reader::pop( uint64_t len )
{
  (void)len; // Your code here.
}

bool Reader::is_finished() const
{
  return {}; // Your code here.
}

uint64_t Reader::bytes_buffered() const
{
  return {}; // Your code here.
}

uint64_t Reader::bytes_popped() const
{
  return {}; // Your code here.
}

