#include <cstdarg>
#include <array>

#include "console.h"
#include "font.h"
#include "common.h"

namespace librl {

void console_t::render(uint32_t *dst, uint32_t pitch, uint32_t chars_x, uint32_t chars_y) {
  const uint32_t bg = 0x202020;
  for (uint32_t y = 0; y < chars_y; ++y) {
    for (uint32_t x = 0; x < chars_x; ++x) {
      const uint32_t fg = attrib.get(x, y);
      const buffer2d_u8_t::type ch = chars.get(x, y);
      font_draw_glyph_8x8(dst + x * 8, pitch, ch, fg, bg);
    }
    dst += pitch * 8;
  }
}

void console_t::putc(char c) {
  switch (c) {
  case '\n':
    caret.x = window_min.x; // reset x
    ++caret.y; // new line
    break;
  case '\r':
    caret.x = window_min.x; // reset x
    break;
  default:
    chars.get(caret.x, caret.y) = c; // write char
    ++caret.x;
    if (caret.x >= window_max.x) {
      caret.x = window_min.x; // reset x
      ++caret.y; // new line
    }
  }
  // scroll if at the bottom of the window
  if (caret.y >= window_max.y) {
    window_scroll();
    --caret.y;
  }
}

void console_t::window_scroll() {
  for (int32_t y = window_min.y; y < (window_max.y - 1); ++y) {
    for (int32_t x = window_min.x; x < window_max.x; ++x) {
      chars.get(x, y) = chars.get(x, y + 1);
    }
  }
  // clear the last line
  for (int32_t x = window_min.x; x < window_max.x; ++x) {
    chars.get(x, window_max.y - 1) = ' ';
  }
}

void console_t::window_set(const int2 &wmin, const int2 &wmax) {
  window_min.x = max<int>(wmin.x, 0);
  window_min.y = max<int>(wmin.y, 0);
  window_max.x = min<int>(wmax.x, int(chars.width));
  window_max.y = min<int>(wmax.y, int(chars.height));
  caret_set(caret);
}

void console_t::window_reset() {
  window_set(int2{ 0, 0 }, int2{ int(chars.width), int(chars.height) });
}

void console_t::caret_set(const int2 &pos) {
  // clamp the caret to the new window
  caret.x = clamp(window_min.x, pos.x, window_max.x - 1);
  caret.y = clamp(window_min.y, pos.y, window_max.y - 1);
}

void console_t::window_clear() {
  for (int32_t y = window_min.y; y < window_max.y; ++y) {
    for (int32_t x = window_min.x; x < window_max.x; ++x) {
      chars.get(x, y) = ' ';
    }
  }
}

void console_t::print(const char *fmt, ...) {
  std::array<char, 1024> temp;
  va_list args;
  va_start(args, fmt);
  vsnprintf(temp.data(), temp.size(), fmt, args);
  va_end(args);
  temp.back() = '\0';
  puts(temp.data());
}

void console_t::fill(const int2 &min, const int2 &max, char ch) {
  for (int y = min.y; y < max.y; ++y) {
    for (int x = min.x; x < max.x; ++x) {
      chars.get(x, y) = ch;
    }
  }
}

void console_t::fill(char ch) {
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      chars.get(x, y) = ch;
    }
  }
}

}  // librl
