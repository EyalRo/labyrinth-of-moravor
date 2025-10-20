#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include "player.h"

// Texture pointers for dungeon rendering
extern SDL_Texture* g_wall_tex;
extern SDL_Texture* g_floor_tex;
extern SDL_Texture* g_item_tex;

// Load and free textures
bool load_dungeon_textures(SDL_Renderer* ren);
void free_dungeon_textures();

// Raycasting-based dungeon renderer
void render_dungeon(SDL_Renderer* ren, const Player& player, const Monster* monster, int win_w, int top_h, int bottom_h);

// Renders the minimap (stub)
void render_minimap(SDL_Renderer* ren, const Player& player, int win_w, int top_h, int bottom_h);

// Draws party/status area under the window
// Stores the rectangles for attack buttons for each party member
extern SDL_Rect g_attack_btn_rects[3];

// Render party status and update attack button rects
void render_party_status(SDL_Renderer* ren, const Party& party, TTF_Font* font, int win_w, int top_h, int bottom_h);

// Returns the rectangle for the attack button for the given member index (0-2)
static inline SDL_Rect get_attack_btn_rect(int idx) { return g_attack_btn_rects[idx]; }
