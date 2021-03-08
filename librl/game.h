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
    mouse_lmb,
    mouse_rmb,
  } type;
  int2 mouse_pos;
};

struct game_t {

  game_t()
    : player(nullptr)
    , seed(12345)
    , generate_new_map(false)
  {
  }

  void map_create(uint32_t w, uint32_t h) {
    // clear the old map entities entirely
    entities.clear();

    map.reset(new buffer2d_t(w, h));
    // run the map generator
    if (generator) {
      generator->generate();
    }

    // XXX: FIXME
    const uint8_t wall_tile = 1;

    // create a potential field
    pfield.reset(new pfield_t(*map, wall_tile));

    fog.reset(new bitset2d_t(map->width, map->height));

    render();
  }

  buffer2d_t &map_get() {
    assert(map);
    return *map;
  }

  void render();

  void console_create(uint32_t w, uint32_t h) {
    console.reset(new console_t(w, h));
  }

  console_t &console_get() {
    assert(console);
    return *console;
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

  void tick();

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
  }

  gc_t gc;
  entity_t *player;

protected:

  bool generate_new_map;

  void post_turn();

  uint64_t seed;

  std::deque<input_event_t> input;
  std::vector<entity_t *> entities;
  std::unique_ptr<map_generator_t> generator;
  std::unique_ptr<buffer2d_t> map;
  std::unique_ptr<console_t> console;
  std::unique_ptr<pfield_t> pfield;
  std::unique_ptr<bitset2d_t> fog;
};

} // namespace librl
