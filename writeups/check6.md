Checkpoint 6 Writeup
====================

My name: Sid Potti

My SUNet ID: sidpotti

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: 

This checkpoint took me about [n] hours to do. I [did/did not] attend the lab session.

Program Structure and Design of the Router [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]: 

For the router implementation, I decided to use a simple array-based approach (well, technically a std::vector, but conceptually an array) to store the routing table. This means that every time a packet arrives, I have to loop through all routes in the table to find the best match, making the routing decision O(N) in the worst case, where N is the number of routes. While this isn’t the most optimized way to do it, I felt that this approach was simple, straightforward, and easy to debug.

To make the longest prefix match as efficient as possible, I sort the vector of routes in descending order by prefix length every time a new route is added. This ensures that when I loop through the routes in route(), the first match I find is always the best match. Sorting happens in O(N log N) time , but that’s only done once per new route, not per packet, so it doesn’t really impact the core routing performance.

For actually matching a route to a destination IP, I implemented a bit-by-bit comparison. Instead of using what could have been a more complicated but stylish bit masking trick, I just looped through the first prefix_length bits of both the route prefix and the destination IP, checking each bit one at a time using (num >> i) & 1. If any bit didn’t match, I immediately broke out of the loop and moved on to the next route. This is technically O(32) = O(1) per route since we only compare at most 32 bits, but since we loop through up to N routes, the worst case is O(N) per packet. That’s still reasonable given the problem constraints.

I considered using a prefix tree to make lookup O(1) per packet, but that would have added a lot of complexity, and debugging would have been harder. The array approach is brute-force but reliable, and it makes debugging easier because I can just print out the routing table and step through it logically. 

Implementation Challenges:
One challenge was when packets weren’t arriving at their expected destinations. A major issue I ran into was TTL handling—I was originally decrementing TTL before checking if it should be dropped, which caused incorrect behavior. After realizing this, I changed the logic to check if TTL ≤ 1 first, then drop, otherwise decrement, which fixed the issue.

Another tricky part was ensuring the checksum was recomputed after modifying TTL. I didn’t initially realize that changing the TTL invalidates the checksum, so packets were getting forwarded with incorrect checksums and being rejected as "bad IPv4 datagrams." Once I added datagram.header.compute_checksum(); after decrementing TTL, the problem was solved.

Also, working with std::queue was a bit annoying because it doesn’t allow iteration, so I had to carefully pop() each datagram and process it one at a time, which made handling the queue a bit more cumbersome compared to a vector or list. Also using a reference was important to remember. 

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
