#include "level.h"
#include <cstring>

const int MAP_W = 12, MAP_H = 10;
const char* level_data[MAP_H] = {
    "############",
    "#E........X#",
    "#..##..##..#",
    "#..#....#..#",
    "#..#....#..#",
    "#..###.##..#",
    "#..........#",
    "#.####.###.#",
    "#..........#",
    "############"
};

void find_entrance(int& x, int& y) {
    for (int j = 0; j < MAP_H; ++j)
        for (int i = 0; i < MAP_W; ++i)
            if (level_data[j][i] == TILE_ENTRANCE) { x = i; y = j; return; }
}

char get_tile(int x, int y) {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) return TILE_WALL;
    return level_data[y][x];
}

bool is_walkable(char tile) {
    return tile == TILE_FLOOR || tile == TILE_ENTRANCE || tile == TILE_EXIT;
}
