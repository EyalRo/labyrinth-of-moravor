#pragma once
#include "level.h"
#include <SDL.h>
#include <string>

// Monster state enum
enum class MonsterState { Idle, Agro, Dead };

struct Monster {
    int x, y;
    int dir; // 0=N,1=E,2=S,3=W
    MonsterState state;
};

struct Player {
    std::string name;
    int x, y;
    int dir; // 0=N,1=E,2=S,3=W
    // RPG stats
    int hp;
    int max_hp;
    int attack;
    int defense;
    int agility;
    // Future: skills, inventory, etc.
};

struct Party {
    Player members[3]; // up to 3 characters
    int count;         // number of active members
};

void player_init(Player& player);
void player_move(Player& player, int dx, int dy);
void player_turn(Player& player, int dir_delta);

// Combat system stubs
int player_attack(Player& attacker, Player& defender);
// Add more combat functions as needed
