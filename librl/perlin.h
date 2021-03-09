#pragma once
#include <cstdint>

namespace librl {

struct perlin_t {

  perlin_t(uint32_t seed)
    : seed(seed)
  {
  }

  uint32_t sample2d(uint32_t x, uint32_t y) {

    uint32_t sum = 0;

    for (int i = 0; i < 8; ++i) {

      const uint32_t q00 = 0xff & hash( ((x >> i) + 0) + ((y >> i) + 0) * 0x8000);
      const uint32_t q10 = 0xff & hash( ((x >> i) + 1) + ((y >> i) + 0) * 0x8000);
      const uint32_t q01 = 0xff & hash( ((x >> i) + 0) + ((y >> i) + 1) * 0x8000);
      const uint32_t q11 = 0xff & hash( ((x >> i) + 1) + ((y >> i) + 1) * 0x8000);

      const uint8_t ix = x >> i;
      const uint8_t iy = x >> i;

      const uint32_t x0 = lerp(q00, q10, ix & 0xff);
      const uint32_t x1 = lerp(q01, q11, ix & 0xff);
      const uint32_t y  = lerp(x0,  x1,  iy & 0xff);

      sum += (y * (i + 1)) / 8;
    }

    return sum;
  }

  static uint32_t lerp(uint32_t a, uint32_t b, uint32_t z) {
    return ((a * z) + (b * (255 - z))) >> 8;
  }

  uint32_t hash(uint32_t x) {
    x = (x ^ 61) ^ (x >> 16) ^ seed;
    x *= 9;
    x = x ^ (x >> 4);
    x *= 0x27d4eb2d;
    x = x ^ (x >> 15);
    return x;
  }

  const uint32_t seed;
};

}  // namespace librl
