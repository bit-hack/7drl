#pragma once
#include <cstdint>

#include "common.h"

namespace librl {

struct buffer2d_t;

// raycast from [sx,sy] to [ex,ey] and return true if there is a collision
bool raycast(const int2 &s, const int2 &e, const uint8_t mask, const buffer2d_t &map);

}  // namespace librl
