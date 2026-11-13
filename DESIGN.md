# L1i set-conflict aware padding for BOLT (--icache-conflict-pad)

## Problem
L1i on current Intel cores is 32KB / 8-way / 64B => 64 sets.
set(addr) = (addr >> 6) & 63 = (addr % 4096) >> 6, so two hot blocks conflict
iff their addresses differ by a multiple of 4096. BOLT layout cost models
(ext-tsp, hfsort+, --print-cache-metrics) are all distance based and never look
at set indices.

With default packing BOLT de-correlates page offsets and the problem mostly
disappears (measured 95,379 -> 1,835 I1 misses). But when hot functions are
forced to a coarse alignment (--align-functions=4096 plus
--align-functions-max-bytes=4096, a configuration used for iTLB/hugepage
friendly layouts), every function starts at page offset 0 and the hot working
set piles up on the same sets: 95,378 I1 misses for a 29.5KB working set that
fits in L1i.

## Design
New pass CacheConflictPadding (bolt/lib/Passes/CacheConflictPadding.cpp),
registered after AlignerPass and before FinalizeFunctions.
1. Replay the emitter order (BC.getOutputBinaryFunctions()) and estimate the
   output offset of every function: min alignment, then --align-functions
   (honoring --align-functions-max-bytes), then the padding already selected
   for earlier functions.
2. Select hot functions: count >= MaxCount/10, at most --icache-conflict-topn.
3. Count hot lines per set. Objective: sum over sets of max(0, lines-ways)^2,
   exactly zero when the hot working set fits.
4. Phase 1: spread the functions uniformly over the 4KB page by rank.
   Phase 2: coordinate descent over all admissible phases (4 sweeps).
5. Enforce --icache-max-pad-pct by re-spreading inside the allowed range
   (scaling every phase instead collapses the layout).
6. Emit the phase as NUL padding before the function symbol, reusing the
   emitter's pad-funcs-before path (padding is emitted after alignment and
   before the symbol, so it shifts the function by exactly that many bytes).

## Options (all in the "BOLT optimization options" category)
--icache-conflict-pad          master switch, default off
--icache-sets/-ways/-line      cache geometry, default 64/8/64
--icache-pad-budget            per-function padding cap, default 2048
--icache-conflict-topn         number of hot functions considered, default 32
--icache-min-conflict-ratio    severity gate, percent of capacity, default 5
--icache-max-pad-pct           total padding cap, percent of hot code, default 100
--icache-conflict-verbose      print decisions

## No-regression
* Switch off: pass returns immediately, PaddingBefore stays empty, emitter
  unchanged.
* No profile, no hot functions, below the severity gate: not a single byte is
  emitted.
* Whole-file hashes are NOT stable across BOLT runs (.build-id/relaxation), so
  the property is checked on the .text section. Verified identical for:
  no profile, and profile but no conflict. Legacy --pad-funcs-before still
  changes .text, so the emitter refactor is safe.

## Testing
* New lit test bolt/test/X86/icache-conflict-pad.c: compiles a small program,
  checks that --icache-conflict-pad leaves .text byte-identical without a
  profile, that the pass reports "not enough hot functions", and that all new
  options show up in --help.
* Full suite (`ninja check-bolt`): 790 tests, 277 unsupported, 502 passed,
  11 failed. The same 11 tests fail on a clean tree with the patch reverted and
  rebuilt (dwarf/dwp/relr/segment-layout tests that need a different toolchain),
  so they are pre-existing and unrelated.

## Measured (Alibaba Cloud ECS, Xeon Platinum, 32KB 8-way L1i, no vPMU)

Both workloads: 10 hot functions, ~2.3KB body each, aligned(4096), total hot
working set 23KB < 32KB L1i. They differ in how the hot code is fetched.

| workload | build | I1 misses (cachegrind) | CPU time vs forced-4KB |
|---|---|---|---|
| sequential (straight line) | forced 4KB | 49,103 | - |
| sequential + pad | | 4,552 (-91%) | +1.38% [-0.90%, 3.66%], not significant |
| branchy (unrolled switch) | forced 4KB | 39,991 | - |
| branchy + pad | | 1,832 (-95%) | **+7.48% [5.92%, 9.03%], significant** |

Padding cost: 11-14KB for ~23KB of hot code (about 50%), configurable through
--icache-max-pad-pct.

Conclusion: the miss reduction is deterministic under cachegrind, but it only
becomes CPU time on workloads with non-sequential fetch. With straight-line
hot code the instruction prefetcher hides the misses and the pass correctly
detects that padding cannot pay for itself.

## Limitations
* Hardware counters are unavailable on this VM, so miss counts come from
  cachegrind simulation; wall-clock gains are small and need CPU-time metrics.
* Spreading N functions over a page costs Range*(N-1)/2 bytes; with a small
  --icache-max-pad-pct the pass cannot fully remove severe conflicts.
* Split functions are skipped (padding is emitted per fragment).
* Only useful when alignment is forced; with default packing the pass detects
  ~no conflict and does nothing (verified).
