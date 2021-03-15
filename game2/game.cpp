#include "game.h"


void game_t::input_event_push(const input_event_t &event) {
  (void)event;
}

void game_t::tick() {
  auto &c = console();
  for (auto &r : com_render.get_all()) {
    com_render_t *x = r.second;
    x->render(r.first, *this);
  }

  // garbage collect step
  // XXX: this could be moved into the game store when it detects a ref is zero
  //      we can collect it immediately
  com_input.collect(ecs.refs());
  com_render.collect(ecs.refs());
  com_info.collect(ecs.refs());
}

void com_render_t::render(librl::ecs_id_t id, game_t &game) {
  auto &con = game.console();
  com_info_t *info = game.com_info.get(id);
  if (info) {
    con.caret_set(info->pos);
    con.putc(glyph);
  }
}

void com_input_t::recv(librl::ecs_id_t id, game_t &game, const input_event_t &event) {

}
