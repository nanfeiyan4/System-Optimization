#include <stdio.h>
#include <stdlib.h>

#define CH(x, v) v = v * 3 + (x); v ^= (v >> 5);
#define RND(K) CH((K)+1, a0) CH((K)+2, a1) CH((K)+3, a2) CH((K)+4, a3) \
               CH((K)+5, a4) CH((K)+6, a5) CH((K)+7, a6) CH((K)+8, a7)

#define DEF(N) \
__attribute__((noinline, aligned(4096))) long f##N(long s) { \
  long a0 = s ^ (N), a1 = s + 1, a2 = s + 2, a3 = s + 3; \
  long a4 = s + 4, a5 = s + 5, a6 = s + 6, a7 = s + 7; \
  RND(1) RND(9) RND(17) RND(25) RND(33) RND(41) RND(49) RND(57) RND(65) RND(73) RND(81) RND(89) RND(97) RND(105) RND(113) RND(121) RND(129) RND(137) RND(145) RND(153)  \
  return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7; \
}
DEF(0) DEF(1) DEF(2) DEF(3) DEF(4) DEF(5) DEF(6) DEF(7) DEF(8) DEF(9)

int main(int argc, char **argv) {
  long rounds = argc > 1 ? atol(argv[1]) : 1000000;
  long r = 0;
  for (long k = 0; k < rounds; k++) {
    r += f0(k); r += f1(k); r += f2(k); r += f3(k); r += f4(k);
    r += f5(k); r += f6(k); r += f7(k); r += f8(k); r += f9(k);
  }
  printf("%ld\n", r);
  return 0;
}
