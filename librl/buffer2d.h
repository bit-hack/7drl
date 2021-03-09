#pragma once
#include <cstdint>
#include <cassert>
#include <memory>

#include "common.h"

namespace librl {

template <typename type_t>
struct buffer2d_t {

  typedef type_t type;

  buffer2d_t(const uint32_t w, const uint32_t h)
    : width(w)
    , height(h)
    , data(new type_t[w * h])
  {
  }

  buffer2d_t(const buffer2d_t &m)
    : width(m.width)
    , height(m.height)
    , data(new type_t[m.width * m.height])
  {
    memcpy(data.get(), m.data.get(), width * height);
  }

  type_t &get(uint32_t x, uint32_t y) {
    assert(x < width && y < height);
    return data[x + y * width];
  }

  const type_t &get(uint32_t x, uint32_t y) const {
    assert(x < width && y < height);
    return data[x + y * width];
  }

  type_t &get(const int2 &p) {
    return get(p.x, p.y);
  }

  const type_t &get(const int2 &p) const {
    return get(p.x, p.y);
  }

  void clear(type_t val) {
    for (uint32_t i = 0; i < width * height; ++i) {
      data[i] = val;
    }
  }

  const uint32_t width, height;

protected:
  std::unique_ptr<type_t[]> data;
};

typedef buffer2d_t<uint8_t>  buffer2d_u8_t;
typedef buffer2d_t<uint32_t> buffer2d_u32_t;

}  // namespace librl
