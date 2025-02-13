Checkpoint 3 Writeup
====================

My name: Sid Potti

My SUNet ID: sidpotti

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This checkpoint took me about 12 hours to do. I did attend the lab session.

Program Structure and Design of the TCPSender [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]: 


I structured the TCPSender around a few core data structures to efficiently manage TCP’s sequencing and flow control. The most important one is a deque, outstanding_segments, which keeps track of unacknowledged packets in order. This allows constant-time access to the oldest unacknowledged segment while maintaining efficient insertion and removal. I also used several global state-tracking variables: next_seqno to track the next sequence number to send, bytes_unack to store the number of bytes currently in flight, and window_size to track the available receive window. Additionally, I introduced zero_window_probe_sent to prevent unnecessary zero-window probes and zero_window_probe_seqno to store the sequence number of the last probe for correct acknowledgment handling. fin_set was necessary to ensure the FIN flag was only sent once, and fin_ready_ was used to defer the FIN transmission in cases where the window was temporarily blocked.

The design choices were made to balance efficiency and correctness. The use of a deque for tracking unacknowledged segments ensures that removing acknowledged packets is efficient, but the tradeoff is that it slightly increases memory usage compared to using a fixed-size buffer. Keeping global state variables rather than recalculating them on every function call reduces redundant operations but increases the risk of state inconsistency if not carefully managed. For complexity, the use of a deque ensures that removing acknowledged packets runs in O(1), and transmitting new packets is also O(1) per segment, making the implementation scalable. The most expensive operations involve iterating through unacknowledged packets in receive(), which is at worst 0(N).

I regarded alternative designs which included using a set instead of a deque for outstanding_segments, but this would have made insertion and iteration more complex. Another option was recalculating window size dynamically rather than storing it as a variable, but this would have increased function call overhead.

I also decided not to use a class originally as I thought it would be relatively simple implementation, but with all the new states and repetitive functions I had to create to manage all the edge cases, looking back on it now, a supplementary class would have been a good idea. 

Report from the hands-on component: 

4.1.1: Works as expected
4.1.2: Works as expected. both programs quit cleanly after both streams have finished with a cntrl D. It needs to wait if its the first to close. 

4.1.3: 
Output works properly as expected:

DEBUG: minnow listening for incoming connection...
DEBUG: minnow new connection from 169.254.144.1:57574.
Absolute Seq: 1, Reassembler Index: 0, FIN: 1, Payload Size: 0
DEBUG: Outbound stream to 169.254.144.1:57574 finished.
DEBUG: minnow outbound stream to 169.254.144.1:57574 finished (0 seqnos still in flight).
DEBUG: minnow outbound stream to 169.254.144.1:57574 has been fully acknowledged.
DEBUG: minnow inbound stream from 169.254.144.1:57574 finished cleanly.
DEBUG: minnow TCP connection finished cleanly.
DEBUG: Inbound stream from 169.254.144.1:57574 finished.
DEBUG: minnow waiting for clean shutdown... done.



</dev/null ./build/apps/tcp_native 169.254.144.9 9090 > /tmp/big-received.txt
DEBUG: Connecting to 169.254.144.9:9090... DEBUG: Successfully connected to 169.254.144.9:9090.
DEBUG: Outbound stream to 169.254.144.9:9090 finished.
DEBUG: Inbound stream from 169.254.144.9:9090 finished.

Hashes match:: 25e21a5023f9f7d031062be0cb35f1fe19d6da9248add3dd78f951deaa5043b2  /tmp/big.txt
25e21a5023f9f7d031062be0cb35f1fe19d6da9248add3dd78f951deaa5043b2  /tmp/big-received.txt

4.3: I did not have a labmate but i tried to replicate it myself using different sizes of the file and sending between tcp_ip4 and native. 

i tried a 200000 byte payload, and it seemed to work fine:

cs144@cs144vm:~/minnow$ dd if=/dev/urandom bs=200000 count=1 of=/tmp/200000.txt
1+0 records in
1+0 records out
200000 bytes (200 kB, 195 KiB) copied, 0.00244092 s, 81.9 MB/s
cs144@cs144vm:~/minnow$ ./build/apps/tcp_ipv4 -l 0 9090
DEBUG: minnow listening for incoming connection...
^C
cs144@cs144vm:~/minnow$ ./build/apps/tcp_native -l 0 9090^C
cs144@cs144vm:~/minnow$ </dev/null ./build/apps/tcp_native 169.254.144.9 9090 > /tmp/big-received.txt
DEBUG: Connecting to 169.254.144.9:9090... DEBUG: Successfully connected to 169.254.144.9:9090.
DEBUG: Outbound stream to 169.254.144.9:9090 finished.
DEBUG: Inbound stream from 169.254.144.9:9090 finished.
cs144@cs144vm:~/minnow$ sha256sum /tmp/big.txt
25e21a5023f9f7d031062be0cb35f1fe19d6da9248add3dd78f951deaa5043b2  /tmp/big.txt
cs144@cs144vm:~/minnow$ sha256sum /tmp/big-received.txt
25e21a5023f9f7d031062be0cb35f1fe19d6da9248add3dd78f951deaa5043b2  /tmp/big-received.txt

I tried a 1000000 byte sequence too: That also seemed to work:

s144@cs144vm:~/minnow$ dd if=/dev/urandom bs=1000000 count=1 of=/tmp/1mb.txt
1+0 records in
1+0 records out
1000000 bytes (1.0 MB, 977 KiB) copied, 0.00999373 s, 100 MB/s
cs144@cs144vm:~/minnow$ </dev/null ./build/apps/tcp_native 169.254.144.9 9090 > /tmp/1mbrec.t
xt
DEBUG: Connecting to 169.254.144.9:9090... DEBUG: Successfully connected to 169.254.144.9:9090.
DEBUG: Outbound stream to 169.254.144.9:9090 finished.
DEBUG: Inbound stream from 169.254.144.9:9090 finished.
cs144@cs144vm:~/minnow$ sha256sum /tmp/1mb.txt
e698b6b074daca2583ab114fa0e81e4f47c721b6780cdedfb66a93d711fa2f78  /tmp/1mb.txt
cs144@cs144vm:~/minnow$ sha256sum /tmp/1mbrec.txt
e698b6b074daca2583ab114fa0e81e4f47c721b6780cdedfb66a93d711fa2f78  /tmp/1mbrec.txt
cs144@cs144vm:~/minnow$ 





Implementation Challenges:
One of the biggest challenges in implementing this was handling all the small edge cases that arise from the role of the sender. One such issue was dealing with cases where receive is called before push, requiring careful sequence number management to avoid sending incorrect data. Another was handling the transition between zero-window probes and normal transmissions, ensuring that a probe does not interfere with valid payload data while also ensuring the FIN flag is eventually sent. The RST flag handling also required making sure that as soon as an error occurs, the sender immediately stops, clears its state, and ensures subsequent transmissions are prevented. Another difficult part was ensuring that retransmissions happened at the correct times, especially when dealing with zero-window probes, and making sure the RTO did not back off when probing a zero-window connection. Also making sure there were no infinite loops and unintended seqno additions was crucial and at times burdensome to debug. 

Remaining Bugs:
N/A

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [describe where to find]
