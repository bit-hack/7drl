#pragma once
#include <memory>
#include <vector>
#include <queue>

// librl
#include "console.h"
#include "gc.h"
#include "random.h"
#include "pfield.h"
#include "bitset2d.h"

// game
#include "generator.h"
#include "enums.h"


namespace game {

struct map_generator_t;
struct entity_t;

struct input_event_t {
  enum {
    key_up,
    key_down,
    key_left,
    key_right,
    key_enter,
    key_escape,
    key_i,  // inventory
    key_u,  // use
    key_d,  // drop
    key_e,  // equip
    mouse_lmb,
    mouse_rmb,
  } type;
  librl::int2 mouse_pos;
};

struct game_t {

  game_t();

  void create_player();
  bool is_player_turn() const;

  void post_turn();

  void tick_game();
  void tick_entities();
  void tick();
  void tick_title();
  void tick_death();
  void tick_inventory(const librl::int2 &dir, bool use, bool drop);

  void render();
  void render_inventory();
  void render_hud();
  void render_map();
  void render_entities();

  void set_seed(uint32_t s);

  void map_create(uint32_t w, uint32_t h);
  void map_next();

  void console_create(uint32_t w, uint32_t h);

  entity_t *entity_add(entity_t *ent);
  void entity_remove(entity_t *ent);
  void entity_clear_all();
  entity_t *entity_find(const librl::int2 &p) const;

  void message_post(const char *fmt, ...);

  void input_event_push(const input_event_t &event);
  bool input_event_pop(input_event_t &out);

  uint64_t random();

  librl::buffer2d_u8_t &map_get();
  librl::console_t &console_get();
  librl::bitset2d_t &walls_get();
  librl::pfield_t &pfield_get();

  librl::gc_t gc;
  entity_t *player;

protected:

  screen_t screen;
  uint32_t inv_slot;
  bool generate_new_map;

  int level;
  uint64_t seed;
  uint32_t tick_index;

  std::deque<input_event_t> input;
  std::vector<entity_t *> entities;
  std::unique_ptr<map_generator_t> generator;
  std::unique_ptr<librl::buffer2d_u8_t> map;
  std::unique_ptr<librl::console_t> console;
  std::unique_ptr<librl::pfield_t> pfield;
  std::unique_ptr<librl::bitset2d_t> fog;
  std::unique_ptr<librl::bitset2d_t> walls;
};

} // namespace game
