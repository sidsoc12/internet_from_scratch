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
  return is_closed_; 
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - stream.size(); 
}

uint64_t Writer::bytes_pushed() const
{
  return total_bytes_pushed; 
}

string_view Reader::peek() const
{
  return {stream.data(), stream.size()}; 
}

void Reader::pop( uint64_t len )
{
   if(is_closed_){
      std::cerr << "ERROR: Cannot read" << std::endl;
      return;
   }
   if(stream.empty()){
      std::cerr << "ERROR: Nothing to read. Stream is empty." << std::endl;
      return;
   }
   uint64_t can_pop = std::min(len, stream.size());
   stream.erase(stream.begin(), stream.begin() + can_pop);
   total_bytes_popped += can_pop;
}

bool Reader::is_finished() const
{
  return is_closed_ && stream.empty(); 
}

uint64_t Reader::bytes_buffered() const
{
  return stream.size();
}

uint64_t Reader::bytes_popped() const
{
  return total_bytes_popped;
}

