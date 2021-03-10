#define _SDL_main_h
#include <SDL.h>

#include <cassert>
#include <memory>
#include <array>

// librl
#include "buffer2d.h"
#include "console.h"
#include "game.h"

// game
#include "generator.h"
#include "enums.h"
#include "entities.h"

struct program_t {

  program_t(librl::game_t &game)
    : game(game)
    , width(0)
    , height(0)
    , active(false)
    , scale(0)
    , screen(nullptr)
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

    SDL_WM_SetCaption("TinyRL", nullptr);

    active = true;
    width = w;
    height = h;
    scale = s;
    game.console_create(w / 8, h / 8);
    return true;
  }

  void on_event(const SDL_Event &event) {

    using namespace librl;

    switch (event.type) {
    case SDL_QUIT:
      active = false;
      break;
    case SDL_KEYUP:

      switch (event.key.keysym.sym) {
      case SDLK_SPACE:
        // XXX: remove this!
        game.map_next();
        break;
      }
      switch (event.key.keysym.sym) {
      case SDLK_LEFT:   game.input_event_push(input_event_t{ input_event_t::key_left  }); break;
      case SDLK_RIGHT:  game.input_event_push(input_event_t{ input_event_t::key_right }); break;
      case SDLK_UP:     game.input_event_push(input_event_t{ input_event_t::key_up    }); break;
      case SDLK_DOWN:   game.input_event_push(input_event_t{ input_event_t::key_down  }); break;
      case SDLK_i:      game.input_event_push(input_event_t{ input_event_t::key_i     }); break;
      case SDLK_u:      game.input_event_push(input_event_t{ input_event_t::key_u     }); break;
      case SDLK_d:      game.input_event_push(input_event_t{ input_event_t::key_d     }); break;
      case SDLK_e:      game.input_event_push(input_event_t{ input_event_t::key_e     }); break;
      case SDLK_ESCAPE: game.input_event_push(input_event_t{ input_event_t::key_escape}); break;
      }
      break;
    case SDL_MOUSEBUTTONUP:
      game.input_event_push(input_event_t{
        (event.button.button == 0) ? input_event_t::mouse_lmb : input_event_t::mouse_rmb,
        event.button.x / 8,
        event.button.y / 8
        });
      break;
    }
  }

  void pump_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      on_event(event);
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
    const uint32_t pitch = width;
    game.console_get().render(d0, pitch, width / 8, height / 8);
    SDL_Flip(screen);
  }

  void render_x2() {
    assert(screen);
    const uint32_t pitch = width * 2;
    uint32_t *d0 = (uint32_t*)screen->pixels;
    uint32_t *d1 = d0 + pitch;
    // render to every second scanline and in the left half of the screen
    game.console_get().render(d0, pitch * 2, width / 8, height / 8);
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

  librl::game_t &game;

  uint32_t width, height;
  bool active;
  uint32_t scale;
  SDL_Surface *screen;
};


namespace game{

struct game_7drl_t : public librl::game_t {

  game_7drl_t()
    : screen(screen_game)
  {
    generator.reset(new game::generator_2_t(*this));
  }

  void create_player() {
    assert(!player);
    player = gc.alloc<ent_player_t>(*this);
  }

  void post_turn() override {
    if (player) {
      bool on_grass = (map_get().get(player->pos) == tile_grass);
      pfield->drop(player->pos.x, player->pos.y, on_grass ? 4 : 6);
    }
    game_t::post_turn();
  }

  void tick_inventory(const librl::int2 &dir, bool use, bool drop) {
    inv_slot = librl::clamp<int>(0, inv_slot + dir.y, librl::inventory_t::num_slots-1);
    assert(player && player->is_type<ent_player_t>());
    ent_player_t &p = *static_cast<ent_player_t*>(player);
    if (use) {
      p.inventory.use(inv_slot, player);
      inv_slot = 0;
    }
    if (drop) {
      librl::entity_t *item = p.inventory.slots()[inv_slot];
      if (item) {
        message_post("%s dropped %s", player->name.c_str(), item->name.c_str());
        p.inventory.drop(inv_slot);
      }
    }
  }

  void tick() override {

    librl::int2 dir = { 0, 0 };
    bool use = false;
    bool drop = false;

    librl::input_event_t event;
    if (input_event_pop(event)) {
      switch (event.type) {
      case librl::input_event_t::key_up:    --dir.y; break;
      case librl::input_event_t::key_down:  ++dir.y; break;
      case librl::input_event_t::key_left:  --dir.x; break;
      case librl::input_event_t::key_right: ++dir.x; break;
      case librl::input_event_t::key_u: use = true;  break;
      case librl::input_event_t::key_d: drop = true; break;

      case librl::input_event_t::key_i:
        if (screen == screen_game) {
          screen = screen_inventory;
          console->chars.clear(' ');
          render();
        }
        break;

      case librl::input_event_t::key_escape:
        if (screen == screen_inventory) {
          screen = screen_game;
          render();
        }
        break;
      }
    }

    if (player) {
      assert(player->is_type<ent_player_t>());
      ent_player_t &p = *static_cast<ent_player_t*>(player);
      p.user_dir = dir;
    }

    switch (screen) {
    case screen_game:
      tick_game();
      break;
    case screen_inventory:
      tick_inventory(dir, use, drop);
      render();
      break;
    }
  }

  void render() override {
    switch (screen) {
    case screen_game:
      game_t::render();
      break;
    case screen_inventory:
      render_inventory();
      render_hud();
      break;
    }
  }

  void render_inventory() {
    if (!player) {
      return;
    }

    auto &c = *console;
    c.colour = 0xfac4d1;

    c.caret_set(librl::int2{ 0, 0 });
    c.puts("Inventory");

    assert(player && player->is_type<ent_player_t>());
    ent_player_t *p = static_cast<ent_player_t*>(player);

    auto &inv = p->inventory;

    librl::int2 loc{ 2, 2 };
    for (int i=0; i<inv.slots().size(); ++i) {
      const librl::entity_t *e = inv.slots()[i];
      c.caret_set(librl::int2{ 2, 2 + i });
      c.colour = (i == inv_slot ) ? 0xfac4d1 : (e ? 0xbaa4b1 : 0x7a6471);
      c.print("%c %s    ", (i < 2 ? '*' : ' '), e ? e->name.c_str() : "empty");
    }
  }

  void render_hud() {
    using namespace librl;
    assert(console);
    auto &c = *console;

    assert(player && player->is_type<ent_player_t>());
    ent_player_t *p = static_cast<ent_player_t*>(player);

    console->fill(int2{ 0, c.height - 1 }, int2{ c.width, c.height }, ' ');

    console->colour = 0xfac4d1;

    console->caret_set(int2{ 0, c.height - 1 });
    console->print("level: %d  ", level);

    console->caret_set(int2{ 10, c.height - 1 });
    console->print("hp: %d  ", p->hp);

    console->caret_set(int2{ 20, c.height - 1 });
    console->print("gold: %d  ", p->gold);
  }

  void render_map() override {
    using namespace librl;
    assert(map && console && walls);
    auto &m = *map;
    auto &c = *console;

    const int32_t px = player->pos.x;
    const int32_t py = player->pos.y;

    static const std::array<char, 128> ramp = {
      '.', '#', '"', '?'
    };

    static const std::array<uint32_t, 128> rgb = {
      0xfac4d1, 0xfac4d1, 0xbad4b1, 0xfac4d1
    };

    for (uint32_t y = 0; y < m.height; ++y) {
      for (uint32_t x = 0; x < m.width; ++x) {
        auto &cell = m.get(x, y);

        uint8_t &ch = c.chars.get(x, y);

        uint32_t clr = rgb[cell];
        ch = ramp[cell];
        const int2 p = int2{ int32_t(x), int32_t(y) };

        const bool seen = raycast(player->pos, p, *walls);
        if (seen) {
          c.attrib.get(x, y) = clr;
          fog->set(int2{ int(x), int(y) });
        }
        else {
          c.attrib.get(x, y) = (clr >> 2) & 0x3f3f3f;
          if (!fog->get(p)) {
            ch = ' ';
          }
        }
      }
    }
  }

  uint32_t inv_slot;
  screen_t screen;
};

} // namespace game

int main(int argc, char *args[]) {

  game::game_7drl_t game;
  game.create_player();
  program_t prog{ game };

  if (!prog.init(game::screen_width,
                 game::screen_height, 1)) {
    return 1;
  }

  game.map_create(prog.width / 8, (prog.height / 8) - 2);

  while (prog.active) {
    prog.tick();
    prog.game.tick();
    prog.render();
    // dont burn up the CPU
    SDL_Delay(0);
  }

  return 0;
}
