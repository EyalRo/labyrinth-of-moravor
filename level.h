#pragma once
#include <vector>
#include <string>

// Level dimensions and tile definitions
extern int MAP_W, MAP_H;
extern std::vector<std::string> level_data;

// Track current floor index (0 = static, >=1 = random)
int get_current_floor();
void set_current_floor(int idx);

// Set level data (for random floors)
void set_level_data(const std::vector<std::string>& data);

// Get entrance/exit positions
std::pair<int,int> get_entrance_pos();
std::pair<int,int> get_exit_pos();

// Tile types
constexpr char TILE_FLOOR = '.';
constexpr char TILE_WALL = '#';
constexpr char TILE_ENTRANCE = 'E'; // Entrance tile (green)
constexpr char TILE_EXIT = 'X';     // Exit tile (maroon)

// Level API
char get_tile(int x, int y);
bool is_walkable(char tile);
