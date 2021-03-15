#pragma once
#include <memory>
#include <vector>
#include <queue>

// librl
#include "console.h"
#include "gc.h"
#include "entity.h"
#include "random.h"
#include "pfield.h"
#include "bitset2d.h"

// game
#include "generator.h"
#include "enums.h"
#include "prog.h"

namespace game {

struct map_generator_t;

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

  game_t()
    : player(nullptr)
    , level(1)
    , seed(12345)
    , generate_new_map(false)
    , tick_index(0)
  {
  }

  void set_seed(uint32_t s);

  void map_create(uint32_t w, uint32_t h);

  librl::buffer2d_u8_t &map_get();

  virtual void render();

  void console_create(uint32_t w, uint32_t h);

  librl::console_t &console_get();

  librl::bitset2d_t &walls_get();

  librl::entity_t *entity_add(librl::entity_t *ent);

  void entity_remove(librl::entity_t *ent);

  void entity_clear_all();

  librl::entity_t *entity_find(const librl::int2 &p) const;

  // post a message to the player
  void message_post(const char *fmt, ...);

  void input_event_push(const input_event_t &event);

  bool input_event_pop(input_event_t &out);

  virtual void delay(uint32_t ms) = 0;

  virtual void tick();

  void tick_game();
  virtual void tick_entities() = 0;

  uint64_t random();

  bool is_player_turn() const;

  librl::pfield_t &pfield_get();

  void map_next();

  librl::gc_t gc;
  librl::entity_t *player;

protected:

  bool generate_new_map;

  virtual void post_turn();

  virtual void render_map() = 0;
  virtual void render_hud() = 0;
  void render_entities();

  int level;
  uint64_t seed;
  uint32_t tick_index;

  std::deque<input_event_t> input;
  std::vector<librl::entity_t *> entities;
  std::unique_ptr<map_generator_t> generator;
  std::unique_ptr<librl::buffer2d_u8_t> map;
  std::unique_ptr<librl::console_t> console;
  std::unique_ptr<librl::pfield_t> pfield;
  std::unique_ptr<librl::bitset2d_t> fog;
  std::unique_ptr<librl::bitset2d_t> walls;
};

} // namespace game


namespace game {

struct game_7drl_t : public game::game_t {

  game_7drl_t();

  void create_player();

  void tick_inventory(const librl::int2 &dir, bool use, bool drop);

  void post_turn() override;

  void tick_entities() override;

  void tick() override;

  void tick_title();

  void tick_death();

  void render() override;

  virtual void delay(uint32_t ms);

  void render_inventory();

  void render_hud();

  void render_map() override;

  uint32_t inv_slot;
  screen_t screen;
  uint32_t time_thresh;
};

} // namespace game
