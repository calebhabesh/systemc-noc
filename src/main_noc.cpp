// main.cpp
#include "packet.h"
#include "router.h"
#include "sink.h"
#include "source.h"
#include "sysc/tracing/sc_trace.h"
#include "systemc.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;
#include "metrics.h"
Metrics metrics;

int sc_main(int argc, char *argv[]) {

  std::string pattern = argc > 1 ? argv[1] : "neighbour";
  int period = 125;
  try {
    if (argc > 2) period = std::stoi(argv[2]);
    if (argc > 3) metrics.limit = std::stoul(argv[3]);
    if (argc > 4 || period < 10 || period > 1000000 || metrics.limit == 0 || metrics.limit > 1000 || metrics.limit % 5 != 0 ||
        (pattern != "neighbour" && pattern != "stride" && pattern != "hotspot")) throw std::invalid_argument("configuration");
  } catch (...) {
    std::cerr << "Usage: noc [neighbour|stride|hotspot] [source-period-ns >= 10] [flits-per-source, multiple of 5, 5..1000]\n";
    return 2;
  }
  // Local Router Signals
  sc_signal<packet> si_source[16]; // Source to Router Input
  sc_signal<packet> si_sink[16];   // Router to Sink Output
  sc_signal<bool> si_ack_src[16];  // Ack from Router to Source
  sc_signal<bool> si_ack_sink[16]; // Ack from Sink to Router

  // Router-to-Router Signals
  sc_signal<packet> si_to_east[4][3]; // Eastbound Links | No Eastbound Path for
                                      // Last Column, j = 0 - 2
  sc_signal<packet> si_to_west[4][3]; // Westbound Links | No Westbound Path for
                                      // First Column j = 0 - 2
  sc_signal<packet> si_to_north[3][4]; // Northbound Links | No Northbound Path
                                       // for First Row i = 0 - 2
  sc_signal<packet> si_to_south[3][4]; // Southbound Links | No Southbound Path
                                       // for Last Row i = 0 - 2

  // Acks
  sc_signal<bool> si_ack_from_east[4][3];  // Ack for Eastbound Links
  sc_signal<bool> si_ack_from_west[4][3];  // Ack for Eastbound Links
  sc_signal<bool> si_ack_from_north[3][4]; // Ack for Eastbound Links
  sc_signal<bool> si_ack_from_south[3][4]; // Ack for Eastbound Links

  // Unconnected input ports
  sc_signal<packet>
      si_dummy_in[16]; // For unconnected input ports along perimeter
  sc_signal<packet>
      si_dummy_out[16]; // For unconnected output ports along perimeter
  sc_signal<bool>
      si_dummy_ack_in[16]; // For unconnected ack ports along perimeter
  sc_signal<bool>
      si_dummy_ack_out[16]; // For unconnected ack ports along perimeter

  // Router IDs and Destination Signals
  sc_signal<sc_uint<4>> id[16];
  sc_signal<sc_uint<4>> dest[16];

  // Clocks
  sc_clock s_clock("S_CLOCK", period, SC_NS, 0.5, 0.0, SC_NS); // source clock
  sc_clock r_clock("R_CLOCK", 5, SC_NS, 0.5, 10.0, SC_NS);  // router clock
  sc_clock d_clock("D_CLOCK", 5, SC_NS, 0.5, 10.0, SC_NS);  // destination clock

  // MODULE INSTANTATIONS

  // Note that modules can be connected by hooking up ports
  // to signals by name or by using a positional notation

  source *source_array[16];
  router *router_array[16];
  sink *sink_array[16];

  // in/out0 = local source/sink
  // in/out1 = NORTH | UP
  // in/out2 = EAST | RIGHT
  // in/out3 = SOUTH | DOWN
  // in/out4 = WEST | LEFT

  for (int idx = 0; idx < 16; idx++) {
    string name;

    // Instatiate Source
    name = "source" + to_string(idx);
    source_array[idx] = new source(name.c_str());
    source_array[idx]->packet_out(si_source[idx]);
    source_array[idx]->source_id(id[idx]);
    source_array[idx]->ach_in(si_ack_src[idx]);
    source_array[idx]->CLK(s_clock);
    source_array[idx]->dest_id(dest[idx]);

    // Instatiate Router
    name = "router" + to_string(idx);
    router_array[idx] = new router(name.c_str());
    router_array[idx]->router_id(id[idx]);
    router_array[idx]->rclk(r_clock);

    // Instatiate Sink
    name = "sink" + to_string(idx);
    sink_array[idx] = new sink(name.c_str());
    sink_array[idx]->packet_in(si_sink[idx]);
    sink_array[idx]->ack_out(si_ack_sink[idx]);
    sink_array[idx]->sink_id(id[idx]);
    sink_array[idx]->sclk(d_clock);
  }

  // PORT MAPPING
  for (int idx = 0; idx < 16; idx++) {
    int i = idx / 4; // Row
    int j = idx % 4; // Column

    // Local connections
    router_array[idx]->in0(si_source[idx]);
    router_array[idx]->out0(si_sink[idx]);
    router_array[idx]->inack0(si_ack_sink[idx]);
    router_array[idx]->outack0(si_ack_src[idx]);

    // North (in1, out1, inack1, outack1)
    if (i > 0) {
      router_array[idx]->in1(si_to_south[i - 1][j]);
      router_array[idx]->out1(si_to_north[i - 1][j]);
      router_array[idx]->inack1(si_ack_from_north[i - 1][j]);
      router_array[idx]->outack1(si_ack_from_south[i - 1][j]);
    } else {
      router_array[idx]->in1(si_dummy_in[j]);
      router_array[idx]->inack1(si_dummy_ack_in[j]);
      router_array[idx]->out1(si_dummy_out[j]);
      router_array[idx]->outack1(si_dummy_ack_out[j]);
    }

    // East (in2, out2, inack2, outack2)
    if (j < 3) {
      router_array[idx]->in2(si_to_west[i][j]);
      router_array[idx]->out2(si_to_east[i][j]);
      router_array[idx]->inack2(si_ack_from_east[i][j]);
      router_array[idx]->outack2(si_ack_from_west[i][j]);
    } else {
      router_array[idx]->in2(si_dummy_in[i + 8]);
      router_array[idx]->inack2(si_dummy_ack_in[i + 8]);
      router_array[idx]->out2(si_dummy_out[i + 8]);
      router_array[idx]->outack2(si_dummy_ack_out[i + 8]);
    }

    // South (in3, out3, inack3, outack3)
    if (i < 3) {
      router_array[idx]->in3(si_to_north[i][j]);
      router_array[idx]->out3(si_to_south[i][j]);
      router_array[idx]->inack3(si_ack_from_south[i][j]);
      router_array[idx]->outack3(si_ack_from_north[i][j]);
    } else {
      router_array[idx]->in3(si_dummy_in[j + 4]);
      router_array[idx]->inack3(si_dummy_ack_in[j + 4]);
      router_array[idx]->out3(si_dummy_out[j + 4]);
      router_array[idx]->outack3(si_dummy_ack_out[j + 4]);
    }

    // West (in4, out4, inack4, outack4)
    if (j > 0) {
      router_array[idx]->in4(si_to_east[i][j - 1]);
      router_array[idx]->out4(si_to_west[i][j - 1]);
      router_array[idx]->inack4(si_ack_from_west[i][j - 1]);
      router_array[idx]->outack4(si_ack_from_east[i][j - 1]);
    } else {
      router_array[idx]->in4(si_dummy_in[12 + i]);
      router_array[idx]->inack4(si_dummy_ack_in[12 + i]);
      router_array[idx]->out4(si_dummy_out[12 + i]);
      router_array[idx]->outack4(si_dummy_ack_out[12 + i]);
    }
    // Set router ID
    id[idx].write(idx);
  }

  // "stride" is the archived deterministic permutation, not uniform random traffic.
  for (int idx = 0; idx < 16; ++idx) {
    int destination = pattern == "stride" ? (idx + 4) % 16 :
                      pattern == "hotspot" ? (idx == 0 ? 15 : 0) :
                      (idx / 4 % 2 == 0 ? idx + 4 : idx - 4);
    dest[idx].write(destination);
  }

  // VCD TRACE FILE
  sc_trace_file *tf = sc_create_vcd_trace_file("NoC_trace");

  // Trace signals
  sc_trace(tf, s_clock, "s_clock");
  sc_trace(tf, r_clock, "r_clock");
  sc_trace(tf, d_clock, "d_clock");

  for (int i = 0; i < 16; i++) {
    string src_name = "si_source(" + std::to_string(i) + ")";
    string sink_name = "si_sink(" + std::to_string(i) + ")";
    sc_trace(tf, si_source[i], src_name.c_str());
    sc_trace(tf, si_sink[i], sink_name.c_str());
  }

  // Finite injection followed by generous drain time; incomplete runs fail.
  sc_start(16.0 * metrics.limit * (period + 100) + 10000, SC_NS);
  sc_close_vcd_trace_file(tf);
  const bool verified = metrics.complete();
  cout << std::fixed << std::setprecision(3)
       << "{\"pattern\":\"" << pattern << "\",\"source_period_ns\":" << period
       << ",\"flits_sent\":" << metrics.sent << ",\"flits_received\":" << metrics.received
       << ",\"packets_received\":" << metrics.packets
       << ",\"mean_flit_latency_ns\":" << (metrics.received ? metrics.flit_latency_ns / metrics.received : 0)
       << ",\"mean_packet_latency_ns\":" << (metrics.packets ? metrics.packet_latency_ns / metrics.packets : 0)
       << ",\"delivery_window_ns\":" << metrics.last_delivery_ns
       << ",\"flits_per_ns\":" << (metrics.last_delivery_ns ? metrics.received / metrics.last_delivery_ns : 0)
       << ",\"verified\":" << (verified ? "true" : "false") << "}" << endl;
  return verified ? 0 : 1;
}
