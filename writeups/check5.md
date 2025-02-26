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

The core of my implementation relies on an unordered_map<uint32_t, pair<EthernetAddress, size_t>> for the ARP cache, which stores IP-to-Ethernet mappings along with a timestamp for expiration. I used unordered_map instead of map because ARP lookups need to be fast (constant time on average), and ordering isn’t necessary. Each entry is removed after 30 seconds, so the time value helps keep track of when mappings should be purged. For ARP request timing, I used a separate unordered_map<uint32_t, size_t> to track when ARP requests were last sent. This prevents redundant requests within a 5-second window. The strategy was to coordinate between lookups amongst these two maps throughout the code. 

For handling datagrams waiting on ARP resolution, I used a std::vector<std::pair<uint32_t, InternetDatagram>>, where each entry stores the next-hop IP and the datagram that needs to be sent once the MAC address is learned. A queue might have worked, but a vector is better here because it allows efficient iteration and removal when an ARP reply arrives. Mainly I wanted to use a vector so that I could quickly iterate through the different datagrams that should be sent or discarded when new ARP info comes in. I could have used a std::deque for efficient popping, but the number of pending datagrams should be small, and a vector avoids the overhead of a more complex data structure. When an ARP reply is received, I iterate through the vector to find all datagrams waiting on that address, send them out, and remove them. This keeps things simple without needing an extra map.

For time tracking, I used a tick() function that decrements all stored timestamps. The ARP cache timestamps count down from 30 seconds, and when any entry hits zero, it gets erased. The ARP request tracker follows a similar logic, where entries count down from 5 seconds, and once they expire, new requests can be sent if needed. An alternative would have been counting up to 3000 or counting up to 5 but I decided on counting down. 


Implementation Challenges:
[]

Remaining Bugs:
N/A

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
