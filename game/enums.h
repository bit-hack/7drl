#pragma once

namespace game {

enum screen_t {
  screen_game,
  screen_inventory,
};

enum {
  tile_floor = 0,
  tile_wall,
  tile_grass,
};

enum {
  ent_type_none = 0,
  ent_type_player,
  ent_type_goblin,
  ent_type_potion,
  ent_type_stairs,
  ent_type_gold,
};

enum {
  screen_width  = 512,
  screen_height = 512,
};

enum {
  colour_player = 0xf7f28e,
  colour_goblin = 0x46803a,
  colour_stairs = 0xf7f28e,
  colour_potion = 0x463a80,
  colour_gold   = 0xdfdf40,
};

}  // namespace game
