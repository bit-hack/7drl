#pragma once

namespace game {

enum screen_t {
  screen_game,
  screen_inventory,
  screen_title,
  screen_death,
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
  ent_type_vampire,
  ent_type_ogre,
  ent_type_wrath,
  ent_type_dwarf,
  ent_type_warlock,
  ent_type_skeleton,
  ent_type_mimic,

  ent_type_potion,

  ent_type_stairs,

  ent_type_gold,

  ent_type_club,
  ent_type_mace,
  ent_type_sword,
  ent_type_dagger,
  ent_type_leather_armour,
  ent_type_metal_armour,
  ent_type_cloak_armour,
  ent_type_shield,
};

enum {
  screen_width  = 512,
  screen_height = 512,
};

enum {
  colour_player   = 0xf7f28e,

  colour_goblin   = 0x46803a,
  colour_vampire  = 0xa0664a,
  colour_ogre     = 0x80763a,
  colour_wrath    = 0x80763a,
  colour_dwarf    = 0x80763a,
  colour_warlock  = 0x80763a,
  colour_skeleton = 0xa0a6aa,

  colour_stairs   = 0xf7f28e,
  colour_potion   = 0x463a80,
  colour_gold     = 0xdfdf40,
  colour_item     = 0xdfdf40,
};

}  // namespace game
