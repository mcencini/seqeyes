import argparse
import json
import subprocess
import sys
from pathlib import Path
import csv

REPO = Path(__file__).resolve().parents[1]
SEQ_DIR = REPO / "test" / "seq_files"
PERF_DIR = REPO / "test" / "perf"
BASELINE_JSON = PERF_DIR / "baseline.json"
RESULTS_JSON = PERF_DIR / "results.json"
RESULTS_CSV = PERF_DIR / "results.csv"


def detect_exe(bin_dir: Path) -> Path:
    for c in [bin_dir / "PerfZoomTest.exe", bin_dir / "test" / "PerfZoomTest.exe", bin_dir / "PerfZoomTest", bin_dir / "test" / "PerfZoomTest"]:
        if c.exists():
            return c
    raise FileNotFoundError(f"PerfZoomTest not found in {bin_dir}")


def run_one(exe: Path, seq: Path) -> float:
    p = subprocess.run([str(exe), "--seq", str(seq)], capture_output=True, text=True)
    out = p.stdout
    if p.returncode != 0:
        print("[FAIL]", seq, "exit=", p.returncode)
        print(out)
        print(p.stderr)
        return float("inf")
    for line in out.splitlines():
        if line.startswith("ZOOM_MS:"):
            try:
                return float(line.split(":", 1)[1].strip())
            except Exception:
                pass
    return float("inf")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin-dir", type=Path, required=True)
    ap.add_argument("--update-baseline", action="store_true")
    ap.add_argument("--tolerance", type=float, default=0.2, help="Allowed regression ratio (e.g. 0.2 for +20%)")
    args = ap.parse_args()

    PERF_DIR.mkdir(parents=True, exist_ok=True)
    exe = detect_exe(args.bin_dir)

    seqs = sorted(SEQ_DIR.glob("*.seq"))
    results = {}
    for s in seqs:
        ms = run_one(exe, s)
        results[s.name] = ms
        print(f"{s.name}: {ms:.2f} ms")

    with open(RESULTS_JSON, "w", encoding="utf-8") as f:
        json.dump(results, f, indent=2)
    with open(RESULTS_CSV, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["file", "zoom_ms"])
        for k, v in results.items():
            w.writerow([k, v])

    if args.update_baseline or not BASELINE_JSON.exists():
        with open(BASELINE_JSON, "w", encoding="utf-8") as f:
            json.dump(results, f, indent=2)
        print("Baseline updated:", BASELINE_JSON)
        sys.exit(0)

    with open(BASELINE_JSON, "r", encoding="utf-8") as f:
        baseline = json.load(f)

    # Compare with baseline
    failures = []
    for k, v in results.items():
        b = baseline.get(k)
        if b is None:
            continue
        # If no data or inf, mark as failure
        if not (v >= 0 and b >= 0 and v != float("inf") and b != float("inf")):
            failures.append((k, v, b, "invalid"))
            continue
        if v > b * (1.0 + args.tolerance):
            failures.append((k, v, b, f"regressed>{args.tolerance*100:.0f}%"))

    if failures:
        print("\n[PERF REGRESSIONS]")
        for k, v, b, why in failures:
            print(f"  {k}: now {v:.2f} ms, was {b:.2f} ms ({why})")
        sys.exit(1)
    else:
        print("\n[PERF] All within tolerance")


if __name__ == "__main__":
    main()

