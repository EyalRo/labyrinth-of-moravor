#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <iostream>
#include "level.h"
#include "player.h"
#include "render.h"
#include "random_floor.h"
#include <vector>

struct FloorData {
    std::vector<std::string> map;
    std::pair<int,int> entrance, exit;
};
static std::vector<FloorData> floors;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    const char* fontPath = "/usr/share/fonts/TTF/DejaVuSerifCondensed.ttf";
    TTF_Font* font = TTF_OpenFont(fontPath, 32);
    if (!font) {
        std::cerr << "Failed to load font: " << fontPath << ". TTF_Error: " << TTF_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_Window* win = SDL_CreateWindow("Labyrinth of Moravor", 100, 100, 800, 600, SDL_WINDOW_SHOWN);
    if (!win) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    // Load main menu background
    SDL_Texture* menu_bg_tex = nullptr;
    {
        SDL_Surface* surf = IMG_Load("assets/Labyrinth_of_Moravor_Cover_800x600.png");
        if (!surf) {
            std::cerr << "IMG_Load failed for menu background: " << IMG_GetError() << std::endl;
        } else {
            menu_bg_tex = SDL_CreateTextureFromSurface(ren, surf);
            SDL_FreeSurface(surf);
            if (!menu_bg_tex) {
                std::cerr << "SDL_CreateTextureFromSurface failed for menu background: " << SDL_GetError() << std::endl;
            }
        }
    }
    if (!load_dungeon_textures(ren)) {
        std::cerr << "Failed to load dungeon textures!" << std::endl;
        // Cleanup order: free textures, quit IMG, destroy renderer/window, quit SDL
        free_dungeon_textures();
        IMG_Quit();
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    enum MenuOption { MENU_START, MENU_QUIT, MENU_COUNT };
    const char* menu_labels[MENU_COUNT] = {"Start Game", "Quit"};
    int selected = MENU_START;
    bool quit = false;
    bool in_menu = true;
    SDL_Event e;
    int mouse_x = 0, mouse_y = 0;
    bool in_game = false;
    // --- Player and Level State ---
    // Initialize persistent floors
    floors.clear();
    // Static map as floor 0
    // Static map with correct exit position
    std::vector<std::string> static_map = {
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
        "##############X#"
    };
    // Find exit position for static map
    std::pair<int,int> static_exit = {-1, -1};
    for (int y = 0; y < (int)static_map.size(); ++y) {
        for (int x = 0; x < (int)static_map[0].size(); ++x) {
            if (static_map[y][x] == 'D') static_exit = {x, y};
        }
    }
    floors.push_back(FloorData{
        static_map,
        {1,1}, static_exit
    });
    set_level_data(floors[0].map);

    Player player;
    player_init(player);


    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
            else if (in_menu && e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        selected = (selected + MENU_COUNT - 1) % MENU_COUNT;
                        break;
                    case SDLK_DOWN:
                        selected = (selected + 1) % MENU_COUNT;
                        break;
                    case SDLK_RETURN:
                    case SDLK_KP_ENTER:
                        if (selected == MENU_START) {
                            in_menu = false;
                            in_game = true;
                        } else if (selected == MENU_QUIT) {
                            quit = true;
                        }
                        break;
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                }
            } else if (in_menu && e.type == SDL_MOUSEMOTION) {
                mouse_x = e.motion.x;
                mouse_y = e.motion.y;
            } else if (in_menu && e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    const int menu_x = 300, menu_y = 200, menu_w = 200, menu_h = 60;
                    for (int i = 0; i < MENU_COUNT; ++i) {
                        SDL_Rect item = { menu_x, menu_y + i * (menu_h + 20), menu_w, menu_h };
                        if (mouse_x >= item.x && mouse_x < item.x + item.w &&
                            mouse_y >= item.y && mouse_y < item.y + item.h) {
                            selected = i;
                            if (selected == MENU_START) {
                                in_menu = false;
                                in_game = true;
                            } else if (selected == MENU_QUIT) {
                                quit = true;
                            }
                        }
                    }
                }
            } else if (in_game && e.type == SDL_KEYDOWN) {
                // --- GAME CONTROLS ---
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    in_game = false;
                    in_menu = true;
                } else if (e.key.keysym.sym == SDLK_LEFT) {
                    player_turn(player, -1);
                } else if (e.key.keysym.sym == SDLK_RIGHT) {
                    player_turn(player, 1);
                } else if (e.key.keysym.sym == SDLK_UP) {
                    // Move forward in facing direction
                    static const int dx[4] = {0, 1, 0, -1};
                    static const int dy[4] = {-1, 0, 1, 0};
                    player_move(player, dx[player.dir], dy[player.dir]);
                } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    // Check if facing doorway
                    static const int dx[4] = {0, 1, 0, -1};
                    static const int dy[4] = {-1, 0, 1, 0};
                    int nx = player.x + dx[player.dir];
                    int ny = player.y + dy[player.dir];
                    int floor = get_current_floor();
                    std::pair<int,int> curr_entrance = get_entrance_pos();
                    std::pair<int,int> curr_exit = get_exit_pos();
                    char tile = get_tile(nx, ny);
                    if (tile == TILE_EXIT || tile == TILE_ENTRANCE) {
                        if (tile == TILE_EXIT) {
                            // Max floor check
                            if (floor + 1 >= 10) {
                                // End game, return to main menu
                                in_game = false;
                                in_menu = true;
                                continue;
                            }
                            // Go to next floor (persist)
                            set_current_floor(floor + 1);
                            if (floor + 1 < (int)floors.size()) {
                                // Already generated, just load
                                set_level_data(floors[floor + 1].map);
                                // Always place player by entrance of new floor, facing away from it
                                std::pair<int,int> entrance = floors[floor + 1].entrance;
                                int dx[4] = {0,1,0,-1}, dy[4] = {-1,0,1,0};
                                int ex = entrance.first, ey = entrance.second;
                                for (int d = 0; d < 4; ++d) {
                                    int px = ex + dx[d], py = ey + dy[d];
                                    if (px >= 0 && px < MAP_W && py >= 0 && py < MAP_H && get_tile(px, py) == TILE_FLOOR) {
                                        player.x = px;
                                        player.y = py;
                                        player.dir = d; // face the direction of the doorway (opposite of entry)
                                        break;
                                    }
                                }
                            } else {
                                // Generate new floor
                                std::pair<int,int> entrance, exitp;
                                std::vector<std::string> next_map = generate_random_floor(MAP_W, MAP_H, entrance, exitp);
                                floors.push_back(FloorData{next_map, entrance, exitp});
                                set_level_data(next_map);
                                // Place player by entrance
                                int dx[4] = {0,1,0,-1}, dy[4] = {-1,0,1,0};
                                int ex = entrance.first, ey = entrance.second;
                                for (int d = 0; d < 4; ++d) {
                                    int px = ex + dx[d], py = ey + dy[d];
                                    if (px >= 0 && px < MAP_W && py >= 0 && py < MAP_H && get_tile(px, py) == TILE_FLOOR) {
                                        player.x = px;
                                        player.y = py;
                                        player.dir = d; // face the direction of the doorway (opposite of entry)
                                        break;
                                    }
                                }
                            }
                        } else if (get_tile(nx, ny) == TILE_ENTRANCE) {
                            // Go to previous floor
                            if (floor == 0) {
                                // First level has no entrance, do nothing
                                break;
                            }
                            set_current_floor(floor - 1);
                            if (floor - 1 == 0) {
                                // Static map
                                set_level_data(floors[0].map);
                                // Place player adjacent to exit doorway, facing inward
                                std::pair<int,int> static_exit = get_exit_pos();
                                int ex = static_exit.first, ey = static_exit.second;
                                int px = ex, py = ey;
                                // Try all 4 inward directions
                                if (ex > 0 && get_tile(ex-1, ey) == TILE_FLOOR) { px = ex-1; py = ey; player.dir = 1; } // face east (came from west)
                                else if (ex < MAP_W-1 && get_tile(ex+1, ey) == TILE_FLOOR) { px = ex+1; py = ey; player.dir = 3; } // face west (came from east)
                                else if (ey > 0 && get_tile(ex, ey-1) == TILE_FLOOR) { px = ex; py = ey-1; player.dir = 0; } // face north (came from south)
                                else if (ey < MAP_H-1 && get_tile(ex, ey+1) == TILE_FLOOR) { px = ex; py = ey+1; player.dir = 2; } // face south (came from north)
                                player.x = px;
                                player.y = py;
                            } else {
                                // For random floors, load from persistent vector
                                set_level_data(floors[floor - 1].map);
                                std::pair<int,int> prev_exit = floors[floor - 1].exit;
                                int ex = prev_exit.first, ey = prev_exit.second;
                                for (int d = 0; d < 4; ++d) {
                                    int dx[4] = {0,1,0,-1}, dy[4] = {-1,0,1,0};
                                    int px = ex + dx[d], py = ey + dy[d];
                                    if (px >= 0 && px < MAP_W && py >= 0 && py < MAP_H && get_tile(px, py) == TILE_FLOOR) {
                                        player.x = px;
                                        player.y = py;
                                        player.dir = d; // face the direction of the doorway (opposite of entry)
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
        // --- Doorway indicator logic ---
        bool show_doorway_indicator = false;
        if (in_game) {
            static const int dx[4] = {0, 1, 0, -1};
            static const int dy[4] = {-1, 0, 1, 0};
            int nx = player.x + dx[player.dir];
            int ny = player.y + dy[player.dir];
            char facing = get_tile(nx, ny);

            if (facing == TILE_ENTRANCE || facing == TILE_EXIT) {
                show_doorway_indicator = true;
            }
        }
        // Mouse hover highlights (menu only)
        if (in_menu) {
            const int menu_x = 300, menu_y = 200, menu_w = 200, menu_h = 60;
            for (int i = 0; i < MENU_COUNT; ++i) {
                SDL_Rect item = { menu_x, menu_y + i * (menu_h + 20), menu_w, menu_h };
                if (mouse_x >= item.x && mouse_x < item.x + item.w &&
                    mouse_y >= item.y && mouse_y < item.y + item.h) {
                    selected = i;
                }
            }
        }
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        // Draw floor number in top left when in game
        if (in_game) {
            char level_buf[32];
            snprintf(level_buf, sizeof(level_buf), "Floor: %d", get_current_floor());
            SDL_Color white = {255,255,255,255};
            SDL_Surface* surf = TTF_RenderText_Blended(font, level_buf, white);
            if (surf) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
                SDL_Rect rect = {20, 20, surf->w, surf->h};
                SDL_RenderCopy(ren, tex, nullptr, &rect);
                SDL_FreeSurface(surf);
                SDL_DestroyTexture(tex);
            }
        }
        if (in_menu) {
            // Draw menu background image if loaded
            if (menu_bg_tex) {
                SDL_Rect bg_rect = {0, 0, 800, 600};
                SDL_RenderCopy(ren, menu_bg_tex, nullptr, &bg_rect);
            }
            // Draw menu
            const int menu_x = 300, menu_y = 200, menu_w = 200, menu_h = 60;
            for (int i = 0; i < MENU_COUNT; ++i) {
                SDL_Rect item = { menu_x, menu_y + i * (menu_h + 20), menu_w, menu_h };
                if (i == selected) {
                    SDL_SetRenderDrawColor(ren, 200, 200, 50, 255); // Highlighted
                } else {
                    SDL_SetRenderDrawColor(ren, 80, 80, 80, 255); // Normal
                }
                SDL_RenderFillRect(ren, &item);
                // Render text
                SDL_Color fg = {255,255,255,255};
                SDL_Surface* surf = TTF_RenderUTF8_Blended(font, menu_labels[i], fg);
                if (surf) {
                    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
                    if (tex) {
                        int tw = surf->w, th = surf->h;
                        SDL_Rect dst = {item.x + (item.w-tw)/2, item.y + (item.h-th)/2, tw, th};
                        SDL_RenderCopy(ren, tex, nullptr, &dst);
                        SDL_DestroyTexture(tex);
                    }
                    SDL_FreeSurface(surf);
                }
            }
        } else if (in_game) {
            int win_w = 0, win_h = 0;
            SDL_GetWindowSize(win, &win_w, &win_h);
            int top_h = win_h * 0.6;
            int bottom_h = win_h - top_h;
            render_dungeon(ren, player, win_w, top_h, bottom_h);
            render_minimap(ren, player, win_w, top_h, bottom_h);
            // Draw doorway indicator if needed
            if (show_doorway_indicator && font) {
                const char* msg = "Press Enter to Enter Doorway";
                SDL_Color fg = {255, 255, 64, 255};
                SDL_Surface* surf = TTF_RenderUTF8_Blended(font, msg, fg);
                if (surf) {
                    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
                    if (tex) {
                        int tw = surf->w, th = surf->h;
                        SDL_Rect dst = { (win_w-tw)/2, 32, tw, th };
                        SDL_RenderCopy(ren, tex, nullptr, &dst);
                        SDL_DestroyTexture(tex);
                    }
                    SDL_FreeSurface(surf);
                }
            }
        }
        SDL_RenderPresent(ren);
    }
    // Cleanup resources in reverse order of creation
    // Free menu background texture
    if (menu_bg_tex) SDL_DestroyTexture(menu_bg_tex);
    TTF_CloseFont(font);
    TTF_Quit();
    free_dungeon_textures();
    IMG_Quit();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
