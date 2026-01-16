#!/usr/bin/env python3
import subprocess
from pathlib import Path
import sys
import argparse
import os

def detect_seqeye(bin_dir: str | None = None) -> str:
    # Prefer new name SeqEyes, fallback to legacy SeqEye for compatibility
    exe_candidates = []
    if os.name == "nt":
        exe_candidates = ["SeqEyes.exe", "SeqEye.exe"]
    else:
        exe_candidates = ["SeqEyes", "SeqEye"]
    if bin_dir:
        for name in exe_candidates:
            cand = Path(bin_dir) / name
            if cand.exists():
                return str(cand)
    # Fallback heuristics if no bin_dir provided
    candidates = []
    for name in exe_candidates:
        candidates.extend([
            Path("out/build/Release")/name,
            Path("out/build/Debug")/name,
            Path("out/build")/name,
            Path(name),
        ])
    for c in candidates:
        if c.exists():
            return str(c)
    # Default to newest name
    return exe_candidates[0]

def main():
    ap = argparse.ArgumentParser(description="Load all .seq headlessly")
    ap.add_argument("--bin-dir", help="Directory containing built executables (SeqEye, tests)")
    args = ap.parse_args()

    exe = detect_seqeye(args.bin_dir)
    seq_dir = Path(__file__).resolve().parents[0] / "seq_files"
    files = sorted(seq_dir.glob("*.seq"))
    passed = 0
    failed = 0
    for f in files:
        print(f"[RUN] {f}")
        cp = subprocess.run([exe, "--headless", "--exit-after-load", str(f)], text=True)
        if cp.returncode == 0:
            print(f"[PASS] {f}")
            passed += 1
        else:
            print(f"[FAIL] {f} (exit={cp.returncode})")
            failed += 1
    print(f"Summary: {passed} passed, {failed} failed")
    sys.exit(0 if failed == 0 else 1)

if __name__ == "__main__":
    main()
