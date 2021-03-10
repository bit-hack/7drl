#pragma once
#include <memory>
#include <vector>
#include <queue>

#include "console.h"
#include "gc.h"
#include "entity.h"
#include "random.h"
#include "pfield.h"
#include "bitset2d.h"


namespace librl {

struct map_generator_t {

  map_generator_t(struct game_t &game)
    : game(game)
  {
  }

  virtual void generate() = 0;

  game_t &game;
};

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
  int2 mouse_pos;
};

struct game_t {

  game_t()
    : player(nullptr)
    , level(1)
    , seed(12345)
    , generate_new_map(false)
  {
  }

  void map_create(uint32_t w, uint32_t h);

  buffer2d_u8_t &map_get() {
    assert(map);
    return *map;
  }

  virtual void render();

  void console_create(uint32_t w, uint32_t h) {
    console.reset(new console_t(w, h));
    console->fill(' ');
  }

  console_t &console_get() {
    assert(console);
    return *console;
  }

  bitset2d_t &walls_get() {
    assert(walls);
    return *walls;
  }

  entity_t *entity_add(entity_t *ent) {
    assert(ent);
    // xxx: this is so crap, please fix me
    auto itt = std::find(entities.begin(), entities.end(), ent);
    if (itt == entities.end()) {
      entities.push_back(ent);
    }
    return ent;
  }

  void entity_remove(entity_t *ent) {
    // xxx: improve me
    auto itt = entities.begin();
    while (itt != entities.end()) {
      if (*itt == ent) {
        entities.erase(itt);
        return;
      }
      else {
        ++itt;
      }
    }
  }

  void entity_clear_all() {
    entities.clear();
    // note: we dont clear game.player here on purpose as we want that player
    // to persist between level changes
  }

  entity_t *entity_find(const int2 &p) const;

  // post a message to the player
  void message_post(const char *fmt, ...);

  void input_event_push(const input_event_t &event) {
    input.push_back(event);
  }

  bool input_event_pop(input_event_t &out) {
    if (input.empty()) {
      return false;
    }
    out = input.front();
    input.pop_front();
    return true;
  }

  void delay(uint32_t ms) {
    // todo
  }

  virtual void tick() {
    tick_game();
  }

  void tick_game();

  uint64_t random() {
    return librl::random(seed);
  }

  bool is_player_turn() const {
    return entities.empty() ? false : (entities.front() == player);
  }

  pfield_t &pfield_get() {
    assert(pfield);
    return *pfield;
  }

  void map_next() {
    generate_new_map = true;
    ++level;
  }

  gc_t gc;
  entity_t *player;

protected:

  bool generate_new_map;

  virtual void post_turn();

  virtual void render_map() = 0;
  virtual void render_hud() = 0;
  void render_entities();

  int level;
  uint64_t seed;

  std::deque<input_event_t> input;
  std::vector<entity_t *> entities;
  std::unique_ptr<map_generator_t> generator;
  std::unique_ptr<buffer2d_u8_t> map;
  std::unique_ptr<console_t> console;
  std::unique_ptr<pfield_t> pfield;
  std::unique_ptr<bitset2d_t> fog;
  std::unique_ptr<bitset2d_t> walls;
};

} // namespace librl
