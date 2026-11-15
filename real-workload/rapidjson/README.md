# Second real workload: RapidJSON (template heavy)

Different shape from SQLite: a C++ program where `-O2` inlining plus template
instantiation produces many *similarly sized* functions, and forcing 4KB
alignment puts all of them at page offset 0. If any real program hits the
conflict pattern, it should be one like this.

## Build

```bash
git clone --depth 1 https://github.com/Tencent/rapidjson.git
g++ -O2 -Wl,--emit-relocs -I rapidjson/include -o bench bench.cpp
```

## Workload

`bench 2000 300` parses a 2000-object JSON document (nested objects, arrays,
numbers, bools) and serialises it back, 300 times (~0.65s).

## Result (BOLT, forced 4KB alignment)

| build | I1 misses (3 rounds) | size |
|---|---:|---:|
| BOLT, default packing | 2,170 | 122,976 |
| BOLT, forced 4KB alignment | 2,226 | 131,440 |
| the same + `--icache-conflict-pad` | 2,226 | 131,456 |

The pass reports

```
cache-conflict-padding: L1i over-subscription 0.000000e+00%
(max 5 hot lines per set, 8-way) is below the threshold, nothing to do
```

and the `.text` of the two forced-4KB builds is byte-identical
(`sha256 b84a786b...`).

Timing, 15 paired ABBA rounds (CPU time): default packing vs forced 4KB is
-1.64%, 95% CI [-4.44%, +1.15%] - not significant, while the binary grows 37%.

## Reading

Even this template-heavy program only reaches 5 hot lines per set. The
microbenchmark reaches 10 because it builds ten equal-sized, page-aligned hot
bodies executed in a tight round-robin - a shape real code does not produce by
accident. The cost of forcing 4KB alignment (size, and no measurable gain) is
however real, which is exactly why the pass refuses to add padding when the
estimated over-subscription is zero.
