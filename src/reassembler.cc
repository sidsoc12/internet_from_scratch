#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if(first_index < next_index){ // need to trim initial overlap of substring
    data = data.substr(0,next_index - first_index);
    first_index = next_index;
  }

  if(data.empty()){ // check for empty substring 
    return;
  }

  uint64_t available_capacity = output_.writer().available_capacity();
  if(first_index >= next_index + available_capacity){ // data is entirely beyond capacity
    return;
  }
  


  // Conduct merge with existing buffer
  
  uint64_t start = first_index;
  uint64_t end = first_index + data.size();



  
   
  }



  if(first_index > next_index){ // regular push substring to buffer 
    reassembler_buffer[first_index] = data;
  }

}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  debug( "unimplemented count_bytes_pending() called" );
  return {};
}
