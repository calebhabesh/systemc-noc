# Verification, 2026-10-04

Environment: Linux x86-64, GCC 16.2.1, Accellera SystemC 3.0.0, C++17.
`cmake --build build -j4` and `ctest --test-dir build --output-on-failure` passed.
Run outputs and the traffic CSV are retained under `docs/evidence/`.

The scoreboard queues every emitted flit per source, including the entire payload,
source/destination IDs, changing marker and tail bit. Sink receipt must match the
next entry and the actual destination. Completion requires every source to emit
and deliver its configured count and every packet tail to arrive. FIFO operations
throw on overflow/underflow. End-to-end tests exercise 24,000 flits in total.

Metrics:

- Flit latency: receipt time minus that flit's injection time.
- Packet latency: tail receipt time minus the packet's first-flit injection time.
- Completed packets: received tail flits, checked against the five-flit packet size.
- Delivery window: final sink receipt time relative to simulation time zero.
- Throughput: received flits / delivery window; the plot multiplies by the 5 ns
  router period to express aggregate flits per router cycle.
- Configured injection: `5 ns / source period`. Backpressure can reduce actual injection.

The historical neighbour source was separately built and run: all 160 emitted
flits reached the intended sink. Its per-sink output says “packets”, although
those counters increment once per flit. The refresh removes that misleading label.
The low-load stride/neighbor latency measurements agree with the report's stated
12.5/10 ns values, now calculated automatically with explicit units.

The regression workloads are neighbour, stride and hotspot at 125, 50, 25, 15 and
10 ns source periods, with 100 flits per source. The runner times out failed
simulations. These results do not establish fairness, arbitrary-traffic reliability,
physical timing or steady-state saturation. Trace files are generated in the working
directory and ignored by git.
