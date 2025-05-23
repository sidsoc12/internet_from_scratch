#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <deque>

#include <functional>

class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) )
    , isn_( isn )
    , initial_RTO_ms_( initial_RTO_ms )
    , window_size( 1 )
    , outstanding_segments()
    , next_seqno( 0 )
    , syn_sent_( false )
    , bytes_unack( 0 )
    , consecutive_retransmissions_( 0 )
    , RTO( initial_RTO_ms )
    , timer( 0 )
    , timer_active( false )
    , fin_set( false )
    , zero_window_probe_sent( false )
    , zero_window_probe_seqno( std::nullopt )
    , fin_ready_( false )
  {}

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // For testing: how many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // For testing: how many consecutive retransmissions have happened?
  const Writer& writer() const { return input_.writer(); }
  const Reader& reader() const { return input_.reader(); }
  Writer& writer() { return input_.writer(); }

private:
  Reader& reader() { return input_.reader(); }
  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  uint16_t window_size { 1 }; // initialize window size to 1
  std::deque<TCPSenderMessage>
    outstanding_segments; // using a double ended queue as my data structure for managing outstanding_segments
  uint64_t next_seqno { 0 };
  bool syn_sent_ { false };
  uint64_t bytes_unack { 0 };
  uint64_t consecutive_retransmissions_ { 0 };
  uint64_t RTO;
  uint64_t timer { 0 };
  bool timer_active { false };
  bool fin_set { false };
  bool zero_window_probe_sent { false };
  std::optional<uint64_t> zero_window_probe_seqno; // Store the seqno of the last zero-window probe
  bool fin_ready_ { false }; // this is just for when the probe gets acknowed, and fin needs to be sent
};
