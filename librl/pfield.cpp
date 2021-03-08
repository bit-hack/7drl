#include "pfield.h"

namespace librl {

void pfield_t::update() {
  const auto &r = read();
  auto &w = write();

  for (uint32_t y = 1; y < r.height - 1; ++y) {
    for (uint32_t x = 1; x < r.width - 1; ++x) {

      if (map.get(x, y) == wall_tile) {
        w.get(x, y) = 0;
        continue;
      }

      const uint8_t v0 = r.get(x, y - 1);
      const uint8_t v1 = r.get(x, y + 1);
      const uint8_t v2 = r.get(x - 1, y);
      const uint8_t v3 = r.get(x + 1, y);
      const uint8_t v4 = r.get(x, y); // center

      const uint8_t o = max(max(max(v0, v1), max(v2, v3)), v4);

      w.get(x, y) = o ? o - 1 : 0;
    }
  }

  // toggle the state
  state ^= 1;
}

void pfield_t::diff(uint32_t x, uint32_t y, int32_t &dx, int32_t &dy) {
  const auto &r = read();

  assert(x > 0 && x < r.width  - 1);
  assert(y > 0 && y < r.height - 1);

  const uint8_t x0 = sample(x - 1, y);
  const uint8_t x1 = sample(x + 1, y);
  dx = librl::sign(int32_t(x1) - int32_t(x0));

  const uint8_t y0 = sample(x, y - 1);
  const uint8_t y1 = sample(x, y + 1);
  dy = librl::sign(int32_t(y1) - int32_t(y0));
}

void pfield_t::drop(uint32_t x, uint32_t y, uint8_t val) {

  // we have to drop into the read buffer so it doesnt get wiped out by the
  // next update.
  auto &r = read();

  assert(x > 0 && x < r.width - 1);
  assert(y > 0 && y < r.height - 1);

  r.get(x, y) = librl::max(r.get(x, y), val);
}

uint8_t pfield_t::sample(uint32_t x, uint32_t y) const {
  const auto &r = read();
  return r.get(x, y);
}

}  // namespace librl
