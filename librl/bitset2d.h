#pragma once
#include <memory>
#include <cstdint>
#include <cassert>

#include "common.h"

namespace librl {

struct bitset2d_t {

  bitset2d_t(int32_t w, int32_t h)
    : width(w)
    , height(h)
    , data(new uint8_t[ (w * h) / 8 ])
  {
    clear();
  }

  bool get(const int2 &p) const {
    assert(p.x >= 0 && p.x < width);
    assert(p.y >= 0 && p.y < height);
    const uint32_t i = p.x + (p.y * width);
    const uint32_t b = i % 8;
    return 0 != (data[i / 8] & (1 << b));
  }

  void set(const int2 &p) {
    assert(p.x >= 0 && p.x < width);
    assert(p.y >= 0 && p.y < height);
    const uint32_t i = p.x + (p.y * width);
    const uint32_t b = i % 8;
    data[i / 8] |= 1 << b;
  }

  void clear(const int2 &p) {
    assert(p.x >= 0 && p.x < width);
    assert(p.y >= 0 && p.y < height);
    const uint32_t i = p.x + (p.y * width);
    const uint32_t b = i % 8;
    data[i / 8] &= ~(1 << b);
  }

  void not(const int2 &p) {
    assert(p.x >= 0 && p.x < width);
    assert(p.y >= 0 && p.y < height);
    const uint32_t i = p.x + (p.y * width);
    const uint32_t b = i % 8;
    data[i / 8] ^= 1 << b;
  }

  void clear() {
    for (int32_t i = 0; i < (width * height) / 8; ++i) {
      data[i] = 0;
    }
  }

  void fill() {
    for (int32_t i = 0; i < (width * height) / 8; ++i) {
      data[i] = 0xff;
    }
  }

  const int32_t width;
  const int32_t height;

protected:
  std::unique_ptr<uint8_t[]> data;
};

}  // namespace librl
