# Real workload check: SQLite 3.37.2

The microbenchmarks are adversarial by construction. This directory records the
same experiment on a real program to establish the "ineffective" side of the
applicability boundary.

## Build (Ubuntu 22.04)

```bash
# sqlite.org is often unreachable from CN clouds, so use the distro source
echo "deb-src http://mirrors.aliyun.com/ubuntu jammy main universe" > /etc/apt/sources.list.d/src.list
apt-get update && apt-get install -y tcl
mkdir -p /root/realwork && cd /root/realwork
apt-get source sqlite3            # -> sqlite3-3.37.2/
cd sqlite3-3.37.2
rm -f shell.c                     # regenerated below with tclsh
./configure CFLAGS="-O2" LDFLAGS="-Wl,--emit-relocs" --disable-shared
make -j8 sqlite3                  # binary carries relocations for BOLT
```

`--emit-relocs` is mandatory: without it BOLT cannot move functions and the
whole experiment is meaningless.

## Workload

`setup.sql` builds a 40k row table with two indexes plus an 8k row joined table.
`gen_queries.py` emits 3000 mixed statements (index range scans, point lookups,
aggregates, two-table joins, LIKE prefixes). 300 statements (~1s) are used for
timing and 40 statements (~0.2s) for cachegrind.

```bash
sqlite3 db.sqlite < setup.sql
sqlite3 db.sqlite < queries.sql > /dev/null    # 3000 stmts, ~14s
```

## Result (BOLT, forced 4KB alignment)

| build | I1 misses (40 stmts) | size |
|---|---:|---:|
| BOLT, default packing | 51,135 | 2,155,696 |
| BOLT, `--align-functions=4096 --align-functions-max-bytes=4096` | 52,082 | 2,201,024 |
| the same + `--icache-conflict-pad` | 52,082 | 2,201,056 |

The pass prints:

```
BOLT-INFO: cache-conflict-padding: L1i over-subscription 0.000000e+00%
(max 2 hot lines per set, 8-way) is below the threshold, nothing to do
```

and the `.text` section of the two `forced 4KB` builds is byte-identical
(`sha256 68a7c152…`). In other words enabling the option costs nothing here.

Timing, 20 paired ABBA rounds (CPU time): default packing vs forced 4KB is
-0.56%, 95% CI [-2.58%, +1.46%] - not significant, while the binary grows 8%.

## Why there is no conflict

438 of 2050 functions (21.4%) have profile, and they are spread over the whole
binary. Each hot function is small, so at most 2 hot lines map to any L1i set,
far below the 8-way associativity. Real programs do not have the
"many equal-sized page-aligned hot bodies" shape that the microbenchmark
constructs on purpose.
