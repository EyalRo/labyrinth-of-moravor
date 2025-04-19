#include "player.h"

void player_init(Player& player) {
    player.x = 1;
    player.y = 1;
    player.dir = 1; // EAST
}

void player_move(Player& player, int dx, int dy) {
    int nx = player.x + dx, ny = player.y + dy;
    char tile = get_tile(nx, ny);
    if (is_walkable(tile)) {
        player.x = nx; player.y = ny;
    }
}

void player_turn(Player& player, int dir_delta) {
    player.dir = (player.dir + dir_delta + 4) % 4;
}
