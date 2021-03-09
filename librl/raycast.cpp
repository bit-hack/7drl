#include "raycast.h"
#include "buffer2d.h"
#include "common.h"

namespace librl {

bool raycast(const int2 &s, const int2 &e, const uint8_t mask, const buffer2d_u8_t &map) {

  const int32_t incrementX = (e.x > s.x) ? 1 : -1;
  const int32_t incrementY = (e.y > s.y) ? 1 : -1;

  int32_t dx = abs<int32_t>(int32_t(e.x) - int32_t(s.x));
  int32_t dy = abs<int32_t>(int32_t(e.y) - int32_t(s.y));
  int32_t x = s.x;
  int32_t y = s.y;
  int32_t steps = 1 + dx + dy;
  int32_t error = dx - dy;
  dx *= 2;
  dy *= 2;

  while (steps--) {
    if (map.get(x, y) & mask) {
      // collision
      return e.x == x && e.y == y;
    }
    if (error > 0) {
      x += incrementX;
      error -= dy;
    }
    else {
      y += incrementY;
      error += dx;
    }
  }
  // no collision
  return true;
}

}  // namespace librl
