#include "reassembler.hh"
#include "debug.hh"
#include <iostream>
#include <numeric>
#include <optional>


using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // std::cerr << "Insert called with first_index=" << first_index
  //           << ", data='" << data << "', is_last_substring=" << is_last_substring << "\n";

  // if (first_index < next_index) { // need to trim initial overlap of substring
  //     std::cerr << "Trimming overlap: next_index=" << next_index
  //               << ", first_index=" << first_index << "\n";

  //     data = data.substr(next_index - first_index);
  //     first_index = next_index;
  // }

  // // Check for empty substring
  // if (data.empty()) {
  //     std::cerr << "Data is empty.\n";
  //     if (is_last_substring) {
  //         last_recieved = true;
  //         last_substring_index = first_index;
  //         std::cerr << "Marking as last substring. last_substring_index=" << last_substring_index << "\n";
  //         if (reassembler_buffer.empty() && next_index == last_substring_index) {
  //             std::cerr << "Buffer is empty. Closing ByteStream.\n";
  //             output_.writer().close();
  //         }
  //     }
  //     return;
  // }

  // // capacity check #1
  // uint64_t available_capacity = output_.writer().available_capacity();
  // if (first_index >= next_index + available_capacity) {
  //     std::cerr << "Data is entirely beyond available capacity.\n";
  //     if (is_last_substring) {
  //         last_recieved = true;
  //         last_substring_index = first_index + data.size();
  //         std::cerr << "Marking as last substring. last_substring_index=" << last_substring_index << "\n";
  //         if (reassembler_buffer.empty() && next_index == last_substring_index) {
  //             std::cerr << "Buffer is empty. Closing ByteStream.\n";
  //             output_.writer().close();
  //         }
  //     }
  //     return;
  // }

  // // Conduct merge with existing buffer
  // uint64_t start = first_index;
  // uint64_t end = first_index + data.size();
  // std::string merged_data = data;

  // auto it = reassembler_buffer.lower_bound(first_index);
  // if (it != reassembler_buffer.begin()) { // find the entry slightly higher than substring index
  //     --it;  // get entry right before substring index (start merge with that)
  // }

  // std::cerr << "Starting merge. Current buffer state:\n";
  // for (const auto &entry : reassembler_buffer) {
  //     std::cerr << "  Index: " << entry.first << ", Data: '" << entry.second << "'\n";
  // }

  // // iterating through all the possible merges
  // while (it != reassembler_buffer.end() && it->first < end) {
  //     uint64_t buff_start = it->first;
  //     uint64_t buff_end = it->first + it->second.size();
  //     if (buff_start <= end && buff_end >= start) {
  //         std::cerr << "Merging with buffer: start=" << start << ", end=" << end
  //                   << ", buff_start=" << buff_start << ", buff_end=" << buff_end << "\n";

  //         start = std::min(start, buff_start);
  //         end = std::max(end, buff_end);
  //         merged_data = std::string(buff_start - start, '\0') + merged_data + std::string(end - buff_end, '\0');
  //         merged_data.replace(buff_start - start, it->second.size(), it->second);
  //         it = reassembler_buffer.erase(it);
  //     } else {
  //         ++it;
  //     }
  // }

  // std::cerr << "Merge completed. Merged data: '" << merged_data << "', start=" << start << ", end=" << end <<
  // "\n";

  // // Do capacity check #2 (trim bytes beyond capacity)
  // if (start + merged_data.size() > next_index + available_capacity) {
  //     std::cerr << "Trimming merged_data based on available capacity.\n";
  //     merged_data = merged_data.substr(0, next_index + available_capacity - start);
  // }

  // if (merged_data.empty()) {
  //     std::cerr << "Merged data is empty after capacity trimming. Returning early.\n";
  //     if (is_last_substring) {
  //         last_recieved = true;
  //         last_substring_index = first_index + data.size();
  //         std::cerr << "Marking as last substring. last_substring_index=" << last_substring_index << "\n";
  //         if (reassembler_buffer.empty() && next_index == last_substring_index) {
  //             std::cerr << "Buffer is empty. Closing ByteStream.\n";
  //             output_.writer().close();
  //         }
  //     }
  //     return;
  // }

  // // if merged_data is next_index, auto push to byte stream without needing to add it to the buffer
  // if (start == next_index) {
  //     std::cerr << "Pushing merged_data to ByteStream: '" << merged_data << "'\n";
  //     output_.writer().push(merged_data);
  //     next_index += merged_data.size();
  // } else {
  //     // Push merged_data to buffer if not the start
  //     std::cerr << "Buffering non-contiguous data at start=" << start << ": '" << merged_data << "'\n";
  //     reassembler_buffer[start] = merged_data;
  // }

  // // Push next buffer item if starts with next_index
  // while (reassembler_buffer.count(next_index)) {
  //     auto &next_chunk = reassembler_buffer[next_index];
  //     std::cerr << "Pushing buffered data to ByteStream: '" << next_chunk << "' at next_index=" << next_index <<
  //     "\n"; output_.writer().push(next_chunk); next_index += next_chunk.size();
  //     reassembler_buffer.erase(next_index - next_chunk.size());
  // }

  // // Close the bytestream if last received is true and all reassembler components have been emptied into stream
  // if (is_last_substring) {
  //     last_recieved = true;
  //     last_substring_index = first_index + data.size();
  //     std::cerr << "Marking as last substring. last_substring_index=" << last_substring_index << "\n";
  //     if (reassembler_buffer.empty() && next_index == last_substring_index) {
  //         std::cerr << "All data is contiguous and complete. Closing ByteStream.\n";
  //         output_.writer().close();
  //     }
  // }

  // CODE TAKEN FROM LECTURE SOLUTIONS
  if ( is_last_substring ) {
    total_size_.emplace( first_index + data.size() );
  }

  buf_.resize( writer().available_capacity() );
  occupancy_.resize( writer().available_capacity() );



  auto first_unassembled = writer().bytes_pushed();
  auto first_unacceptable = first_unassembled + writer().available_capacity();
  auto left_wanted = max( first_unassembled, first_index );
  auto right_wanted = min( first_unacceptable, first_index + data.size() );
  auto left_in_data = left_wanted - first_index, right_in_data = right_wanted - first_index;
  auto left_in_buf = left_wanted - first_unassembled, right_in_buf = right_wanted - first_unassembled;

  if ( left_wanted < right_wanted ) {
    copy( data.begin() + left_in_data, data.begin() + right_in_data, buf_.begin() + left_in_buf );
    fill( occupancy_.begin() + left_in_buf, occupancy_.begin() + right_in_buf, true );
  }

  auto count = ranges::find( occupancy_, false ) - occupancy_.begin();
  output_.writer().push( buf_.substr( 0, count ) );
  buf_.erase( 0, count );
  occupancy_.erase( occupancy_.begin(), occupancy_.begin() + count );

  if ( total_size_.has_value() and total_size_.value() == writer().bytes_pushed() ) {
    output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  //   uint64_t total = 0;
  //   for (const auto& [index, data] : reassembler_buffer) {
  //         total += data.size();
  //   }
  //   return total;
  return accumulate( occupancy_.begin(), occupancy_.end(), 0 );
}
