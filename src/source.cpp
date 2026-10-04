// source.cpp
#include "source.h"
#include "metrics.h"
#include "sysc/kernel/sc_simcontext.h"
void source::func() {
  packet v_packet_out;
  v_packet_out.data = 1000;   // e.g.
  v_packet_out.pkt_clk = '0'; // an imaginary clock for packets

  while (true) {
    wait();
    if (!ach_in.read() && pkt_snt < static_cast<int>(metrics.limit)) {
      v_packet_out.data =
          v_packet_out.data + source_id.read() + 1; // made a desired data
      v_packet_out.id = source_id.read();
      v_packet_out.dest = dest_id.read(); // Configurable destination

      if (v_packet_out.id == v_packet_out.dest)
        goto exclode; // prevent from receiving flits from self

      v_packet_out.pkt_clk =
          ~v_packet_out.pkt_clk; // add an imaginary clock to each flit

      v_packet_out.h_t = false;

      pkt_snt++;
      if ((pkt_snt % 5) == 0)
        v_packet_out.h_t = true; // make tail flit (the packet size is 5)

      send_time = sc_time_stamp(); // assign timestamp to sc_time var

      metrics.inject(v_packet_out);
      packet_out.write(v_packet_out);

      if (metrics.verbose) cout << "t = " << send_time << " | New Pkt: " << v_packet_out.data
           << " is sent by source: " << source_id.read()
           << "  to Destination:   " << v_packet_out.dest << endl;
    exclode:;
    }
  }
}
