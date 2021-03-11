#pragma once
#include <cstdint>

namespace librl {

inline uint64_t random(uint64_t &seed) {
  seed ^= seed << 13;
  seed ^= seed >> 7;
  seed ^= seed << 17;
  return seed;
}

inline uint64_t random(uint64_t &seed, uint64_t max) {
  if (max == 0) {
    return 0;
  }
  return random(seed) % max;
}

inline bool random_bool(uint64_t &seed) {
  return (random(seed) & 0x8000) == 0;
}

}  // namespace librl
