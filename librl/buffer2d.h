#pragma once
#include <cstdint>
#include <cassert>
#include <memory>


namespace librl {

struct buffer2d_t {

  typedef uint8_t type_t;

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

  void clear(uint8_t tile) {
    memset(data.get(), tile, width * height);
  }

  const uint32_t width, height;

protected:
  std::unique_ptr<type_t[]> data;
};

}  // namespace librl
