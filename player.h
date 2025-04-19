#pragma once
#include "level.h"
#include <SDL.h>

struct Player {
    int x, y;
    int dir; // 0=N,1=E,2=S,3=W
};

void player_init(Player& player);
void player_move(Player& player, int dx, int dy);
void player_turn(Player& player, int dir_delta);
