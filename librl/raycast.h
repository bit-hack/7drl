#pragma once
#include <cstdint>

namespace librl {

struct map_t;

// raycast from [sx,sy] to [ex,ey] and return true if there is a collision
bool raycast(
  const uint32_t sx, const uint32_t sy,
  const uint32_t ex, const uint32_t ey,
  const map_t &map);

}  // namespace librl
