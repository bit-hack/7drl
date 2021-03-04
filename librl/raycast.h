#pragma once
#include <cstdint>

namespace librl {

struct buffer2d_t;

// raycast from [sx,sy] to [ex,ey] and return true if there is a collision
bool raycast(
  const uint32_t sx, const uint32_t sy,
  const uint32_t ex, const uint32_t ey,
  const uint8_t mask,
  const buffer2d_t &map);

}  // namespace librl
