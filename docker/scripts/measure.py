#!/usr/bin/env python3
"""Paired A/B measurement with confidence intervals.

Runs A and B in ABBA order every round, keeps the per-round ratio and reports a
t-based 95% confidence interval of the speed-up. Paired ratios cancel the drift
of a shared/virtualized machine, which is why we do not compare raw means.
"""
import argparse
import json
import resource
import shutil
import statistics
import subprocess


def run(cmd, args):
    r0 = resource.getrusage(resource.RUSAGE_CHILDREN)
    prefix = ["taskset", "-c", "2"] if shutil.which("taskset") else []
    subprocess.run(prefix + ["./" + cmd] + args, stdout=subprocess.DEVNULL, check=True)
    r1 = resource.getrusage(resource.RUSAGE_CHILDREN)
    return (r1.ru_utime - r0.ru_utime) + (r1.ru_stime - r0.ru_stime)


def t_ci(x, t):
    m = statistics.mean(x)
    s = statistics.stdev(x)
    h = t * s / (len(x) ** 0.5)
    return m - h, m + h


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--a", required=True)
    ap.add_argument("--b", required=True)
    ap.add_argument("--iters", default="2000000")
    ap.add_argument("--rounds", type=int, default=25)
    ap.add_argument("--label-a", default=None)
    ap.add_argument("--label-b", default=None)
    args = ap.parse_args()

    # t(0.975, df=24) for 25 rounds; good enough for the default round count
    tval = 2.064 if args.rounds == 25 else 2.0

    run(args.a, [args.iters])
    run(args.b, [args.iters])

    ta, tb, ratios = [], [], []
    for r in range(args.rounds):
        if r % 2 == 0:
            a1, b1, b2, a2 = (run(args.a, [args.iters]), run(args.b, [args.iters]),
                              run(args.b, [args.iters]), run(args.a, [args.iters]))
        else:
            b1, a1, a2, b2 = (run(args.b, [args.iters]), run(args.a, [args.iters]),
                              run(args.a, [args.iters]), run(args.b, [args.iters]))
        a, b = (a1 + a2) / 2, (b1 + b2) / 2
        ta.append(a)
        tb.append(b)
        ratios.append(b / a)

    lo, hi = t_ci(ratios, tval)
    mean = statistics.mean(ratios)
    out = {
        "a": args.label_a or args.a,
        "b": args.label_b or args.b,
        "rounds": args.rounds,
        "cpu_ms_a": round(statistics.median(ta) * 1000, 1),
        "cpu_ms_b": round(statistics.median(tb) * 1000, 1),
        "speedup_pct": round((1 - mean) * 100, 2),
        "ci95_pct": [round((1 - hi) * 100, 2), round((1 - lo) * 100, 2)],
        "significant": bool(hi < 1.0),
    }
    print(json.dumps(out))


if __name__ == "__main__":
    main()
