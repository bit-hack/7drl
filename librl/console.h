#pragma once
#include "buffer2d.h"
#include "common.h"

namespace librl {

struct console_t {

  console_t(uint32_t w, uint32_t h)
    : chars(w, h)
    , caret{0, 0}
  {
    window_reset();
  }

  void render(uint32_t *dst, uint32_t pitch, uint32_t chars_x,
              uint32_t chars_y);

  void putc(const char ch);

  void puts(const char *str) {
    for (; *str; ++str) {
      putc(*str);
    }
  }

  void window_set(const int2 &wmin, const int2 &wmax);

  void window_reset();

  void window_clear();

  void caret_set(const int2 &pos);

  const int2 &caret_get() const {
    return caret;
  }

  buffer2d_t chars;

protected:
  // scroll the window up one row
  void window_scroll();

  int2 caret;
  int2 window_min;
  int2 window_max;  // max size inclusive
};

}  // librl