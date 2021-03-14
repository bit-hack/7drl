//#include <Windows.h>

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
#include "prog.h"


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


namespace game{

struct game_7drl_t : public game::game_t {

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
      librl::entity_t *item = p.inventory.slots()[inv_slot];
      p.inventory.use(inv_slot, player);
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

    game::input_event_t event;
    if (input_event_pop(event)) {
      switch (event.type) {
      case game::input_event_t::key_up:    --dir.y; break;
      case game::input_event_t::key_down:  ++dir.y; break;
      case game::input_event_t::key_left:  --dir.x; break;
      case game::input_event_t::key_right: ++dir.x; break;
      case game::input_event_t::key_u:
        if (screen == screen_title) {
          screen = screen_game;
          console->fill(' ');
          render();
        }
        use = true;
        break;

      case game::input_event_t::key_d: drop = true; break;

      case game::input_event_t::key_i:
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

extern "C" {
int __stdcall FreeConsole(void);
}

int main(int argc, char *args[]) {

  FreeConsole();

  game::game_7drl_t game;
  game.set_seed(SDL_GetTicks());
  game.create_player();
  program_t prog{ game };

  if (!prog.init(game::screen_width,
                 game::screen_height,
                 game::screen_scale)) {
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
