#pragma once
#include <cstdint>

void font_draw_glyph_8x8(
  uint32_t *dst, const uint32_t pitch, uint16_t ch,
  const uint32_t rgb_a, const uint32_t rgb_b);

void font_draw_glyph_8x16(
  uint32_t *dst, const uint32_t pitch, uint16_t ch,
  const uint32_t rgb_a, const uint32_t rgb_b);
