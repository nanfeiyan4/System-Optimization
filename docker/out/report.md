# L1i set-conflict padding: experiment report

Generated 2026-11-13T08:44:09Z inside the container.

Both workloads use 10 hot functions, each `aligned(4096)`, with a
~2.3KB hot body and a total hot working set below the 32KB L1i size.
They differ in *how* the hot code is fetched:

* **sequential** - straight-line body, the instruction prefetcher hides
  the conflict misses, so the pass cannot help wall-clock time.
* **branchy** - unrolled `switch`, non-sequential fetch, every conflict
  miss stalls the front end.

```
LLVM (http://llvm.org/):
  LLVM version 24.0.0git
Ubuntu clang version 18.1.3 (1ubuntu1)
valgrind-3.22.0
cores: 8
```

## Workload: sequential

| build | I1 misses |
|---|---:|
| original binary | 49304 |
| BOLT (default packing) | 1865 |
| BOLT, forced 4KB alignment | 49103 |
| BOLT, forced 4KB alignment + `--icache-conflict-pad` | 4552 |

BOLT-INFO: cache-conflict-padding: considered 10 hot function(s), padded 7 with 11328 bytes; max hot lines per set 10 -> 9 (8-way)

| comparison | median CPU a | median CPU b | speed-up | 95% CI | significant |
|---|---:|---:|---:|---|---|
| BOLT, forced 4KB vs BOLT, forced 4KB + pad | 1439.6 ms | 1430.3 ms | **1.38%** | [-0.90%, 3.66%] | no |

## Workload: branchy

| build | I1 misses |
|---|---:|
| original binary | 36398 |
| BOLT (default packing) | 1816 |
| BOLT, forced 4KB alignment | 39991 |
| BOLT, forced 4KB alignment + `--icache-conflict-pad` | 1832 |

BOLT-INFO: cache-conflict-padding: considered 10 hot function(s), padded 9 with 14208 bytes; max hot lines per set 10 -> 8 (8-way)

| comparison | median CPU a | median CPU b | speed-up | 95% CI | significant |
|---|---:|---:|---:|---|---|
| BOLT, forced 4KB vs BOLT, forced 4KB + pad | 586.8 ms | 542.5 ms | **7.48%** | [5.92%, 9.03%] | yes |

## Conclusion

The simulated I1 miss reduction is large on both workloads, but only the
branchy one converts it into CPU time: with sequential fetch the
prefetcher hides the misses, which is exactly the applicability boundary
of this optimization.
