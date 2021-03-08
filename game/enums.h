#pragma once

namespace game {

enum {
  tile_floor = 0,
  tile_wall  = 1,
};

enum {
  ent_type_none = 0,
  ent_type_player,
  ent_type_test,
  ent_type_potion,
  ent_type_stairs,
};

enum {
  screen_width = 320,
  screen_height = 240
};

}  // namespace game
