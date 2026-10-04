"""Exercise the real SystemC model, including heavier and contending traffic."""
import json
import subprocess
import sys

exe = sys.argv[1]
for pattern in ("neighbour", "stride", "hotspot"):
    for period in (125, 50, 25, 15, 10):
        result = subprocess.run([exe, pattern, str(period), "100"], capture_output=True, text=True, timeout=10)
        assert result.returncode == 0, (pattern, period, result.stdout, result.stderr)
        row = json.loads(result.stdout.splitlines()[-1])
        assert row["flits_sent"] == row["flits_received"] == 1600, row
        assert row["packets_received"] == 320 and row["verified"], row
        assert row["mean_flit_latency_ns"] > 0 and row["mean_packet_latency_ns"] >= row["mean_flit_latency_ns"], row
        print(json.dumps(row))
# A non-packet-aligned run must fail instead of silently producing partial packets.
assert subprocess.run([exe, "neighbour", "125", "7"], capture_output=True).returncode != 0
print("PASS: 15 end-to-end delivery scenarios; invalid configuration rejected")
