#include <stdarg.h>
#include <array>
#include <cstdio>

#include "game.h"
#include "raycast.h"
#include "perlin.h"
#include "generator.h"
#include "entities.h"


namespace game {

void game_7drl_t::tick_game() {
  // update entities
  for (auto &e : entities) {
    assert(e);
    gc.check_in(e);
  }
  if (player) {
    gc.check_in(player);
  }

  // tick the next entitys
  tick_entities();

  // if we have been instructed to generate a new map
  if (generate_new_map) {
    entities.clear();
    map_create(map->width, map->height);
    generate_new_map = false;
  }
  // run a garbage collection cycle
  gc.collect();
}

void game_7drl_t::render_entities() {
  for (auto &e : entities) {
    assert(e);
    e->render();
  }
}

void game_7drl_t::map_create(uint32_t w, uint32_t h) {
  // clear the old map entities entirely
  entities.clear();
  // create various maps and arrays
  fog.reset(new librl::bitset2d_t(w, h));
  walls.reset(new librl::bitset2d_t(w, h));
  map.reset(new librl::buffer2d_u8_t(w, h));
  // run the map generator
  if (generator) {
    generator->generate(level);
  }
  // create a potential field
  assert(walls);
  pfield.reset(new librl::pfield_t(*map, *walls));
  render();
}

entity_t *game_7drl_t::entity_find(const librl::int2 &p) const {
  for (entity_t *e : entities) {
    if (e->pos == p) {
      return e;
    }
  }
  return nullptr;
}

void game_7drl_t::message_post(const char *str, ...) {

  using namespace librl;

  assert(console);
  auto &c = *console;

  std::array<char, 1024> temp;
  va_list args;
  va_start(args, str);
  vsnprintf(temp.data(), temp.size(), str, args);
  va_end(args);
  temp.back() = '\0';

  c.attrib.fill(
    int2{ 0,       c.height - 5 },
    int2{ c.width, c.height - 1 },
    c.colour);
  c.window_set(
    int2{ 0,       c.height - 5 },
    int2{ c.width, c.height - 1 });
  c.caret_set(int2{ 0, c.height - 2 });
  c.window_scroll();
  c.fill(int2{ 0, c.height - 2 }, int2{ c.width, c.height - 1 }, ' ');
  c.puts(temp.data());
  c.window_reset();
}

void game_7drl_t::set_seed(uint32_t s) {
  seed = s;
}

librl::buffer2d_u8_t &game_7drl_t::map_get() {
  assert(map);
  return *map;
}

void game_7drl_t::console_create(uint32_t w, uint32_t h) {
  console.reset(new librl::console_t(w, h));
  console->fill(' ');
}

librl::console_t &game_7drl_t::console_get() {
  assert(console);
  return *console;
}

librl::bitset2d_t &game_7drl_t::walls_get() {
  assert(walls);
  return *walls;
}

entity_t *game_7drl_t::entity_add(entity_t *ent) {
  assert(ent);
  // xxx: this is so crap, please fix me
  auto itt = std::find(entities.begin(), entities.end(), ent);
  if (itt == entities.end()) {
    entities.push_back(ent);
  }
  return ent;
}

void game_7drl_t::entity_remove(entity_t *ent) {
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

void game_7drl_t::entity_clear_all() {
  entities.clear();
  // note: we dont clear game.player here on purpose as we want that player
  // to persist between level changes
}

void game_7drl_t::input_event_push(const input_event_t &event) {
  input.push_back(event);
}

bool game_7drl_t::input_event_pop(input_event_t &out) {
  if (input.empty()) {
    return false;
  }
  out = input.front();
  input.pop_front();
  return true;
}

uint64_t game_7drl_t::random() {
  return librl::random(seed);
}

bool game_7drl_t::is_player_turn() const {
  return entities.empty() ? false : (entities.front() == player);
}

librl::pfield_t &game_7drl_t::pfield_get() {
  assert(pfield);
  return *pfield;
}

void game_7drl_t::map_next() {
  generate_new_map = true;
  ++level;
}

game_7drl_t::game_7drl_t()
  : screen(screen_title)
  , player(nullptr)
  , level(1)
  , seed(12345)
  , generate_new_map(false)
  , tick_index(0)
{
  generator.reset(new game::map_generator_t(*this));
}

void game_7drl_t::create_player() {
  assert(!player);
  player = gc.alloc<ent_player_t>(*this);
}

void game_7drl_t::tick_inventory(const librl::int2 &dir, bool use, bool drop) {
  inv_slot = librl::clamp<int>(0, inv_slot + dir.y, inventory_t::num_slots - 1);
  assert(player && player->is_type<ent_player_t>());
  ent_player_t &p = *static_cast<ent_player_t*>(player);
  if (use) {
    entity_t *item = p.inventory.slots()[inv_slot];
    p.inventory.use(inv_slot, player);
  }
  if (drop) {
    entity_t *item = p.inventory.slots()[inv_slot];
    if (item) {
      message_post("%s dropped %s", player->name.c_str(), item->name.c_str());
      p.inventory.drop(inv_slot);
    }
  }
}

void game_7drl_t::post_turn() {
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

void game_7drl_t::tick_entities() {
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

void game_7drl_t::tick() {
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

void game_7drl_t::tick_death() {
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

void game_7drl_t::render() {
  switch (screen) {
  case screen_game:
    if (!player) {
      return;
    }
    render_map();
    render_entities();
    render_hud();
    break;
  case screen_inventory:
    render_inventory();
    render_hud();
    break;
  }
}

void game_7drl_t::render_inventory() {
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
  for (int i = 0; i < inv.slots().size(); ++i) {
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

void game_7drl_t::render_hud() {
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

void game_7drl_t::render_map() {
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

}  // namespace game
