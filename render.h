#pragma once
#include <SDL.h>
#include "player.h"

// Texture pointers for dungeon rendering
extern SDL_Texture* g_wall_tex;
extern SDL_Texture* g_floor_tex;
extern SDL_Texture* g_item_tex;

// Load and free textures
bool load_dungeon_textures(SDL_Renderer* ren);
void free_dungeon_textures();

// Raycasting-based dungeon renderer
void render_dungeon(SDL_Renderer* ren, const Player& player, int win_w, int top_h, int bottom_h);

// Renders the minimap (stub)
void render_minimap(SDL_Renderer* ren, const Player& player, int win_w, int top_h, int bottom_h);
