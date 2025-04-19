#include "level.h"
#include <cstring>

#include <vector>
#include <string>
#include <utility>
#include <cassert>

int MAP_W = 16, MAP_H = 14;
std::vector<std::string> level_data = {
    "################",
    "#..............#",
    "#..##..##..##..#",
    "#..#....#..#...#",
    "#..#....#..#...#",
    "#..###.##..#...#",
    "#..............#",
    "#.####.###.##..#",
    "#..............#",
    "#..##..##..##..#",
    "#..#....#..#...#",
    "#..#....#..#...#",
    "#..###.##..#...#",
    "##############D#"
};

static int current_floor = 0;
static std::pair<int,int> entrance_pos = {1,1};
static std::pair<int,int> exit_pos = {14,13};

int get_current_floor() { return current_floor; }
void set_current_floor(int idx) { current_floor = idx; }

void set_level_data(const std::vector<std::string>& data) {
    assert(!data.empty());
    level_data = data;
    MAP_H = level_data.size();
    MAP_W = level_data[0].size();
    // Find entrance/exit
    entrance_pos = {-1,-1};
    exit_pos = {-1,-1};
    // Find entrance ('E') and exit ('X') tiles
    entrance_pos = {-1, -1};
    exit_pos = {-1, -1};
    for (int y = 0; y < MAP_H; ++y) {
        for (int x = 0; x < MAP_W; ++x) {
            if (level_data[y][x] == TILE_ENTRANCE) entrance_pos = {x, y};
            if (level_data[y][x] == TILE_EXIT) exit_pos = {x, y};
        }
    }
}

std::pair<int,int> get_entrance_pos() { return entrance_pos; }
std::pair<int,int> get_exit_pos() { return exit_pos; }

char get_tile(int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return TILE_WALL;
    return level_data[y][x];
}

bool is_walkable(char tile) {
    return tile == TILE_FLOOR;
}
