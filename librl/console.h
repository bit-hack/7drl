#pragma once
#include "buffer2d.h"
#include "common.h"

namespace librl {

struct console_t {

  console_t(uint32_t w, uint32_t h)
    : attrib(w, h)
    , chars(w, h)
    , width(w)
    , height(h)
    , caret{0, 0}
    , colour(0xfac4d1)
  {
    window_reset();
    chars.clear(' ');
    attrib.clear(0x33f080);
  }

  void render(uint32_t *dst, uint32_t pitch, uint32_t chars_x,
              uint32_t chars_y);

  void putc(const char ch);

  void puts(const char *str) {
    for (; *str; ++str) {
      putc(*str);
    }
  }

  void fill(const int2 &min, const int2 &max, char ch);

  void fill(char ch);

  void print(const char *fmt, ...);

  void window_set(const int2 &wmin, const int2 &wmax);

  void window_reset();

  void window_clear();

  void window_scroll();

  void caret_set(const int2 &pos);

  const int2 &caret_get() const {
    return caret;
  }

  buffer2d_u32_t attrib;
  buffer2d_u8_t chars;
  const int width;
  const int height;

  uint32_t colour;

protected:

  int2 caret;
  int2 window_min;
  int2 window_max;  // max size inclusive
};

}  // librl
