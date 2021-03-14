#pragma once
#include <cstdint>

#include "common.h"
#include "buffer2d.h"
#include "bitset2d.h"

namespace librl {

// raycast from [sx,sy] to [ex,ey] and return true if there is a collision
bool raycast(const int2 &s, const int2 &e, const bitset2d_t &map);

}  // namespace librl
