#!/usr/bin/env python3
import argparse
import subprocess
import tempfile
import shutil
import os
import statistics
from pathlib import Path

# This script compares the performance of two commits in the utf8cpp library
# by running benchmarks for different scenarios and functions.

# Example:
# ./bench/compare_commits.py HEAD~1 HEAD

SCENARIOS = ["ascii", "cyrillic", "mixed"]
FUNCTIONS = ["utf8::next", "utf8::unchecked::next", "utf8::find_invalid"]

def run(cmd, cwd=None):
    result = subprocess.run(cmd, cwd=cwd, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"Command failed: {' '.join(cmd)}\n{result.stderr}")
    return result.stdout

def checkout_commit(commit_ref):
    tempdir = tempfile.mkdtemp(prefix="utfcpp_")
    archive = subprocess.Popen(["git", "archive", commit_ref],
                               stdout=subprocess.PIPE)
    subprocess.run(["tar", "-xC", tempdir], stdin=archive.stdout)
    return tempdir

def build_benchmark(source_dir):
    build_dir = Path(source_dir) / "build"
    build_dir.mkdir(exist_ok=True)
    run(["cmake", "..", "-DCMAKE_BUILD_TYPE=Release",
         "-DUTF8CPP_ENABLE_BENCHMARKS=ON"], cwd=build_dir)
    run(["cmake", "--build", ".", "--config", "Release", "--target", "benchmark"],
        cwd=build_dir)
    exe = build_dir / "bench" / "benchmark"
    if not exe.exists():
        raise RuntimeError("Benchmark executable not found")
    return exe

def parse_csv_output(text):
    results = {}
    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue

        parts = [p.strip() for p in line.split(",")]
        if parts[0] == "Function":
            continue
        if len(parts) < 4:
            continue

        func = parts[0]
        try:
            time_us = float(parts[1])
            mbps = float(parts[3])
        except ValueError:
            continue

        results[func] = {"time_us": time_us, "mbps": mbps}
    return results

def run_benchmarks(exe, runs):
    scenario_results = {s: {f: {"time_us": [], "mbps": []}
                            for f in FUNCTIONS}
                        for s in SCENARIOS}

    for scenario in SCENARIOS:
        print(f"  Running scenario: {scenario}")
        for _ in range(runs):
            out = run([str(exe), scenario])
            parsed = parse_csv_output(out)
            for func in FUNCTIONS:
                if func in parsed:
                    scenario_results[scenario][func]["time_us"].append(parsed[func]["time_us"])
                    scenario_results[scenario][func]["mbps"].append(parsed[func]["mbps"])

    medians = {}
    for scenario in SCENARIOS:
        medians[scenario] = {}
        for func in FUNCTIONS:
            vals = scenario_results[scenario][func]
            if vals["time_us"]:
                medians[scenario][func] = {
                    "time_us": statistics.median(vals["time_us"]),
                    "mbps": statistics.median(vals["mbps"])
                }
    return medians

def pct_change(old, new):
    if old == 0:
        return 0.0
    return (new - old) / old * 100.0

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("commit1")
    parser.add_argument("commit2")
    parser.add_argument("--runs", type=int, default=10,
                        help="Number of runs per scenario")
    args = parser.parse_args()

    commit1_sha = run(["git", "rev-parse", args.commit1]).strip()[:7]
    commit2_sha = run(["git", "rev-parse", args.commit2]).strip()[:7]

    print(f"Comparing commits:")
    print(f"  Commit1: {commit1_sha} ({args.commit1})")
    print(f"  Commit2: {commit2_sha} ({args.commit2})")
    print("")

    tempdirs = []

    # Build commit1
    print(f"Building {commit1_sha}...")
    src1 = checkout_commit(args.commit1)
    tempdirs.append(src1)
    exe1 = build_benchmark(src1)
    results1 = run_benchmarks(exe1, args.runs)

    # Build commit2
    print(f"Building {commit2_sha}...")
    src2 = checkout_commit(args.commit2)
    tempdirs.append(src2)
    exe2 = build_benchmark(src2)
    results2 = run_benchmarks(exe2, args.runs)

    # Print comparison
    print("=" * 80)
    print(f"Benchmark Comparison (median of {args.runs} runs): {commit1_sha} vs {commit2_sha}")
    print("=" * 80)

    for scenario in SCENARIOS:
        print(f"\nScenario: {scenario.upper()}")
        print("-" * 80)
        print(f"{'Function':<30} {'Commit1 MB/s':>15} {'Commit2 MB/s':>15} {'Change (%)':>12}  Result")
        print("-" * 80)

        for func in FUNCTIONS:
            if func not in results1[scenario] or func not in results2[scenario]:
                continue

            old = results1[scenario][func]["mbps"]
            new = results2[scenario][func]["mbps"]
            change = pct_change(old, new)

            if change > 2:
                verdict = "Commit2 is slower"
            elif change < -2:
                verdict = "Commit2 is faster"
            else:
                verdict = "Similar performance"

            print(f"{func:<30} {old:>15.2f} {new:>15.2f} {change:>12.2f}  {verdict}")

    print("\n" + "=" * 80)
    print("Interpretation:")
    print("  Change > +2%   → Commit2 is slower")
    print("  Change < -2%   → Commit2 is faster")
    print("  Otherwise      → Similar performance")
    print("=" * 80)

    # Cleanup
    for d in tempdirs:
        shutil.rmtree(d, ignore_errors=True)

if __name__ == "__main__":
    main()
