#pragma once
#include <cstdint>

namespace librl {

struct random_t {

  random_t(uint64_t s = 12345)
    : seed(s) {
  }

  bool get_bool() {
    return (next() & 0x8000) == 0;
  }

  int32_t get_int32() {
    return int32_t(next());
  }

  uint32_t get_uint32() {
    return uint32_t(next());
  }

protected:

  uint64_t next() {
    seed ^= seed >> 12;
    seed ^= seed << 25;
    seed ^= seed >> 27;
    return seed * 0x2545F4914F6CDD1dllu;
  }

  uint64_t seed;
};

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
