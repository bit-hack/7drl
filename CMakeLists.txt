cmake_minimum_required(VERSION 3.0)
project(7drl)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

file(GLOB LIBRL_SRC librl/*.cpp librl/*.h)
add_library(librl ${LIBRL_SRC})
target_include_directories(librl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/librl)

file(GLOB GAME_SRC game/*.cpp game/*.h)
add_executable(game ${GAME_SRC})
target_link_libraries(game librl ${SDL_LIBRARY})

file(GLOB GAME2_SRC game2/*.cpp game2/*.h)
add_executable(game2 ${GAME2_SRC})
target_link_libraries(game2 librl ${SDL_LIBRARY})
