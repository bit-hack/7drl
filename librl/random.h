#pragma once
#include <cstdint>


inline uint64_t random(uint64_t &seed) {
  seed ^= seed << 13;
  seed ^= seed >> 7;
  seed ^= seed << 17;
  return seed;
}
