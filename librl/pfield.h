#pragma once
#include "buffer2d.h"

namespace librl {

struct pfield_t {

  pfield_t(buffer2d_t &m, uint8_t tile)
    : wall_tile(tile)
    , map(m)
    , state(0)
    , a(new buffer2d_t(m.width, m.height))
    , b(new buffer2d_t(m.width, m.height))
  {
    a->clear(0);
    b->clear(0);
  }

  const buffer2d_t &read() const {
    return *(state ? a : b);
  }

  buffer2d_t &read() {
    return *(state ? a : b);
  }

  buffer2d_t &write() {
    return *(state ? b : a);
  }

  void update();

  void diff(uint32_t x, uint32_t y, int32_t &dx, int32_t &dy);

  void drop(uint32_t x, uint32_t y, uint8_t);

protected:

  const uint8_t wall_tile;

  uint8_t sample(uint32_t x, uint32_t y) const;

  buffer2d_t &map;
  uint32_t state;
  std::unique_ptr<buffer2d_t> a;
  std::unique_ptr<buffer2d_t> b;
};

}  // namespace librl
