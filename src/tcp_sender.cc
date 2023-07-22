#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return outstanding_cnt_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return retransmit_cnt_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if (queued_messages_.empty())
    return {};

  if (timer_.is_running() == false) {
    timer_.start();
  }

  TCPSenderMessage msg = queued_messages_.front();
  queued_messages_.pop();
  return msg;
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  size_t curr_window_size = (window_size_ == 0) ? 1 : window_size_;
  while (outstanding_cnt_ < curr_window_size)
  {
    TCPSenderMessage msg;
    msg.seqno = isn_ + next_seqno_;
    if (syn_ == false) {
      syn_ = msg.SYN = true;
      outstanding_cnt_ += 1;
    }

    uint64_t payload_size = min(TCPConfig::MAX_PAYLOAD_SIZE, curr_window_size - outstanding_cnt_);
    read(outbound_stream, payload_size, msg.payload);
    outstanding_cnt_ += msg.payload.size();

    if (fin_ == false && outbound_stream.is_finished() && outstanding_cnt_ < curr_window_size )
    {
      fin_ = msg.FIN = true;
      outstanding_cnt_ += 1;
    }

    if (msg.sequence_length() == 0) break;

    queued_messages_.push(msg);
    next_seqno_ += msg.sequence_length();
    outstanding_messages_.push(msg);

    if (msg.FIN || outbound_stream.bytes_buffered() == 0) break;
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  TCPSenderMessage msg;
  msg.seqno = isn_ + next_seqno_;
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  window_size_ = msg.window_size;
  if (msg.ackno.has_value()) {
    uint64_t ackno = msg.ackno.value().unwrap( isn_, next_seqno_ );
    if (ackno > next_seqno_) return;

    acked_seqno_ = ackno;

    while (outstanding_messages_.empty() == false) {
      TCPSenderMessage outstanding_msg = outstanding_messages_.front();
      if (outstanding_msg.seqno.unwrap( isn_, next_seqno_ ) + outstanding_msg.sequence_length() <= acked_seqno_) {
        outstanding_cnt_ -= outstanding_msg.sequence_length();
        outstanding_messages_.pop();

        timer_.reset_RTO();
        if (outstanding_messages_.empty() == false) {
          timer_.start();
        }
        retransmit_cnt_ = 0;
      } else {
        break;
      }
    }

    if ( outstanding_messages_.empty() ) {
      timer_.stop();
    }
  } else {}
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  timer_.tick(ms_since_last_tick);
  if (timer_.is_expire()) 
  {
    queued_messages_.push(outstanding_messages_.front());
    if ( window_size_ != 0 ) 
    {
      timer_.double_RTO();
      retransmit_cnt_++;
    }
    timer_.start();
  }
}
