"""Plot the recorded sweep (optional matplotlib dependency)."""
import csv
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

root = Path(__file__).resolve().parents[1]
rows = list(csv.DictReader((root / "docs/evidence/traffic-sweep.csv").open()))
plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 11})
fig, axes = plt.subplots(1, 2, figsize=(12, 4.6), layout="constrained")
for pattern, color in [("neighbour", "#38705a"), ("stride", "#596b95"), ("hotspot", "#ad583e")]:
    points = [r for r in rows if r["pattern"] == pattern]
    x = [5 / float(r["source_period_ns"]) for r in points]
    axes[0].plot(x, [float(r["mean_flit_latency_ns"]) for r in points], "o-", label=pattern, color=color)
    axes[1].plot(x, [5 * float(r["flits_per_ns"]) for r in points], "o-", label=pattern, color=color)
for ax in axes:
    ax.set_xlabel("Configured injection / source / router cycle")
    ax.grid(alpha=0.2)
    ax.spines[["top", "right"]].set_visible(False)
axes[0].set_yscale("log")
axes[0].set_ylabel("Mean flit latency (simulated ns)")
axes[1].set_ylabel("Aggregate delivered flits / router cycle")
axes[1].legend(frameon=False)
fig.suptitle("4×4 SystemC mesh · 1,600 verified flits per run", fontweight="bold")
fig.savefig(root / "docs/assets/traffic-sweep.png", dpi=160)
print("Saved docs/assets/traffic-sweep.png")
