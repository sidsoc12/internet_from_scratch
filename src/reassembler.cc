#include "reassembler.hh"
#include "debug.hh"
#include <iostream>
using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
    std::cerr << "Insert called with first_index=" << first_index
              << ", data='" << data << "', is_last_substring=" << is_last_substring << "\n";

    // Step 1: Trim already written data
    if (first_index < next_index) {
        std::cerr << "Trimming overlap: next_index=" << next_index
                  << ", first_index=" << first_index << "\n";

        data = data.substr(next_index - first_index);
        first_index = next_index;
    }

    if (data.empty()) { // Check for empty substring
        if (is_last_substring) {
            last_recieved = true;
            std::cerr << "Marking as last substring received with no data.\n";
            if (reassembler_buffer.empty()&& next_index == output_.writer().bytes_pushed()) {
                std::cerr << "Buffer is empty. Closing ByteStream.\n";
                output_.writer().close();
            }
        }
        return; // Nothing to process
    }

    // Capacity check #1: Check if data is entirely beyond capacity
    uint64_t available_capacity = output_.writer().available_capacity();
    if (first_index >= next_index + available_capacity) {
        std::cerr << "Data is entirely beyond available capacity. Returning early.\n";
        if (is_last_substring) {
            last_recieved = true;
            std::cerr << "Marking as last substring received with no data.\n";
            if (reassembler_buffer.empty() && next_index == output_.writer().bytes_pushed()) {
                std::cerr << "Buffer is empty. Closing ByteStream.\n";
                output_.writer().close();
            }
        }
        return;
    }

    // Step 2: Merge with existing buffer
    uint64_t start = first_index;
    uint64_t end = first_index + data.size();
    std::string merged_data = data;

    auto it = reassembler_buffer.lower_bound(first_index);
    if (it != reassembler_buffer.begin()) {
        --it; // Check the previous entry for overlap
    }

    std::cerr << "Starting merge. Current buffer state:\n";
    for (const auto &entry : reassembler_buffer) {
        std::cerr << "  Index: " << entry.first << ", Data: '" << entry.second << "'\n";
    }

    while (it != reassembler_buffer.end() && it->first < end) {
        uint64_t buff_start = it->first;
        uint64_t buff_end = it->first + it->second.size();

        if (buff_start <= end && buff_end >= start) {
            std::cerr << "Merging with buffer: start=" << start << ", end=" << end
                      << ", buff_start=" << buff_start << ", buff_end=" << buff_end << "\n";

            start = std::min(start, buff_start);
            end = std::max(end, buff_end);
            merged_data = std::string(start - buff_start, '\0') + merged_data + std::string(end - buff_end, '\0');
            merged_data.replace(buff_start - start, it->second.size(), it->second);
            it = reassembler_buffer.erase(it);
        } else {
            ++it;
        }
    }

    std::cerr << "Merge completed. Merged data: '" << merged_data << "', start=" << start << ", end=" << end << "\n";

    // Step 3: Trim merged data based on capacity
    if (start + merged_data.size() > next_index + available_capacity) {
        std::cerr << "Trimming merged_data based on available capacity.\n";
        merged_data = merged_data.substr(0, next_index + available_capacity - start);
    }

    if (merged_data.empty()) {
        std::cerr << "Merged data is empty after capacity trimming. Returning early.\n";
        if (is_last_substring) {
          last_recieved = true;
          std::cerr << "Marking as last substring received with no data.\n";
          if (reassembler_buffer.empty() && next_index == output_.writer().bytes_pushed()) {
              std::cerr << "Buffer is empty and all data written. Closing ByteStream.\n";
              output_.writer().close();
          }
        }
        return;
    }

    // Step 4: Push contiguous data to ByteStream
    if (start == next_index) {
        std::cerr << "Pushing merged_data to ByteStream: '" << merged_data << "'\n";
        output_.writer().push(merged_data);
        next_index += merged_data.size();

        // Push contiguous buffered data
        while (reassembler_buffer.count(next_index)) {
            auto &next_chunk = reassembler_buffer[next_index];
            std::cerr << "Pushing buffered data to ByteStream: '" << next_chunk << "' at next_index=" << next_index << "\n";
            output_.writer().push(next_chunk);
            next_index += next_chunk.size();
            reassembler_buffer.erase(next_index - next_chunk.size());
        }
    } else {
        // Step 5: Buffer non-contiguous data
        std::cerr << "Buffering non-contiguous data at start=" << start << ": '" << merged_data << "'\n";
        reassembler_buffer[start] = merged_data;
    }

    // Step 6: Handle end of stream
    if (is_last_substring) {
    last_recieved = true;
    std::cerr << "Marking as last substring received.\n";
    if (reassembler_buffer.empty() && next_index == output_.writer().bytes_pushed()) {
        std::cerr << "All data is contiguous and complete. Closing ByteStream.\n";
        output_.writer().close();
    }
}
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  uint64_t total = 0;
  for (const auto& [index, data] : reassembler_buffer) {
        total += data.size(); 
  }
  return total;
}
