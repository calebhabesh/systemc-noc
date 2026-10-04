"""Write measured results; every row must pass the model's delivery scoreboard."""
import csv
import json
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[1]
exe = str(Path(sys.argv[1]).resolve()) if len(sys.argv) > 1 else str(root / "build/noc")
output = Path(sys.argv[2]) if len(sys.argv) > 2 else root / "docs/evidence/traffic-sweep.csv"
rows = []
for pattern in ("neighbour", "stride", "hotspot"):
    for period in (125, 50, 25, 15, 10):
        run = subprocess.run([exe, pattern, str(period), "100"], check=True, text=True, capture_output=True, timeout=10)
        row = json.loads(run.stdout.splitlines()[-1])
        assert row["verified"]
        rows.append(row)
output.parent.mkdir(parents=True, exist_ok=True)
with output.open("w", newline="") as f:
    writer = csv.DictWriter(f, fieldnames=rows[0])
    writer.writeheader()
    writer.writerows(rows)
print(f"Wrote {len(rows)} verified measurements to {output}")
