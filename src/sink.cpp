// sink.cpp
#include "sink.h"
#include "metrics.h"
#include "sysc/kernel/sc_simcontext.h"
void sink::receive_data() {

  packet v_packet;
  if (sclk.event())
    ack_out.write(false);

  if (packet_in.event()) {
    pkt_recv++;
    ack_out.write(true);
    v_packet = packet_in.read();

    metrics.receive(v_packet, sink_id.read().to_uint());
    receive_time = sc_time_stamp(); // assign timestamp to sc_time var

    if (metrics.verbose) cout << "\tt = " << receive_time << " | New Pkt: " << (int)v_packet.data
         << " from source: " << (int)v_packet.id
         << " to sink: " << (int)sink_id.read() << " (dest was "
         << (int)v_packet.dest << ")" << endl;
  }
}
