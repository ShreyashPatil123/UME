#!/usr/bin/env python3
"""Performance gate checker for UME CI.

Validates benchmark results against defined performance gates.
Exits with code 1 if any gate is exceeded beyond tolerance.

Usage:
    python check-perf-gates.py --results results.json --baselines ci/performance-gates.json
    python check-perf-gates.py --results results.json --baselines ci/performance-gates.json --tolerance 0.10
"""

import argparse
import json
import sys
from pathlib import Path


def load_json(path: Path) -> dict:
    """Load and parse a JSON file."""
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def check_gates(
    results: dict,
    gates_config: dict,
    tolerance_override: float | None = None,
) -> list[dict]:
    """Check benchmark results against performance gates.

    Args:
        results: Benchmark results (metric_name -> measured_value).
        gates_config: Performance gates configuration.
        tolerance_override: Optional override for both latency and throughput tolerance.

    Returns:
        List of failed gates with details.
    """
    tolerance_latency = tolerance_override or gates_config.get("tolerance_latency", 0.10)
    tolerance_throughput = tolerance_override or gates_config.get("tolerance_throughput", 0.15)

    failures: list[dict] = []

    for gate in gates_config.get("gates", []):
        gate_id = gate["id"]
        gate_name = gate["name"]
        metric = gate["metric"]
        direction = gate["direction"]
        threshold = gate["threshold"]
        unit = gate.get("unit", "")

        measured = results.get(metric)
        if measured is None:
            # Metric not found in results — skip (may not be implemented yet)
            continue

        tolerance = tolerance_throughput if direction == "max" else tolerance_latency

        if direction == "max":
            # Higher is better (throughput). Fail if measured < threshold * (1 - tolerance)
            limit = threshold * (1.0 - tolerance)
            passed = measured >= limit
        else:
            # Lower is better (latency). Fail if measured > threshold * (1 + tolerance)
            limit = threshold * (1.0 + tolerance)
            passed = measured <= limit

        if not passed:
            failures.append(
                {
                    "id": gate_id,
                    "name": gate_name,
                    "metric": metric,
                    "threshold": threshold,
                    "measured": measured,
                    "limit": limit,
                    "direction": direction,
                    "unit": unit,
                    "tolerance": tolerance,
                }
            )

    return failures


def main() -> int:
    """Main entry point."""
    parser = argparse.ArgumentParser(description="UME Performance Gate Checker")
    parser.add_argument(
        "--results",
        type=Path,
        required=True,
        help="Path to benchmark results JSON file",
    )
    parser.add_argument(
        "--baselines",
        type=Path,
        required=True,
        help="Path to performance gates configuration JSON",
    )
    parser.add_argument(
        "--tolerance",
        type=float,
        default=None,
        help="Override tolerance for all gates (0.0 to 1.0)",
    )
    args = parser.parse_args()

    if not args.results.exists():
        print(f"ERROR: Results file not found: {args.results}", file=sys.stderr)
        return 1

    if not args.baselines.exists():
        print(f"ERROR: Gates file not found: {args.baselines}", file=sys.stderr)
        return 1

    results = load_json(args.results)
    gates_config = load_json(args.baselines)

    failures = check_gates(results, gates_config, args.tolerance)

    if not failures:
        gates_checked = sum(
            1 for g in gates_config.get("gates", []) if g["metric"] in results
        )
        print(f"✅ All {gates_checked} performance gates PASSED.")
        return 0

    print(f"❌ {len(failures)} performance gate(s) FAILED:\n", file=sys.stderr)
    for f in failures:
        direction_label = "≥" if f["direction"] == "max" else "≤"
        print(
            f"  GATE {f['id']}: {f['name']}\n"
            f"    Threshold: {direction_label} {f['threshold']} {f['unit']}\n"
            f"    Measured:  {f['measured']} {f['unit']}\n"
            f"    Limit:     {f['limit']:.1f} {f['unit']} (with {f['tolerance']*100:.0f}% tolerance)\n",
            file=sys.stderr,
        )

    return 1


if __name__ == "__main__":
    sys.exit(main())
