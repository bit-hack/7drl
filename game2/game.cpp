#include "game.h"


void game_t::input_event_push(const input_event_t &event) {
  for (auto &i : com_input.get_all()) {
    com_input_t *x = i.second;
    assert(x);
    x->handle(i.first, *this, event);
  }
}

void game_t::_collect() {
  const auto &dead = ecs.dead_get();
  if (!dead.empty()) {
    com_input.collect(dead);
    com_render.collect(dead);
    com_info.collect(dead);
    com_think.collect(dead);
    ecs.dead_clear();
  }
}

void game_t::tick() {
  auto &c = console();
  c.fill(' ');
  // tick everything
  for (auto &r : com_think.get_all()) {
    com_think_t *x = r.second;
    x->think(r.first, *this);
  }
  // render everything
  for (auto &r : com_render.get_all()) {
    com_render_t *x = r.second;
    x->render(r.first, *this);
  }
  // collect any dead componants
  _collect();
}

void com_render_t::render(librl::ecs_id_t id, game_t &game) {
  auto &con = game.console();
  if (com_info_t *info = game.com_info.get(id)) {
    auto &p = info->pos;
    if (p.x >= 0 && p.y >= 0 && p.x < con.width && p.y < con.height) {
      con.caret_set(p);
      con.putc(glyph);
    }
  }
}

void com_input_t::handle(librl::ecs_id_t id, game_t &game, const input_event_t &event) {
  if (com_info_t *info = game.com_info.get(id)) {
    switch (event.type) {
    case input_event_t::key_up:    --info->pos.y; break;
    case input_event_t::key_down:  ++info->pos.y; break;
    case input_event_t::key_left:  --info->pos.x; break;
    case input_event_t::key_right: ++info->pos.x; break;
    case input_event_t::key_u:
      // delete enemy
      game.ecs.id_release(game.enemy);
      game.enemy = librl::ecs_id_invalid;
      break;
    }
  }
}

void com_think_t::think(librl::ecs_id_t id, game_t &game) {
  if (com_info_t *info = game.com_info.get(id)) {
    // todo
  }
}

void game_t::player_create() {
  player = ecs.id_new();
  assert(player != librl::ecs_id_invalid);
  if (com_info_t *i = com_info.insert(player, new com_info_t)) {
    i->pos = librl::int2{ 0, 0 };
  }
  if (com_render_t *r = com_render.insert(player, new com_render_t)) {
    r->glyph = '@';
  }
  com_input.insert(player, new com_input_t);
}

void game_t::enemy_create() {
  enemy = ecs.id_new();
  assert(enemy != librl::ecs_id_invalid);
  if (com_info_t *i = com_info.insert(enemy, new com_info_t)) {
    i->pos = librl::int2{ 8, 8 };
  }
  if (com_render_t *r = com_render.insert(enemy, new com_render_t)) {
    r->glyph = '?';
  }
  com_think.insert(player, new com_think_t);
}
