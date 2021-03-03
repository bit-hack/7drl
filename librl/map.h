#pragma once
#include <cstdint>
#include <cassert>
#include <memory>


namespace librl {

struct map_t {
  
  enum {
    mask_solid = 0x80,
  };

  map_t(const uint32_t w, const uint32_t h)
    : width(w)
    , height(h)
    , data(new uint8_t[w * h])
  {
  }

  map_t(const map_t &m)
    : width(m.width)
    , height(m.height)
    , data(new uint8_t[m.width * m.height])
  {
    memcpy(data.get(), m.data.get(), width * height);
  }

  uint8_t &get(uint32_t x, uint32_t y) {
    assert(x < width && y < height);
    return data[x + y * width];
  }

  const uint8_t &get(uint32_t x, uint32_t y) const {
    assert(x < width && y < height);
    return data[x + y * width];
  }

  bool is_solid(uint32_t x, uint32_t y) const {
    return get(x, y) & mask_solid;
  }

  void clear(uint8_t tile) {
    memset(data.get(), tile, width * height);
  }

  const uint32_t width, height;

protected:
  std::unique_ptr<uint8_t []> data;
};

}  // namespace librl
