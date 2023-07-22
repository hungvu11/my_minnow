#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <queue>

class Timer
{
  uint64_t initial_RTO_ms_;
  uint64_t curr_RTO_ms_;
  uint64_t time_ms_ {0};
  bool running_ { false };

public:
  explicit Timer( uint64_t init_RTO ) : initial_RTO_ms_( init_RTO ), curr_RTO_ms_( init_RTO ) {};
  void start()
  {
    running_ = true;
    time_ms_ = 0;
  }

  bool is_running() const { return running_; }

  void stop() { running_ = false; }

  bool is_expire() { return running_ && (time_ms_ >= curr_RTO_ms_); }

  void tick( uint64_t ms_since_last_tick )
  {
    if (running_)
      time_ms_ += ms_since_last_tick;
  }

  void double_RTO() { curr_RTO_ms_ *= 2; }

  void reset_RTO() { curr_RTO_ms_ = initial_RTO_ms_; }
};
class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  bool syn_ {false};
  bool fin_ {false};
  uint16_t window_size_ {1};
  uint64_t acked_seqno_ {0};
  uint64_t next_seqno_ {0};

  uint64_t retransmit_cnt_ {0};
  uint64_t outstanding_cnt_ {0};

  std::queue<TCPSenderMessage> outstanding_messages_ {};
  std::queue<TCPSenderMessage> queued_messages_ {};

  Timer timer_{ initial_RTO_ms_ };

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
