#include "raycast.h"
#include "map.h"
#include <utility>


namespace {

template <typename type_t>
type_t abs(const type_t x) {
  return (x < 0) ? -x : x;
}

}  // namespace {}

namespace librl{

struct map_t;

bool raycast(
  const uint32_t sx, const uint32_t sy,
  const uint32_t ex, const uint32_t ey,
  const map_t &map) {

  const int32_t incrementX = (ex > sx) ? 1 : -1;
  const int32_t incrementY = (ey > sy) ? 1 : -1;

  int32_t dx = abs<int32_t>(int32_t(ex) - int32_t(sx));
  int32_t dy = abs<int32_t>(int32_t(ey) - int32_t(sy));
  int32_t x = sx;
  int32_t y = sy;
  int32_t steps = 1 + dx + dy;
  int32_t error = dx - dy;
  dx *= 2;
  dy *= 2;

  while (steps--) {
    if (map.is_solid(x, y)) {
      // collision
      return false;
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
