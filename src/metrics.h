#pragma once
#include "packet.h"
#include <array>
#include <deque>
#include <iomanip>
#include <stdexcept>

// End-to-end scoreboard: compare the complete flit in source order at its sink.
struct Metrics {
  struct Entry { packet flit; sc_time sent; sc_time packet_started; };
  std::array<std::deque<Entry>, 16> pending;
  std::array<unsigned, 16> emitted{}, delivered{};
  std::array<sc_time, 16> packet_start{};
  unsigned sent = 0, received = 0, packets = 0;
  double flit_latency_ns = 0, packet_latency_ns = 0, last_delivery_ns = 0;
  unsigned limit = 10;
  bool verbose = false;

  void inject(const packet& p) {
    const unsigned src = p.id.to_uint();
    if (emitted[src] % 5 == 0) packet_start[src] = sc_time_stamp();
    pending[src].push_back({p, sc_time_stamp(), packet_start[src]});
    ++emitted[src]; ++sent;
  }
  void receive(const packet& p, unsigned sink) {
    const unsigned src = p.id.to_uint();
    if (p.dest != sink || pending[src].empty() || !(pending[src].front().flit == p))
      throw std::runtime_error("delivery mismatch: wrong destination, order, data, or duplicate flit");
    const auto entry = pending[src].front();
    pending[src].pop_front();
    ++delivered[src]; ++received;
    flit_latency_ns += (sc_time_stamp() - entry.sent).to_seconds() * 1e9;
    last_delivery_ns = sc_time_stamp().to_seconds() * 1e9;
    if (p.h_t == 1) {
      ++packets;
      packet_latency_ns += (sc_time_stamp() - entry.packet_started).to_seconds() * 1e9;
    }
  }
  bool complete() const {
    for (unsigned i = 0; i < 16; ++i)
      if (emitted[i] != limit || delivered[i] != limit || !pending[i].empty()) return false;
    return sent == received && packets == 16 * limit / 5;
  }
};
extern Metrics metrics;
