#pragma once
#include <vector>
#include <string>
#include <utility>

// Generates a random floor with a guaranteed path from entrance to exit.
// entrance_pos and exit_pos will be set to the generated positions.
// Returns a vector of strings representing the map.
std::vector<std::string> generate_random_floor(int w, int h, std::pair<int,int>& entrance_pos, std::pair<int,int>& exit_pos, unsigned int seed = 0);
