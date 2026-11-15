# Third real workload: CPython 3.11.9

The third shape: a large binary whose hot code is dominated by one huge
interpreter function (`_PyEval_EvalFrameDefault`, tens of KB) plus many small
helpers. If a large hot region were going to overload single L1i sets, it would
be one like this.

## Build

```bash
curl -sL -o cpython.tar.gz https://github.com/python/cpython/archive/refs/tags/v3.11.9.tar.gz
tar xzf cpython.tar.gz && cd cpython-3.11.9
./configure CFLAGS="-O2" LDFLAGS="-Wl,--emit-relocs" \
            --without-ensurepip --disable-test-modules
make -j8                      # ~5 min on 8 cores, produces a 41 MB ./python
```

## Workload

`bench.py`: 300k iterations of dict churn plus 60k-element list/string
splitting and filtering (about 140 ms per run). Profiled by running the
BOLT-instrumented interpreter on it (1.5 MB of profile data).

## Result (BOLT, forced 4KB alignment)

| build | size |
|---|---:|
| original `./python` | 41,246,784 |
| BOLT, default packing | 7,936,560 |
| BOLT, forced 4KB alignment | 8,018,528 |
| the same + `--icache-conflict-pad` | 8,018,560 |

The pass reports

```
cache-conflict-padding: L1i over-subscription 0.000000e+00%
(max 4 hot lines per set, 8-way) is below the threshold, nothing to do
```

and the `.text` of the two forced-4KB builds is byte-identical
(`sha256 462dfa72...`), so the option is a no-op. Forced alignment itself only
costs 1% of size here, because few functions are hot enough to be worth
aligning by the aligner heuristics.

No timing comparison is reported: the two binaries are identical in `.text`,
so any difference would be measurement noise.

## Why a giant hot function does not create conflicts

A single function larger than 4KB wraps around all 64 sets as it executes, so
it spreads its lines evenly instead of piling them up. Conflicts need *many*
hot functions that all start at the same page offset - the shape the
microbenchmark constructs on purpose.
