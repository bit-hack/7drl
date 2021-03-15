//#include <Windows.h>

#define _SDL_main_h
#include <SDL.h>

#include <cassert>
#include <memory>
#include <array>
#include <thread>

// librl
#include "buffer2d.h"
#include "console.h"

// game
#include "generator.h"
#include "enums.h"
#include "entities.h"
#include "prog.h"
#include "game.h"


static const char splash1[] = R"(
                                                                
                                                                
     @@@@@@@  @@@  @@@  @@@  @@@ @@@  @@@@@@@   @@@             
     @@@@@@@  @@@  @@@@ @@@  @@@ @@@  @@@@@@@@  @@@             
       @@!    @@!  @@!@!@@@  @@! !@@  @@!  @@@  @@!             
       !@!    !@!  !@!!@!@!  !@! @!!  !@!  @!@  !@!             
       @!!    !!@  @!@ !!@!   !@!@!   @!@!!@!   @!!             
       !!!    !!!  !@!  !!!    @!!!   !!@!@!    !!!             
       !!:    !!:  !!:  !!!    !!:    !!: :!!   !!:             
       :!:    :!:  :!:  !:!    :!:    :!:  !:!   :!:            
        ::     ::   ::   ::     ::    ::   :::   :: ::::        
        :     :    ::    :      :      :   : :  : :: : :        
           TinyRL - 7DRL Entry by BitHack (2021)                
                                                                
     Controls:                                                  
       [cursor] - move                                          
       [i]      - open/close inventory                          
       [d]      - drop item       (in inventory)                
       [u]      - use/equip item  (in inventory)                
                                                                
     Glyphs:                                                    
       @ - player       d - Dwarf             " - Grass         
       ? - Item         W - Warlock           . - Floor         
       $ - Gold         s - Skeleton                            
       g - Goblin       p - Potion (health)                     
       v - Vampire      = - Stairs                              
       o - Orge         w - Wrath                               
                                                                
                                                                
                     To start press [u]                         
                                                                
)";

namespace game {
void game_t::tick_title() {
  console->puts(splash1);
}
}  // namespace game

extern "C" {
int __stdcall FreeConsole(void);
}

int main(int argc, char *args[]) {

  FreeConsole();

  game::game_t game;
  game.set_seed(SDL_GetTicks());
  game.create_player();
  program_t prog{ game };

  if (!prog.init(game::screen_width,
                 game::screen_height,
                 game::screen_scale)) {
    return 1;
  }

  game.map_create(prog.width / 8, (prog.height / 8) - 5);

  while (prog.active) {
    prog.tick();
    prog.game.tick();
    prog.render();
    // dont burn up the CPU
    SDL_Delay(10);
    std::this_thread::yield();
  }

  return 0;
}
