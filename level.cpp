#include "level.h"
#include <cstring>

const int MAP_W = 16, MAP_H = 14;
const char* level_data[MAP_H] = {
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

char get_tile(int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return TILE_WALL;
    return level_data[y][x];
}

bool is_walkable(char tile) {
    return tile == TILE_FLOOR;
}
