Checkpoint 2 Writeup
====================

My name: Sid Potti

My SUNet ID: sidpotti

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This lab took me about [9] hours to do. I did attend the lab session.

Describe Wrap32 and TCPReceiver structure and design. [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

For the Wrap32 implementation, I designed a system that efficiently converts between 64-bit absolute sequence numbers and 32-bit wrapped sequence numbers, crucial for handling TCP’s sequence number wrapping. The wrap() function ensures a given absolute sequence number is correctly mapped to a relative sequence number based on the Initial Sequence Number (ISN). I encountered overflow/underflow issues when dealing with modular arithmetic, so I had to explicitly cast values to uint64_t and int64_t at key steps to prevent unexpected behavior due to implicit type conversions. My unwrap() implementation initially considered looping through all possible absolute sequence numbers by iterating through multiples of 2^32 , but this was inefficient. Instead, I developed an optimized O(1) approach by calculating the nearest absolute sequence number using integer division and modulo operations. I compute a base absolute sequence number, then determine the closest match by checking both the previous and next possible valid numbers relative to a given checkpoint. The logic ensures that unwrapping always produces the closest valid absolute sequence number, reducing complexity and significantly improving performance.

For the TCPReceiver, my approach focused on efficiently handling incoming TCP segments, ensuring correctness in acknowledgment tracking, window size management, and stream reconstruction. A key challenge was handling a wide range of edge cases, particularly when SYN and FIN flags arrived without payloads, SYN+FIN were received together, or out-of-order segments needed to be properly managed. Furthermore, when a SYN arrives without a payload, the receiver must correctly initialize the stream while ensuring ackno is properly set. When a SYN+FIN segment is received together, the system must immediately close the stream while ensuring correct indexing. I also had to add the error handling functionality. When an RST flag is received, the receiver must immediately abort processing and set an error state in the underlying ByteStream. Instead of designing a highly generalized implementation that attempts to automatically handle all cases, I opted for a case-by-case approach, where I explicitly check for and handle each edge case individually. This decision was based on development speed and clarity, as debugging discrete conditions proved simpler than designing a fully generalized solution. However, in a future iteration, I would explore a more universal implementation that abstracts these cases into a single streamlined logic, ensuring automatic handling of most edge cases without requiring explicit condition checks. Additionally, I had to carefully manage acknowledgment number updates, ensuring that the next expected byte was always reflected in the ackno, properly incorporating both SYN and FIN bytes while avoiding premature updates.

Debug: To debug, one of the most effective strategies I used was inserting debug print statements at critical points in the code to track sequence numbers, acknowledgment numbers (ackno), reassembler indices, and window sizes. For instance, in TCPReceiver, I printed out absolute sequence numbers, reassembler indices, FIN flags, and payload sizes every time data was processed, which helped me catch issues where indices were being miscalculated—particularly in edge cases involving SYN without payload, FIN without data, or out-of-order segments.

For Wrap32, I initially suspected that my unwrap function had issues due to improper handling of modular arithmetic and signed vs. unsigned integer conversions. To verify correctness, I tested specific cases where the checkpoint was near the 32-bit wrap boundary, ensuring that the closest valid absolute sequence number was being selected. Initially, I attempted a brute-force approach, looping through all possible values, but after realizing the inefficiency, I implemented an optimized approach and validated it against known expected values.

I also leveraged the provided test suite to iteratively refine my implementation. When a test failed, I analyzed the expected vs. actual output and traced the error back to specific miscalculations in state transitions or sequence number management. One recurring issue was incorrect acno calculations, where I initially updated ackno too early, before the FIN byte could actually be assembled. By comparing my output against the expected acknowledgment values, I refined my logic to only increment ackno when the FIN byte was actually assembled.

Handling RST errors required careful verification as well. When the "Stream error → RST flag" test failed, I realized I wasn’t correctly propagating errors to the ByteStream. After reviewing the project structure, I identified that I needed to use reassembler_.reader().set_error() to correctly signal the error state, which ultimately resolved the issue.

Additionally, I paid close attention to corner cases that weren’t explicitly covered in standard tests. I manually constructed test cases to validate behavior when:

SYN and FIN were received simultaneously with no payload.
Payload data was received before the SYN.
Duplicate SYN segments were sent.
The FIN flag was received but the data couldn’t yet be assembled.

Implementation Challenges:
I merged this section with the section above. 

Remaining Bugs:
N/A

- Optional: I had unexpected difficulty with: Finding all of the edge cases. Some were easy to think of ahead of time, but others were quite difficult to find and think of. 

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [describe where to find]
