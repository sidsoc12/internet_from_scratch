Checkpoint 0 Writeup
====================

My name: Sid Potti

My SUNet ID: sidpotti

I collaborated with: [list sunetids here]

I would like to credit/thank these classmates for their help: [list sunetids here]

This lab took me about [5] hours to do. I did attend the lab session.

My secret code from section 2.1 was: 185776

I was surprised by or edified to learn that: the socket is really just a file descriptor associated with a ip/port address mapped by the OS. 

Describe ByteStream implementation. [Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

My ByteStream implementation uses a std::vector<char> as the underlying buffer to store the stream of bytes, along with state variables like capacity_, is_closed_, total_bytes_pushed, and total_bytes_popped to track the stream's size, state, and usage metrics. I chose std::vector<char> because it provides a simple, efficient, and well-tested way to manage a dynamic array of bytes. Data is added using std::vector::insert, and bytes are removed using erase. While erase incurs a linear time cost for shifting elements, I prioritized simplicity and maintainability since the buffer is designed for small to medium-sized workloads where this tradeoff is acceptable. I considered alternatives, like using std::deque. Deque would allow for constant-time removal of elements from the front, which could improve performance for frequent pop operations. However, these designs add complexity, and the benefits didn’t outweigh the simplicity of sticking with std::vector<char>. I also ensured that the implementation tracks the state of the stream explicitly, with is_closed_ signaling when no more bytes can be written, and error_ indicating invalid operations. Overall, while std::vector<char> isn't the most efficient choice for very high-throughput usecases, it allowed me to implement the functionality quickly and with minimal risk of bugs

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I contributed a new test case that catches a plausible bug
  not otherwise caught: [provide Pull Request URL]