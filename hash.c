#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint64_t hash(const char *key) {
  uint64_t hash = FNV_OFFSET;
  for (const char *p = key; *p; p++) {
    hash ^= (uint64_t)(unsigned char)(*p);
    hash *= FNV_PRIME;
  }
  return hash;
}

int main() {
  uint64_t h1 = hash("a");  //  97
  uint64_t h2 = hash("b");  //  98
  uint64_t h3 = hash("c");  //  99
  uint64_t h4 = hash("d");  // 100
  uint64_t h5 = hash("e");  // 101
  uint64_t h6 = hash("f");  // 102
  uint64_t h7 = hash("g");  // 103
  uint64_t h8 = hash("h");  // 104
  uint64_t h9 = hash("i");  // 105
  uint64_t h10 = hash("j"); // 106
  uint64_t h11 = hash("k"); // 107
  uint64_t h12 = hash("l"); // 108
  uint64_t h13 = hash("m"); // 109
  uint64_t h14 = hash("n"); // 110
  uint64_t h15 = hash("o"); // 111
  uint64_t h16 = hash("p"); // 112
  uint64_t h17 = hash("q"); // 113
  uint64_t h18 = hash("r"); // 114
  uint64_t h19 = hash("s"); // 115
  uint64_t h20 = hash("t"); // 116
  uint64_t h21 = hash("u"); // 117
  uint64_t h22 = hash("v"); // 118
  uint64_t h23 = hash("w"); // 119
  uint64_t h24 = hash("x"); // 120
  uint64_t h25 = hash("y"); // 121
  uint64_t h26 = hash("z"); // 122

  size_t index;

  index = (size_t)(h1 & (uint64_t)15);
  printf("a: %zu\n", index);
  index = (size_t)(h2 & (uint64_t)15);
  printf("b: %zu\n", index);
  index = (size_t)(h3 & (uint64_t)15);
  printf("c: %zu\n", index);
  index = (size_t)(h4 & (uint64_t)15);
  printf("d: %zu\n", index);
  index = (size_t)(h5 & (uint64_t)15);
  printf("e: %zu\n", index);
  index = (size_t)(h6 & (uint64_t)15);
  printf("f: %zu\n", index);
  index = (size_t)(h7 & (uint64_t)15);
  printf("g: %zu\n", index);
  index = (size_t)(h8 & (uint64_t)15);
  printf("h: %zu\n", index);
  index = (size_t)(h9 & (uint64_t)15);
  printf("i: %zu\n", index);
  index = (size_t)(h10 & (uint64_t)15);
  printf("j: %zu\n", index);
  index = (size_t)(h11 & (uint64_t)15);
  printf("k: %zu\n", index);
  index = (size_t)(h12 & (uint64_t)15);
  printf("l: %zu\n", index);
  index = (size_t)(h13 & (uint64_t)15);
  printf("m: %zu\n", index);
  index = (size_t)(h14 & (uint64_t)15);
  printf("n: %zu\n", index);
  index = (size_t)(h15 & (uint64_t)15);
  printf("o: %zu\n", index);
  index = (size_t)(h16 & (uint64_t)15);
  printf("p: %zu\n", index);
  index = (size_t)(h17 & (uint64_t)15);
  printf("q: %zu\n", index);
  index = (size_t)(h18 & (uint64_t)15);
  printf("r: %zu\n", index);
  index = (size_t)(h19 & (uint64_t)15);
  printf("s: %zu\n", index);
  index = (size_t)(h20 & (uint64_t)15);
  printf("t: %zu\n", index);
  index = (size_t)(h21 & (uint64_t)15);
  printf("u: %zu\n", index);
  index = (size_t)(h22 & (uint64_t)15);
  printf("v: %zu\n", index);
  index = (size_t)(h23 & (uint64_t)15);
  printf("w: %zu\n", index);
  index = (size_t)(h24 & (uint64_t)15);
  printf("x: %zu\n", index);
  index = (size_t)(h25 & (uint64_t)15);
  printf("y: %zu\n", index);
  index = (size_t)(h26 & (uint64_t)15);
  printf("z: %zu\n", index);
}
