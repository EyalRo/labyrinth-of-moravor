#include "random_floor.h"
#include "level.h"
#include <random>
#include <algorithm>
#include <queue>

// Directions: N, E, S, W
static const int dx[4] = {0, 1, 0, -1};
static const int dy[4] = {-1, 0, 1, 0};

// Helper: carve a path using randomized DFS
static void carve_path(std::vector<std::string>& map, int x, int y, int ex, int ey, std::mt19937& rng, std::vector<std::vector<bool>>& visited) {
    int w = map[0].size(), h = map.size();
    visited[y][x] = true;
    if (x == ex && y == ey) return;
    std::vector<int> dirs = {0, 1, 2, 3};
    std::shuffle(dirs.begin(), dirs.end(), rng);
    for (int d : dirs) {
        int nx = x + dx[d], ny = y + dy[d];
        if (nx < 1 || nx >= w-1 || ny < 1 || ny >= h-1) continue;
        if (!visited[ny][nx]) {
            map[ny][nx] = '.';
            carve_path(map, nx, ny, ex, ey, rng, visited);
        }
    }
}

std::vector<std::string> generate_random_floor(int w, int h, std::pair<int,int>& entrance_pos, std::pair<int,int>& exit_pos, unsigned int seed) {
    std::mt19937 rng(seed ? seed : std::random_device{}());
    std::vector<std::string> map(h, std::string(w, '#'));
    // 1. Place random rooms
    int num_rooms = 3 + rng() % 4;
    std::vector<std::pair<int,int>> room_centers;
    for (int r = 0; r < num_rooms; ++r) {
        int rw = 3 + rng() % 4; // room width
        int rh = 3 + rng() % 4; // room height
        int rx = 1 + rng() % (w - rw - 1);
        int ry = 1 + rng() % (h - rh - 1);
        for (int y = ry; y < ry+rh; ++y)
            for (int x = rx; x < rx+rw; ++x)
                map[y][x] = '.';
        room_centers.emplace_back(rx+rw/2, ry+rh/2);
    }
    // 2. Maze/hallways using randomized DFS from first room center
    std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));
    auto maze_carve = [&](int x, int y, auto&& self) -> void {
        visited[y][x] = true;
        std::vector<int> dirs = {0,1,2,3};
        std::shuffle(dirs.begin(), dirs.end(), rng);
        for (int d : dirs) {
            int nx = x + dx[d]*2, ny = y + dy[d]*2;
            if (nx > 0 && nx < w-1 && ny > 0 && ny < h-1 && !visited[ny][nx]) {
                map[y + dy[d]][x + dx[d]] = '.';
                map[ny][nx] = '.';
                self(nx, ny, self);
            }
        }
    };
    int startx = room_centers[0].first, starty = room_centers[0].second;
    maze_carve(startx, starty, maze_carve);
    // 3. Connect all rooms
    for (size_t i = 1; i < room_centers.size(); ++i) {
        int x0 = room_centers[i-1].first, y0 = room_centers[i-1].second;
        int x1 = room_centers[i].first, y1 = room_centers[i].second;
        while (x0 != x1 || y0 != y1) {
            if (x0 < x1) ++x0;
            else if (x0 > x1) --x0;
            else if (y0 < y1) ++y0;
            else if (y0 > y1) --y0;
            map[y0][x0] = '.';
        }
    }
    // 4. Randomize entrance/exit on outer wall (not corners)
    auto pick_wall_pos = [&](int& x, int& y) {
        int wall = rng()%4;
        if (wall == 0) { // top
            y = 0; x = 1 + rng()%(w-2);
        } else if (wall == 1) { // bottom
            y = h-1; x = 1 + rng()%(w-2);
        } else if (wall == 2) { // left
            x = 0; y = 1 + rng()%(h-2);
        } else { // right
            x = w-1; y = 1 + rng()%(h-2);
        }
    };
    int ex1, ey1, ex2, ey2;
    pick_wall_pos(ex1, ey1);
    pick_wall_pos(ex2, ey2);
    // Ensure entrance and exit are not the same and not too close
    while ((ex1 == ex2 && ey1 == ey2) || (abs(ex1-ex2)+abs(ey1-ey2) < (w+h)/4)) {
        pick_wall_pos(ex2, ey2);
    }
    // Ensure tile next to entrance/exit is open
    int dx1 = (ex1==0)?1:(ex1==w-1)?-1:0, dy1 = (ey1==0)?1:(ey1==h-1)?-1:0;
    int dx2 = (ex2==0)?1:(ex2==w-1)?-1:0, dy2 = (ey2==0)?1:(ey2==h-1)?-1:0;
    map[ey1][ex1] = TILE_ENTRANCE;
    map[ey1+dy1][ex1+dx1] = TILE_FLOOR;
    map[ey2][ex2] = TILE_EXIT;
    map[ey2+dy2][ex2+dx2] = TILE_FLOOR;
    entrance_pos = {ex1, ey1};
    exit_pos = {ex2, ey2};

    // Ensure a path exists between entrance-adjacent and exit-adjacent tiles
    int sx = ex1+dx1, sy = ey1+dy1;
    int gx = ex2+dx2, gy = ey2+dy2;
    std::vector<std::vector<bool>> bfs_visited(h, std::vector<bool>(w, false));
    std::queue<std::pair<int,int>> q;
    q.push({sx, sy}); bfs_visited[sy][sx] = true;
    bool found = false;
    while (!q.empty()) {
        auto [x, y] = q.front(); q.pop();
        if (x == gx && y == gy) { found = true; break; }
        for (int d = 0; d < 4; ++d) {
            int nx = x+dx[d], ny = y+dy[d];
            if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
            if (!bfs_visited[ny][nx] && map[ny][nx] == '.') {
                bfs_visited[ny][nx] = true;
                q.push({nx, ny});
            }
        }
    }
    // If not found, forcibly carve a path
    if (!found) {
        // Simple straight tunnel
        int x = sx, y = sy;
        while (x != gx || y != gy) {
            if (x < gx) ++x;
            else if (x > gx) --x;
            else if (y < gy) ++y;
            else if (y > gy) --y;
            if (map[y][x] == '#') map[y][x] = '.';
        }
    }
    return map;
}
