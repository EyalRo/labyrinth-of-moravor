#include "player.h"

void player_init(Player& player) {
    player.name = "Hero";
    player.x = 1;
    player.y = 1;
    player.dir = 1; // EAST
    player.max_hp = 30;
    player.hp = 30;
    player.attack = 8;
    player.defense = 5;
    player.agility = 4;
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

// Simple attack: returns damage dealt
int player_attack(Player& attacker, Player& defender) {
    int base = attacker.attack - defender.defense;
    if (base < 1) base = 1;
    defender.hp -= base;
    if (defender.hp < 0) defender.hp = 0;
    return base;
}
