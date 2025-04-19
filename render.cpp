#include "render.h"
#include "level.h"
#include "player.h"
#include <SDL.h>
#include <SDL_image.h>
#include <algorithm>
#include <cmath>
#include <iostream>

// Global texture pointers
SDL_Texture *g_wall_tex = nullptr;
SDL_Texture *g_floor_tex = nullptr;
SDL_Texture *g_item_tex = nullptr;

// Load textures from assets/
bool load_dungeon_textures(SDL_Renderer *ren) {
  auto load_tex = [&](const char *path) -> SDL_Texture * {
    SDL_Surface *surf = IMG_Load(path);
    if (!surf) {
      std::cerr << "IMG_Load failed: " << path << " - " << IMG_GetError()
                << std::endl;
      return nullptr;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);
    if (!tex) {
      std::cerr << "SDL_CreateTextureFromSurface failed: " << path << " - "
                << SDL_GetError() << std::endl;
    }
    return tex;
  };
  g_wall_tex = load_tex("assets/wall.png");
  g_floor_tex = load_tex("assets/floor.png");
  g_item_tex = load_tex("assets/item.png");
  return g_wall_tex && g_floor_tex && g_item_tex;
}

void free_dungeon_textures() {
  if (g_wall_tex)
    SDL_DestroyTexture(g_wall_tex);
  g_wall_tex = nullptr;
  if (g_floor_tex)
    SDL_DestroyTexture(g_floor_tex);
  g_floor_tex = nullptr;
  if (g_item_tex)
    SDL_DestroyTexture(g_item_tex);
  g_item_tex = nullptr;
}

// Raycasting-based dungeon renderer (Wolfenstein style)
void render_dungeon(SDL_Renderer *ren, const Player &player, int win_w,
                    int top_h, int /*bottom_h*/) {
  // Colors
  SDL_Color ceil = {0, 0, 60, 255}; // Darker blue

  // Fill background (ceiling)
  SDL_SetRenderDrawColor(ren, ceil.r, ceil.g, ceil.b, ceil.a);
  SDL_Rect rect = {0, 0, win_w, top_h / 2};
  SDL_RenderFillRect(ren, &rect);
  // Fill floor with texture (repeat)
  if (g_floor_tex) {
    int tex_w, tex_h;
    SDL_QueryTexture(g_floor_tex, nullptr, nullptr, &tex_w, &tex_h);
    for (int x = 0; x < win_w; x += tex_w) {
      for (int y = top_h / 2; y < top_h; y += tex_h) {
        SDL_Rect dst = {x, y, tex_w, tex_h};
        SDL_RenderCopy(ren, g_floor_tex, nullptr, &dst);
      }
    }
  } else {
    SDL_SetRenderDrawColor(ren, 30, 30, 60, 255);
    rect = {0, top_h / 2, win_w, top_h / 2};
    SDL_RenderFillRect(ren, &rect);
  }

  // Raycasting parameters
  float fov = M_PI / 3.0f; // 60 degrees
  float cam_x, ray_dir_x, ray_dir_y;
  float pos_x = player.x + 0.5f;
  float pos_y = player.y + 0.5f;
  float dir_x, dir_y;
  // Cardinal directions: 0=N,1=E,2=S,3=W
  switch (player.dir) {
  case 0:
    dir_x = 0;
    dir_y = -1;
    break;
  case 1:
    dir_x = 1;
    dir_y = 0;
    break;
  case 2:
    dir_x = 0;
    dir_y = 1;
    break;
  case 3:
    dir_x = -1;
    dir_y = 0;
    break;
  default:
    dir_x = 0;
    dir_y = -1;
    break;
  }
  // Camera plane perpendicular to dir
  float plane_x = -dir_y * tanf(fov / 2);
  float plane_y = dir_x * tanf(fov / 2);

  for (int x = 0; x < win_w; ++x) {
    cam_x = 2.0f * x / win_w - 1.0f;
    ray_dir_x = dir_x + plane_x * cam_x;
    ray_dir_y = dir_y + plane_y * cam_x;
    int map_x = (int)pos_x;
    int map_y = (int)pos_y;
    float side_dist_x, side_dist_y;
    float delta_dist_x = (ray_dir_x == 0) ? 1e30 : fabs(1.0f / ray_dir_x);
    float delta_dist_y = (ray_dir_y == 0) ? 1e30 : fabs(1.0f / ray_dir_y);
    float perp_wall_dist;
    int step_x, step_y;
    int hit = 0, side = 0;
    // Calculate step and initial sideDist
    if (ray_dir_x < 0) {
      step_x = -1;
      side_dist_x = (pos_x - map_x) * delta_dist_x;
    } else {
      step_x = 1;
      side_dist_x = (map_x + 1.0f - pos_x) * delta_dist_x;
    }
    if (ray_dir_y < 0) {
      step_y = -1;
      side_dist_y = (pos_y - map_y) * delta_dist_y;
    } else {
      step_y = 1;
      side_dist_y = (map_y + 1.0f - pos_y) * delta_dist_y;
    }
    // Perform DDA
    while (!hit) {
      if (side_dist_x < side_dist_y) {
        side_dist_x += delta_dist_x;
        map_x += step_x;
        side = 0;
      } else {
        side_dist_y += delta_dist_y;
        map_y += step_y;
        side = 1;
      }
      if (get_tile(map_x, map_y) == TILE_WALL)
        hit = 1;
    }
    // Calculate distance to wall
    if (side == 0)
      perp_wall_dist = (side_dist_x - delta_dist_x);
    else
      perp_wall_dist = (side_dist_y - delta_dist_y);
    // Calculate height of line to draw
    int line_height = (int)(top_h / (perp_wall_dist + 1e-6));
    int draw_start = -line_height / 2 + top_h / 2;
    int draw_end = line_height / 2 + top_h / 2;
    // Clamp to ceiling/floor bounds
    if (draw_start < 0)
      draw_start = 0;
    if (draw_end > top_h)
      draw_end = top_h;
    // Draw the vertical wall slice
    if (g_wall_tex) {
      int tex_w, tex_h;
      SDL_QueryTexture(g_wall_tex, nullptr, nullptr, &tex_w, &tex_h);
      // Calculate texture X coordinate (simple, based on wall hit location)
      float wall_x;
      if (side == 0)
        wall_x = pos_y + perp_wall_dist * ray_dir_y;
      else
        wall_x = pos_x + perp_wall_dist * ray_dir_x;
      wall_x -= floorf(wall_x);
      int tex_x = int(wall_x * tex_w);
      if ((side == 0 && ray_dir_x > 0) || (side == 1 && ray_dir_y < 0))
        tex_x = tex_w - tex_x - 1;
      SDL_Rect src = {tex_x, 0, 1, tex_h};
      SDL_Rect dst = {x, draw_start, 1, draw_end - draw_start};
      SDL_RenderCopy(ren, g_wall_tex, &src, &dst);
    } else {
      SDL_SetRenderDrawColor(ren, 180, 180, 180, 255);
      SDL_RenderDrawLine(ren, x, draw_start, x, draw_end);
    }
  }

  // Line of sight check using Bresenham's algorithm
  auto has_line_of_sight = [](int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    while (x0 != x1 || y0 != y1) {
      if (x0 == x1 && y0 == y1)
        break;
      if (x0 != x1 || y0 != y1) {
        if (level_data[y0][x0] == TILE_WALL)
          return false;
      }
      int e2 = 2 * err;
      if (e2 > -dy) {
        err -= dy;
        x0 += sx;
      }
      if (e2 < dx) {
        err += dx;
        y0 += sy;
      }
    }
    return true;
  };

  // Find entrance/exit positions
  int ex = -1, ey = -1, xx = -1, xy = -1;
  for (int j = 0; j < MAP_H; ++j)
    for (int i = 0; i < MAP_W; ++i) {
      if (level_data[j][i] == TILE_ENTRANCE) {
        ex = i;
        ey = j;
      }
      if (level_data[j][i] == TILE_EXIT) {
        xx = i;
        xy = j;
      }
    }
  // Classic raycasting sprite rendering for objects (Wolfenstein/Doom style)
  struct Sprite {
    SDL_Texture *tex;
    float obj_x, obj_y; // world position
    float dist;
  };
  std::vector<Sprite> sprites;
  // Collect entrance/exit (add items as needed)
  if (ex >= 0 && ey >= 0)
    sprites.push_back({g_item_tex, float(ex) + 0.5f, float(ey) + 0.5f, 0});
  if (xx >= 0 && xy >= 0)
    sprites.push_back({g_item_tex, float(xx) + 0.5f, float(xy) + 0.5f, 0});
  // Compute distance from player for sorting
  for (auto &s : sprites) {
    float dx = s.obj_x - pos_x;
    float dy = s.obj_y - pos_y;
    s.dist = dx * dx + dy * dy;
  }
  // Sort farthest to nearest
  std::sort(sprites.begin(), sprites.end(),
            [](const Sprite &a, const Sprite &b) { return a.dist > b.dist; });
  // Camera vectors
  // Use existing cam_x, cam_y, cam_dir_x, cam_dir_y, plane_x, plane_y
  // For each sprite
  for (const auto &s : sprites) {
    // Only render if player has line of sight to the sprite's tile
    if (!has_line_of_sight(player.x, player.y, int(s.obj_x), int(s.obj_y)))
      continue;
    // Use a robust threshold for 'same tile' (within 0.3 units)
    // Special rendering for same-tile objects: much larger than one-tile-away,
    // base-aligned
    if (std::abs(s.obj_x - pos_x) < 0.3f && std::abs(s.obj_y - pos_y) < 0.3f) {
      // Make the sprite extremely large for same-tile
      int tile_sprite_h = int(top_h * 6.0f);
      int tile_sprite_w = tile_sprite_h;
      int tile_draw_start_y = top_h - tile_sprite_h;
      // Clamp only if needed, always keep base at bottom
      if (tile_draw_start_y < 0) {
        tile_sprite_h += tile_draw_start_y; // reduce height
        tile_sprite_w = tile_sprite_h;
        tile_draw_start_y = 0;
      }
      int tile_draw_start_x = (win_w - tile_sprite_w) / 2;
      SDL_Rect dst = {tile_draw_start_x, tile_draw_start_y, tile_sprite_w,
                      tile_sprite_h};
      if (dst.h > 0 && dst.w > 0)
        SDL_RenderCopy(ren, s.tex, nullptr, &dst);
      continue;
    }
    // Classic projection for remote tiles
    float rel_x = s.obj_x - pos_x;
    float rel_y = s.obj_y - pos_y;
    // Transform to camera space
    float inv_det = 1.0f / (plane_x * dir_y - dir_x * plane_y);
    float trans_x = inv_det * (dir_y * rel_x - dir_x * rel_y);
    float trans_y = inv_det * (-plane_y * rel_x + plane_x * rel_y);
    if (trans_y <= 0)
      continue; // behind camera
    // Projected screen position
    int sprite_screen_x = int((win_w / 2) * (1 + trans_x / trans_y));
    // Projected height
    int sprite_h = abs(int(top_h / trans_y));
    int sprite_w = sprite_h;
    int draw_start_x = sprite_screen_x - sprite_w / 2;
    int draw_start_y = (top_h - sprite_h) / 2;
    SDL_Rect dst = {draw_start_x, draw_start_y, sprite_w, sprite_h};
    // Clamp to main view
    if (dst.y < 0) {
      dst.h += dst.y;
      dst.y = 0;
    }
    if (dst.y + dst.h > top_h)
      dst.h = top_h - dst.y;
    if (dst.x < 0) {
      dst.w += dst.x;
      dst.x = 0;
    }
    if (dst.x + dst.w > win_w)
      dst.w = win_w - dst.x;
    if (dst.h > 0 && dst.w > 0)
      SDL_RenderCopy(ren, s.tex, nullptr, &dst);
  }
}

// Minimap overlay in bottom right
void render_minimap(SDL_Renderer *ren, const Player &player, int win_w,
                    int top_h, int bottom_h) {
  // Minimap size
  const int map_w = 120, map_h = 120;
  const int cell_w = map_w / MAP_W;
  const int cell_h = map_h / MAP_H;
  const int margin = 12;
  int x0 = win_w - map_w - margin;
  int y0 = top_h + bottom_h - map_h - margin;

  // Semi-transparent background
  SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(ren, 0, 0, 0, 128);
  SDL_Rect bg = {x0 - 4, y0 - 4, map_w + 8, map_h + 8};
  SDL_RenderFillRect(ren, &bg);
  SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);

  // Draw map tiles
  for (int j = 0; j < MAP_H; ++j) {
    for (int i = 0; i < MAP_W; ++i) {
      SDL_Rect cell = {x0 + i * cell_w, y0 + j * cell_h, cell_w - 1,
                       cell_h - 1};
      if (level_data[j][i] == TILE_WALL) {
        SDL_SetRenderDrawColor(ren, 80, 80, 80, 255);
      } else if (level_data[j][i] == TILE_ENTRANCE) {
        SDL_SetRenderDrawColor(ren, 0, 200, 0, 255);
      } else if (level_data[j][i] == TILE_EXIT) {
        SDL_SetRenderDrawColor(ren, 200, 0, 0, 255);
      } else {
        SDL_SetRenderDrawColor(ren, 160, 160, 160, 255);
      }
      SDL_RenderFillRect(ren, &cell);
    }
  }
  // Draw player
  int px = x0 + int((player.x + 0.5f) * cell_w);
  int py = y0 + int((player.y + 0.5f) * cell_h);
  SDL_SetRenderDrawColor(ren, 255, 255, 0, 255);
  SDL_Rect pcell = {px - 3, py - 3, 6, 6};
  SDL_RenderFillRect(ren, &pcell);
  // Draw facing direction
  float dx = 0, dy = 0;
  switch (player.dir) {
  case 0:
    dy = -1;
    break;
  case 1:
    dx = 1;
    break;
  case 2:
    dy = 1;
    break;
  case 3:
    dx = -1;
    break;
  }
  int fx = px + int(dx * 10), fy = py + int(dy * 10);
  SDL_RenderDrawLine(ren, px, py, fx, fy);
}
