#define _SDL_main_h
#include <SDL.h>

#include <cassert>
#include <memory>
#include <array>
#include <thread>

// librl
#include "buffer2d.h"
#include "console.h"
#include "game.h"

// game
#include "generator.h"
#include "enums.h"
#include "entities.h"


static const char splash1[] = R"(
                                                                
                                                                
     @@@@@@@  @@@  @@@  @@@  @@@ @@@  @@@@@@@   @@@             
     @@@@@@@  @@@  @@@@ @@@  @@@ @@@  @@@@@@@@  @@@             
       @@!    @@!  @@!@!@@@  @@! !@@  @@!  @@@  @@!             
       !@!    !@!  !@!!@!@!  !@! @!!  !@!  @!@  !@!             
       @!!    !!@  @!@ !!@!   !@!@!   @!@!!@!   @!!             
       !!!    !!!  !@!  !!!    @!!!   !!@!@!    !!!             
       !!:    !!:  !!:  !!!    !!:    !!: :!!   !!:             
       :!:    :!:  :!:  !:!    :!:    :!:  !:!   :!:            
        ::     ::   ::   ::     ::    ::   :::   :: ::::        
        :     :    ::    :      :      :   : :  : :: : :        
           TinyRL - 7DRL Entry by BitHack (2021)                
                                                                
     Controls:                                                  
       [cursor] - move                                          
       [i]      - open/close inventory                          
       [d]      - drop item       (in inventory)                
       [u]      - use/equip item  (in inventory)                
                                                                
     Glyphs:                                                    
       @ - player       d - Dwarf             " - Grass         
       ? - Item         W - Warlock           . - Floor         
       $ - Gold         s - Skeleton                            
       g - Goblin       p - Potion (health)                     
       v - Vampire      = - Stairs                              
       o - Orge         w - Wrath                               
                                                                
                                                                
                     To start press [u]                         
                                                                
)";


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

    SDL_EnableKeyRepeat(100, 100);

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
//    case SDL_KEYUP:
    case SDL_KEYDOWN:

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
    : screen(screen_title)
    , time_thresh(0)
  {
    generator.reset(new game::generator_2_t(*this));
  }

  void create_player() {
    assert(!player);
    player = gc.alloc<ent_player_t>(*this);
  }

  void tick_inventory(const librl::int2 &dir, bool use, bool drop) {
    inv_slot = librl::clamp<int>(0, inv_slot + dir.y, librl::inventory_t::num_slots - 1);
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

  void post_turn() override {
    using namespace librl;

    if (entities.empty() || !player) {
      return;
    }

    // render before player turn
    const entity_t *this_ent = entities[tick_index % entities.size()];
    if (this_ent->is_type<ent_player_t>()) {
      // update the potential field
      assert(pfield);
      // tick more times for a good spread
      for (int i = 0; i < 4; ++i) {
        pfield->update();
        // drop the player smell
        const bool on_grass = (map_get().get(player->pos) == tile_grass);
        pfield->drop(player->pos.x, player->pos.y, on_grass ? 253 : 255);
      }
      // update the view
      render();
    }

    // tick the next entity
    tick_index++;

    // render after player turn
    const entity_t *next_ent = entities[tick_index % entities.size()];
    if (next_ent->is_type<ent_player_t>()) {
      render();
    }
  }

  void tick_entities() override {
    using namespace librl;
    while (!entities.empty()) {
      entity_t *ent = entities[tick_index % entities.size()];
      assert(ent);
      if (ent->turn()) {
        post_turn();
      }
      if (ent->is_type<ent_player_t>()) {
        break;
      }
    }
  }

  void tick() override {
#if 0
    if (time_thresh >= SDL_GetTicks()) {
      return;
    }
#endif

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
      case librl::input_event_t::key_u:
        if (screen == screen_title) {
          screen = screen_game;
          render();
        }
        use = true;
        break;

      case librl::input_event_t::key_d: drop = true; break;

      case librl::input_event_t::key_i:
        switch (screen) {
        case screen_game:
          screen = screen_inventory;
          console->chars.clear(' ');
          render();
          break;
        case screen_inventory:
          screen = screen_game;
          render();
          break;
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
    case screen_title:
      tick_title();
      break;
    case screen_death:
      tick_death();
      break;
    }

    if (player->as_a<ent_player_t>()->hp == 0) {
      screen = screen_death;
    }
  }

  void tick_title() {
    console->puts(splash1);
  }

  void tick_death() {
    using namespace librl;

    const uint32_t xbase = 23;
    const uint32_t ybase = 28;

    console->attrib.fill(0xfac4d1);
    console->chars.fill(' ');
    console->caret_set(int2{ xbase, ybase + 0 });
    console->print("You have died");

    console->caret_set(int2{ xbase, ybase + 2 });
    console->print("  Level: %d", level);

    ent_player_t *p = player->as_a<ent_player_t>();
    if (p) {
      console->caret_set(int2{ xbase, ybase + 4 });
      console->print("   Gold: %d", p->gold);

      console->caret_set(int2{ xbase, ybase + 6 });
      console->print("     XP: %d", p->xp);

      console->caret_set(int2{ xbase, ybase + 8 });
      int score = p->xp + (int((float(level) / 8.f) * float(p->gold))) + level * 51;
      console->print("  Score: %d", score);
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

  virtual void delay(uint32_t ms) {
    time_thresh = SDL_GetTicks() + ms;
  }

  void render_inventory() {
    using namespace librl;

    if (!player) {
      return;
    }

    auto &c = *console;
    c.colour = 0xfac4d1;

    c.caret_set(int2{ 0, 0 });
    c.puts("Inventory");

    assert(player && player->is_type<ent_player_t>());
    ent_player_t *p = static_cast<ent_player_t*>(player);

    auto &inv = p->inventory;

    int2 loc{ 2, 2 };
    for (int i=0; i<inv.slots().size(); ++i) {
      const entity_t *e = inv.slots()[i];

      c.caret_set(librl::int2{ 2, 2 + i });
      c.colour = (i == inv_slot) ? 0xfac4d1 : (e ? 0xbaa4b1 : 0x7a6471);

      const int y = 2 + i;
      c.fill(int2{ 0, y }, int2{ c.width, y + 1 }, ' ');
      c.print(i < 2 ? "* " : "  ");

      if (e == nullptr) {
        const int y = c.caret_get().y;
        c.print("empty");
        continue;
      }

      if (auto *x = e->as_a<entity_item_t>()) {
        c.print("%s", x->name.c_str());
        continue;
      }

      if (auto *x = e->as_a<entity_equip_t>()) {
        const int dam = x->damage;
        const int acc = x->accuracy;
        const int eva = x->evasion;
        const int cri = x->crit;
        const int def = x->defense;

        c.print("%-20s +%02ddam +%02dacc +%02deva +%02dcri +%02ddef",
          x->name.c_str(), dam, acc, eva, cri, def);
        continue;
      }
    }
  }

  void render_hud() {
    using namespace librl;
    assert(console);
    auto &c = *console;

    if (!player) {
      return;
    }

    assert(player && player->is_type<ent_player_t>());
    ent_player_t *p = static_cast<ent_player_t*>(player);

    console->fill(int2{ 0, c.height - 1 }, int2{ c.width, c.height }, ' ');

    console->colour = 0xfac4d1;

    int2 l = int2{ 0, c.height - 1 };

    const int x = 8;

    console->caret_set(l);
    console->print("lev %d  ", level);
    l.x += 7;
    console->caret_set(l);
    console->print("hp %d  ", p->hp);
    l.x += x;
    console->caret_set(l);
    console->print("gol %d  ", p->gold);
    l.x += x;
    console->caret_set(l);
    console->print("dam %d  ", p->get_damage());
    l.x += x;
    console->caret_set(l);
    console->print("acc %d  ", p->get_accuracy());
    l.x += x;
    console->caret_set(l);
    console->print("eva %d  ", p->get_evasion());
    l.x += x;
    console->caret_set(l);
    console->print("cri %d  ", p->get_crit());
    l.x += x;
    console->caret_set(l);
    console->print("def %d  ", p->get_defense());
    l.x += x;
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

        const bool seen = false | raycast(player->pos, p, *walls);
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
  uint32_t time_thresh;
};

} // namespace game

int main(int argc, char *args[]) {

  game::game_7drl_t game;
  game.set_seed(SDL_GetTicks());
  game.create_player();
  program_t prog{ game };

  if (!prog.init(game::screen_width,
                 game::screen_height, 1)) {
    return 1;
  }

  game.map_create(prog.width / 8, (prog.height / 8) - 5);

  while (prog.active) {
    prog.tick();
    prog.game.tick();
    prog.render();
    // dont burn up the CPU
    SDL_Delay(10);
    std::this_thread::yield();
  }

  return 0;
}
