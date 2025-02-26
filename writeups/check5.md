Checkpoint 5 Writeup
====================

My name: Sid Potti

My SUNet ID: sidpotti

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: ejpark24

This checkpoint took me about 6 hours to do. I did attend the lab session.

Program Structure and Design of the NetworkInterface [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]:

The core of my implementation relies on an unordered_map<uint32_t, pair<EthernetAddress, size_t>> for the ARP cache, which stores IP-to-Ethernet mappings along with a timestamp for expiration. I used unordered_map instead of map because ARP lookups need to be fast (constant time on average), and ordering isn't necessary. Each entry is removed after 30 seconds, so the time value helps keep track of when mappings should be purged. The time complexity advantage here is significant - while std::map would provide O(log n) lookups, the unordered_map delivers O(1) average case lookups, which is critical for network performance where latency matters.


For ARP request timing, I used a separate unordered_map<uint32_t, size_t> to track when ARP requests were last sent. This prevents redundant requests within a 5-second window. This approach offers constant time complexity for checking if we need to send another ARP request. An alternative design might have used a priority queue to schedule ARP requests, but this would have introduced O(log n) complexity and unnecessary overhead for what is essentially a simple timeout mechanism.


For handling datagrams waiting on ARP resolution, I used a std::vector<std::pair<uint32_t, InternetDatagram>>, where each entry stores the next-hop IP and the datagram that needs to be sent once the MAC address is learned. I guess a queue might have worked too, but a vector is better here because it allows efficient iteration and removal when an ARP reply arrives. While a queue would provide O(1) push/pop operations, we need to find and process all datagrams for a specific IP address when an ARP reply comes in - vectors allow us to iterate through all pending datagrams in O(n) time and filter by IP.


When an ARP reply is received, I iterate through the vector to find all datagrams waiting on that address, send them out, and remove them. This keeps things simple without needing an extra map. I could have used a more complex data structure like an unordered_multimap mapping IP addresses to datagrams (O(1) lookup), but the expected number of pending datagrams is typically small in practice, making the linear search through a vector acceptable and simpler to implement and maintain.

For time tracking, I used a tick() function that decrements all stored timestamps. The ARP cache timestamps count down from 30 seconds, and when any entry hits zero, it gets erased. The ARP request tracker follows a similar logic, where entries count down from 5 seconds, and once they expire, new requests can be sent if needed. An alternative would have been counting up to 3000 or counting up to 5 but I decided on counting down. 


Implementation Challenges:
I faced a few key challenges while implementing the Network Interface. The most significant issue was correctly managing the ARP cache lifetime - initially I mistakenly set the timer to 30000ms immediately rather than starting at 0 and incrementing it. Another hurdle was namespace handling in C++, as I forgot to use the std:: prefix in my header file, leading to compilation errors. The ARP request timing logic was also tricky; I inadvertently inverted the condition for checking if an ARP request was recently sent by using != instead of == in my comparison. Additionally, balancing the requirements of not flooding the network with ARP requests while ensuring timely packet delivery required careful implementation of both the datagram queue and the timestamp tracking system

Remaining Bugs:
N/A

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
