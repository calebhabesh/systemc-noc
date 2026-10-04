# Provenance

Author of the student submission: Caleb Habesh. Course: COE838, winter 2025.
The original archive was recovered without modifying the school source.

- Course-supplied origin: 1×2 SystemC NoC, router structure, FIFO and crossbar
  infrastructure, and the fixed-priority/output-reservation framework.
- Student extension: 16-node 4×4 mesh and links, XY coordinate routing in the
  arbiter, destination ports, neighbour/stride patterns, timestamp logging and
  a CMake build.
- October 2026 refresh: portable SystemC discovery, automated CLI, finite
  injection/drain, source-order/full-payload/destination scoreboard, tail-based
  packet counts, measured flit and packet latency, hotspot traffic, repeatable
  sweep/plot, FIFO overflow/underflow guards, and per-cycle output availability.

`archive/` preserves the final submitted source and original CMake file; it is
excluded from the current build. Compare with `src/` to see the refresh.
[SHA-256 manifest](evidence/archive-sha256.txt) records the recovered files.
The supplied CPU/other course projects and assignment PDFs are not part of this repository.

Do not infer exclusive authorship of inherited components. No blanket license is
asserted over course-provided code; no upstream license was found in the recovered
project. Preserve existing attribution when referencing the model.
