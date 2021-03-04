#define _SDL_main_h
#include <SDL.h>

#include <cassert>
#include <memory>

#include "font.h"
#include "random.h"


struct program_t {

  struct buffer_t {
    uint32_t *pixels;
    uint32_t width;
    uint32_t height;
  };

  program_t()
    : screen(nullptr)
    , active(false)
  {
  }

  bool init(uint32_t w, uint32_t h, uint32_t s) {
    assert(screen == nullptr);
    assert(w && h);
    assert(s == 1 || s == 2);
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      return false;
    }
    screen = SDL_SetVideoMode(w * s, h * s, 32, 0);
    if (!screen) {
      return false;
    }
    active = true;
    width = w;
    height = h;
    scale = s;
    if (s > 1) {
      buffer.reset(new uint32_t[w * h]);
    }
    return true;
  }

  void pump_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        active = false;
        return;
      }
    }
  }

  void tick() {
    pump_events();
  }

  void render() {
    assert(screen);
    switch (scale) {
    case 1: render_x1(); break;
    case 2: render_x2(); break;
    default: assert(!"unsupported scale factor");
    }
  }

  void render_x1() {
    assert(screen);
    SDL_Flip(screen);
  }

  void render_x2() {
    assert(screen);
    uint32_t *d0 = (uint32_t*)screen->pixels;
    uint32_t *d1 = d0 + width * 2;
    const uint32_t *src = buffer.get();
    for (uint32_t y = 0; y < height; ++y) {
      for (uint32_t x = 0; x < width; ++x) {
        const uint32_t pix = src[x];
        d0[x * 2 + 0] = pix;
        d0[x * 2 + 1] = pix;
        d1[x * 2 + 0] = pix;
        d1[x * 2 + 1] = pix;
      }
      src += width;
      d0  += width * 4;
      d1  += width * 4;
    }
    SDL_Flip(screen);
  }

  void get_buffer(buffer_t &out) {
    assert(screen && width && height);
    out.width = width;
    out.height = height;
    if (buffer) {
      out.pixels = buffer.get();
    }
    else {
      out.pixels = (uint32_t*)screen->pixels;
    }
  }

  uint32_t width, height;
  bool active;
  uint32_t scale;
  std::unique_ptr<uint32_t[]> buffer;
  SDL_Surface *screen;
};

int main(int argc, char *args[]) {

  program_t prog;

  if (!prog.init(320, 240, 2)) {
    return 1;
  }

  uint64_t seed = 12345;

  while (prog.active) {
    prog.tick();

    program_t::buffer_t b;
    prog.get_buffer(b);

    for (uint32_t y = 0; y < b.height; ++y) {
      for (uint32_t x = 0; x < b.width; ++x) {
        b.pixels[x + y * b.width] = librl::random(seed);
      }
    }

    prog.render();
  }

  return 0;
}
