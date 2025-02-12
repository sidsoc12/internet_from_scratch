Checkpoint 3 Writeup
====================

My name: [your name here]

My SUNet ID: [your sunetid here]

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This checkpoint took me about [n] hours to do. I [did/did not] attend the lab session.

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

Report from the hands-on component: []

Implementation Challenges:
One of the biggest challenges in implementing this was handling all the small edge cases that arise from the role of the sender. One such issue was dealing with cases where receive is called before push, requiring careful sequence number management to avoid sending incorrect data. Another was handling the transition between zero-window probes and normal transmissions, ensuring that a probe does not interfere with valid payload data while also ensuring the FIN flag is eventually sent. The RST flag handling also required making sure that as soon as an error occurs, the sender immediately stops, clears its state, and ensures subsequent transmissions are prevented. Another difficult part was ensuring that retransmissions happened at the correct times, especially when dealing with zero-window probes, and making sure the RTO did not back off when probing a zero-window connection. Also making sure there were no infinite loops and unintended seqno additions was crucial and at times burdensome to debug. 

Remaining Bugs:
N/A

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [describe where to find]
