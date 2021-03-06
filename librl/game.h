#pragma once
#include <memory>
#include <vector>
#include <queue>

#include "console.h"
#include "gc.h"
#include "entity.h"
#include "random.h"

namespace librl {

struct map_generator_t {
  virtual void generate(game_t &game) = 0;
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
  {
  }

  void map_create(uint32_t w, uint32_t h) {
    map.reset(new buffer2d_t(w, h));
    // run the map generator
    if (generator) {
      generator->generate(*this);
    }
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
    entities.push_back(ent);

    // bubble to front as needed
    auto itt = entities.rbegin();
    while (std::next(itt) != entities.rend()) {
      if ((*itt)->order < (*std::next(itt))->order) {
        std::swap(*itt, *std::next(itt));
        itt = std::next(itt);
      }
      else {
        break;
      }
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
    }
  }

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

  gc_t gc;
  entity_t *player;

protected:

  void post_turn();

  uint64_t seed;

  std::deque<input_event_t> input;
  std::vector<entity_t *> entities;
  std::unique_ptr<map_generator_t> generator;
  std::unique_ptr<buffer2d_t> map;
  std::unique_ptr<console_t> console;
};

} // namespace librl
