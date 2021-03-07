#pragma once
#include <cstdint>

namespace librl {

template <typename type_t>
static inline type_t abs(const type_t x) {
  return (x < 0) ? -x : x;
}

template <typename type_t>
static inline type_t min(const type_t x, const type_t y) {
  return (x < y) ? x : y;
}

template <typename type_t>
static inline type_t max(const type_t x, const type_t y) {
  return (x > y) ? x : y;
}

template <typename type_t>
static inline type_t clamp(const type_t lo, const type_t x, const type_t hi) {
  return (x < lo) ? lo : ((x > hi) ? hi : x);
}

template <typename type_t>
static inline type_t sign(const type_t x) {
  return (x < 0) ? -1 : (x > 0 ? 1 : 0);
}

template <typename type_t>
struct vec2_t {
  type_t x, y;
};

typedef vec2_t<int32_t> int2;

template <typename type_t>
inline bool operator == (const vec2_t<type_t> &a, const vec2_t<type_t> &b) {
  return a.x == b.x && a.y == b.y;
}

}  // namespace librl
