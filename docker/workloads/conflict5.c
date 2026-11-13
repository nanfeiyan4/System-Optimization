#include <stdio.h>
#include <stdlib.h>
__attribute__((noinline, aligned(4096))) long f0(long s) {
#pragma clang loop unroll(full)
  for (int i = 0; i < 44; i++) {
    switch (s & 3) {
    case 0: s += 3; break;
    case 1: s ^= (s >> 2); break;
    case 2: s -= 5; break;
    default: s = s * 3 + 1; break;
    }
  }
  return s;
}
__attribute__((noinline, aligned(4096))) long f1(long s) {
#pragma clang loop unroll(full)
  for (int i = 0; i < 44; i++) {
    switch (s & 3) {
    case 0: s += 3; break;
    case 1: s ^= (s >> 2); break;
    case 2: s -= 5; break;
    default: s = s * 3 + 1; break;
    }
  }
  return s;
}
__attribute__((noinline, aligned(4096))) long f2(long s) {
#pragma clang loop unroll(full)
  for (int i = 0; i < 44; i++) {
    switch (s & 3) {
    case 0: s += 3; break;
    case 1: s ^= (s >> 2); break;
    case 2: s -= 5; break;
    default: s = s * 3 + 1; break;
    }
  }
  return s;
}
__attribute__((noinline, aligned(4096))) long f3(long s) {
#pragma clang loop unroll(full)
  for (int i = 0; i < 44; i++) {
    switch (s & 3) {
    case 0: s += 3; break;
    case 1: s ^= (s >> 2); break;
    case 2: s -= 5; break;
    default: s = s * 3 + 1; break;
    }
  }
  return s;
}
__attribute__((noinline, aligned(4096))) long f4(long s) {
#pragma clang loop unroll(full)
  for (int i = 0; i < 44; i++) {
    switch (s & 3) {
    case 0: s += 3; break;
    case 1: s ^= (s >> 2); break;
    case 2: s -= 5; break;
    default: s = s * 3 + 1; break;
    }
  }
  return s;
}
__attribute__((noinline, aligned(4096))) long f5(long s) {
#pragma clang loop unroll(full)
  for (int i = 0; i < 44; i++) {
    switch (s & 3) {
    case 0: s += 3; break;
    case 1: s ^= (s >> 2); break;
    case 2: s -= 5; break;
    default: s = s * 3 + 1; break;
    }
  }
  return s;
}
__attribute__((noinline, aligned(4096))) long f6(long s) {
#pragma clang loop unroll(full)
  for (int i = 0; i < 44; i++) {
    switch (s & 3) {
    case 0: s += 3; break;
    case 1: s ^= (s >> 2); break;
    case 2: s -= 5; break;
    default: s = s * 3 + 1; break;
    }
  }
  return s;
}
__attribute__((noinline, aligned(4096))) long f7(long s) {
#pragma clang loop unroll(full)
  for (int i = 0; i < 44; i++) {
    switch (s & 3) {
    case 0: s += 3; break;
    case 1: s ^= (s >> 2); break;
    case 2: s -= 5; break;
    default: s = s * 3 + 1; break;
    }
  }
  return s;
}
__attribute__((noinline, aligned(4096))) long f8(long s) {
#pragma clang loop unroll(full)
  for (int i = 0; i < 44; i++) {
    switch (s & 3) {
    case 0: s += 3; break;
    case 1: s ^= (s >> 2); break;
    case 2: s -= 5; break;
    default: s = s * 3 + 1; break;
    }
  }
  return s;
}
__attribute__((noinline, aligned(4096))) long f9(long s) {
#pragma clang loop unroll(full)
  for (int i = 0; i < 44; i++) {
    switch (s & 3) {
    case 0: s += 3; break;
    case 1: s ^= (s >> 2); break;
    case 2: s -= 5; break;
    default: s = s * 3 + 1; break;
    }
  }
  return s;
}
int main(int argc, char **argv) {
  long rounds = argc > 1 ? atol(argv[1]) : 500000;
  long r = 0;
  for (long k = 0; k < rounds; k++) { r += f0(k); r += f1(k); r += f2(k); r += f3(k); r += f4(k); r += f5(k); r += f6(k); r += f7(k); r += f8(k); r += f9(k); }
  printf("%ld\n", r);
  return 0;
}
