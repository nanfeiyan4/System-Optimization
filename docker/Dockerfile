# Docker image for the BOLT L1i set-conflict padding experiment.
#
# It ships:
#   /opt/bolt/bin/llvm-bolt        patched llvm-bolt (LLVM 24.0.0git)
#   /opt/bolt/lib/libbolt_rt*.a    BOLT runtime libraries
#   /opt/bench/workloads           microbenchmark sources
#   /opt/bench/scripts             one-click experiment driver
#   /opt/bolt/patch                the patch used to build llvm-bolt
#
# Build:  docker build -t bolt-icache-conflict .
# Run:    docker run --rm bolt-icache-conflict
#         docker run --rm -v "$PWD/out:/work/out" bolt-icache-conflict
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
RUN sed -i 's|http://archive.ubuntu.com/ubuntu/|http://mirrors.aliyun.com/ubuntu/|g;             s|http://security.ubuntu.com/ubuntu/|http://mirrors.aliyun.com/ubuntu/|g;             s|https://archive.ubuntu.com/ubuntu/|http://mirrors.aliyun.com/ubuntu/|g'       /etc/apt/sources.list.d/ubuntu.sources 2>/dev/null || true
RUN apt-get update && apt-get install -y --no-install-recommends \
      clang \
      binutils \
      valgrind \
      python3 \
      ca-certificates \
      procps \
    && rm -rf /var/lib/apt/lists/*

COPY prebuilt/llvm-bolt /opt/bolt/bin/llvm-bolt
COPY prebuilt/perf2bolt /opt/bolt/bin/perf2bolt
COPY prebuilt/lib/ /opt/bolt/lib/
COPY workloads/ /opt/bench/workloads/
COPY scripts/ /opt/bench/scripts/
COPY patch/ /opt/bolt/patch/

ENV PATH=/opt/bolt/bin:$PATH \
    BOLT=/opt/bolt/bin/llvm-bolt \
    RT_INSTR=/opt/bolt/lib/libbolt_rt_instr.a \
    RT_HUGIFY=/opt/bolt/lib/libbolt_rt_hugify.a

RUN chmod +x /opt/bench/scripts/run_experiments.sh
WORKDIR /work
ENTRYPOINT ["/opt/bench/scripts/run_experiments.sh"]
