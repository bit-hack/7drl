#define _SDL_main_h
#include <SDL.h>

#include <cassert>
#include <memory>
#include <array>

#include "font.h"
#include "random.h"
#include "buffer2d.h"
#include "console.h"


struct program_t {

  program_t()
    : screen(nullptr)
    , active(false)
    , console(32, 32)
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
    uint32_t *d0 = (uint32_t*)screen->pixels;
    const uint32_t pitch = width * 2;
    console.render(d0, pitch, width / 8, height / 8);
    SDL_Flip(screen);
  }

  void render_x2() {
    assert(screen);
    const uint32_t pitch = width * 2;
    uint32_t *d0 = (uint32_t*)screen->pixels;
    uint32_t *d1 = d0 + pitch;
    // render to every second scanline and in the left half of the screen
    console.render(d0, pitch * 2, width / 8, height / 8);
    // step over the screen unpacking the scanlines
    for (int y = 0; y < screen->h; y += 2) {
      for (int x = 0; x < screen->w; ++x) {
        d1[x] = d0[x >> 1];
      }
      // copy the now correct scanline to above one
      memcpy(d0, d1, screen->w * 4);
      // step down two scanlines
      d0 = d1 + pitch;
      d1 = d0 + pitch;
    }
    SDL_Flip(screen);
  }

  librl::console_t console;
  uint32_t width, height;
  bool active;
  uint32_t scale;
  SDL_Surface *screen;
};

std::array<const char *, 8> examples = {
  "Hello ",
  "Heya \r this is something",
  "really long string that will most likely wrap around\n",
  "Hello World\n",
  "this is another string\r\n",
  "foo bar",
  "this is a thing\n",
  "1\n"
};

int main(int argc, char *args[]) {

  program_t prog;

  if (!prog.init(256, 256, 2)) {
    return 1;
  }

  uint64_t seed = 12345;

  uint32_t ticks = SDL_GetTicks();

  prog.console.window_set(librl::int2{ 2, 2 }, librl::int2{ 16, 16 });
  prog.console.window_clear();

  while (prog.active) {
    prog.tick();

#if 0
    uint32_t cw = prog.console.chars.width;
    uint32_t ch = prog.console.chars.height;
    for (uint32_t y = 0; y < ch; ++y) {
      for (uint32_t x = 0; x < cw; ++x) {
        prog.console.chars.get(x, y) = (uint8_t)librl::random(seed);
      }
    }
#else
    uint32_t diff = SDL_GetTicks() - ticks;
    if (diff > 100) {
      ticks += diff;
      const char *str = examples[librl::random(seed) & 0x7];
      prog.console.puts(str);
    }
#endif

    prog.render();
    SDL_Delay(10);
  }

  return 0;
}
