# BOLT L1i set-conflict padding (系统优化课程作业)

在 LLVM 24.0.0git / BOLT 主线之上新增一个 pass：**`--icache-conflict-pad`**。
它把热函数在 4KB 页内做相位移位（NUL padding），以消除 L1 指令缓存
**组相联冲突缺失**（L1i 32KB/8-way/64B ⇒ 64 个 set，冲突条件是地址差为 4096 的整数倍）。

## 内容

| 路径 | 说明 |
|---|---|
| `bolt-cache-conflict.patch` | 代码补丁（6 文件，+499 行，含 lit 测试） |
| `DESIGN.md` | 设计文档：问题、算法、选项、无回退证明、测试、实测、局限性 |
| `docker/` | 可复现实验镜像：Dockerfile + 一键脚本 + 两个负载 |
| `docker/out/report.md` | 容器内实际跑出的实验报告 |

## 补丁内容

```
bolt/include/bolt/Passes/CacheConflictPadding.h   |  38 +
bolt/lib/Core/BinaryEmitter.cpp                   |   6 +-
bolt/lib/Passes/CMakeLists.txt                    |   1 +
bolt/lib/Passes/CacheConflictPadding.cpp          | 407 +
bolt/lib/Rewrite/BinaryPassManager.cpp            |   5 +
bolt/test/X86/icache-conflict-pad.c               |  43 +
```

## 选项

```
--icache-conflict-pad          总开关（默认关闭）
--icache-sets/-ways/-line      缓存几何（默认 64/8/64）
--icache-pad-budget            单函数 padding 上限（默认 2048）
--icache-conflict-topn         参与的热函数数（默认 32）
--icache-min-conflict-ratio    严重度闸门，% 容量（默认 5）
--icache-max-pad-pct           总 padding 上限，% 热代码（默认 100）
--icache-conflict-verbose      打印决策
```

## 实验结论

两个负载都是 10 个 `aligned(4096)` 热函数、热工作集 23KB < 32KB L1i，区别只在取指方式：

| 负载 | I1 misses（forced 4KB → +pad） | CPU 时间 |
|---|---|---|
| sequential（直行） | 49,103 → 4,552（−91%） | +1.38% [−0.90%, 3.66%] 不显著 |
| branchy（展开 switch） | 39,991 → 1,832（−95%） | **+7.48% [5.92%, 9.03%] 显著** |

**适用边界**：顺序取指时硬件预取会掩盖冲突缺失，收益为 0；分支密集、非顺序取指时
冲突缺失真实停顿，收益 7.5%。

## 复现

```bash
# 应用补丁并构建 llvm-bolt
git clone --depth 1 https://github.com/llvm/llvm-project.git
cd llvm-project && git apply /path/to/bolt-cache-conflict.patch
cmake -G Ninja -S llvm -B build -DCMAKE_BUILD_TYPE=Release \
      -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_ENABLE_PROJECTS=bolt \
      -DLLVM_ENABLE_ASSERTIONS=OFF -DLLVM_INCLUDE_TESTS=OFF -DLLVM_USE_LINKER=lld
ninja -C build llvm-bolt

# 或者直接用镜像跑完整实验
cd docker
docker build -t bolt-icache-conflict .
docker run --rm -v "$PWD/out:/work/out" bolt-icache-conflict
cat out/report.md
```

> 注意：LLVM 24 起 BOLT 是 `LLVM_ENABLE_PROJECTS` 里的可选项，缺了会报 `unknown target 'llvm-bolt'`。
