#include "tcp_receiver.hh"
#include <cstdint>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.

  if (message.SYN) {
    zero_point = message.seqno;
    syn_flag = true;
  } else if (syn_flag == false) return;
  
  uint64_t checkpoint = inbound_stream.bytes_pushed();
  uint64_t abs_seqno = Wrap32(message.seqno).unwrap(zero_point, checkpoint);

  if (message.FIN) fin_flag = true;
  
  reassembler.insert(abs_seqno + message.SYN - 1, message.payload, message.FIN, inbound_stream);
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  (void)inbound_stream;
  TCPReceiverMessage output;
  output.window_size = (inbound_stream.available_capacity() > UINT16_MAX) ? UINT16_MAX : inbound_stream.available_capacity();

  uint64_t num_bytes = inbound_stream.bytes_pushed();
  if (syn_flag) {
    if (fin_flag && inbound_stream.is_closed()) num_bytes++;
    output.ackno = Wrap32().wrap(num_bytes + 1, zero_point);
  }
  return output;
}