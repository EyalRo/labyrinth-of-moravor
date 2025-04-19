#pragma once
#include <vector>
#include <string>

// Level dimensions and tile definitions
extern const int MAP_W, MAP_H;
extern const char* level_data[];

// Tile types
constexpr char TILE_FLOOR = '.';
constexpr char TILE_WALL = '#';
constexpr char TILE_DOORWAY = 'D'; // Special: not walkable, can be activated when facing

// Level API
char get_tile(int x, int y);
bool is_walkable(char tile);
