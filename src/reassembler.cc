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

  // capacity check #1
  uint64_t available_capacity = output_.writer().available_capacity();
  if(first_index >= next_index + available_capacity){ // data is entirely beyond capacity
    return;
  }


  // Conduct merge with existing buffer
  
  uint64_t start = first_index;
  uint64_t end = first_index + data.size();
  std::string merged_data = data;

  auto it = reassembler_buffer.lower_bound(first_index);
  if( it != reassembler_buffer.begin()){ // find the entry slightly higher than substring index
    --it;  // get entry right before substring index (start merge with that)
  }

  // iterating through all the possible merges 
  while(it -> first < end && it != reassembler_buffer.end() ){
    uint64_t buff_start = it->first;
    uint64_t buff_end = it->first + it->second.size();
    if(buff_start <= end && buff_end >= start){
      start = std::min(start, buff_start);
      end = std::max(end, buff_end);
      // Here, I pad the data string with 0's so I can perform the replace operation later. 
      merged_data = std::string(buff_start - start, '\0') + merged_data + std::string(end - buff_end, '\0');
      merged_data.replace(buff_start - start, it->second.size(), it->second);
      // Remove buffered sring 
      it = reassembler_buffer.erase(it);
    }
    else{
      break;
    }
  }

  // At this point, we have a merged_data which represents the data we can now trim/modify/check before we keep it in the buffer or push it into the stream

  // Do capacity check #2 (trim  bytes beyond capacity) 
  if(start + merged_data.size() > next_index + available_capacity){
    merged_data = merged_data.substr(0, next_index + available_capacity - start);
  }

  if (merged_data.empty()){
    return; 
  } 

  // if merged_data is next_index, auto push to byte stream without needing to add it to the buffer
  if(start == next_index){
    output_.writer().push(merged_data);
    next_index += merged_data.size();
  }
  else{
    // Push merged_data to buffer if not the start
    reassembler_buffer[start] = merged_data;
  }

  // Push next buffer item if starts with next_index
  while(reassembler_buffer.count(next_index)){
    output_.writer().push(reassembler_buffer[next_index]);
    next_index += reassembler_buffer[next_index].size();
    reassembler_buffer.erase(next_index);
  }

  
  









  
   
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
