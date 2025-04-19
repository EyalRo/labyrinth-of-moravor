# Labyrinth of Moravor

A modular, SDL2-based dungeon crawler engine and game.

## Directory Structure

- `engine/`: Core engine (rendering, input, audio, tilemap)
- `game/`: Game logic (dungeon, combat, skills, turns, entities)
- `assets/`: Sprites, tilesets, maps
- `third_party/`: External dependencies (SDL2, stb, pugixml)
- `main.cpp`: Entry point
- `CMakeLists.txt`: Build system

## Build Instructions

### Requirements
- C++17 or later
- SDL2 development libraries
- CMake 3.10+

### Build
```sh
mkdir build
cd build
cmake ..
make
./moravor
```

## Notes
- Asset and third_party directories are placeholders; add actual dependencies as needed.
- Flatpak and advanced modules to be added later.
