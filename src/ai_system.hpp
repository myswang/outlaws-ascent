#pragma once
#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE


class AISystem {
public:

    struct Node {
        vec2 position = vec2(0.f, 0.f);
        float g_cost = 0.f;
        float f_cost= 0.f;
    };

    void step(float elapsed_ms);
    std::vector<vec2> getBFSPath(vec2 start, vec2 goal, std::vector<std::vector<int>> grid);

    std::vector<vec2>
    getAStarPath(vec2 start, vec2 goal, std::vector<std::vector<int>> grid, std::unordered_map<vec2, vec2> &came_from,
                 std::unordered_map<vec2, float> &cost);

    std::vector<vec2> getAStarPath(vec2 start, vec2 goal, std::vector<std::vector<int>> grid, bool until_touching);

    std::vector<vec2> getNeighboursAsVec2(vec2 curr_pos, std::vector<std::vector<int>> grid);

    std::vector<Node*> getNeighboursAsNode(Node *curr_node, std::vector<std::vector<int>> grid, bool until_touching);

    std::vector<vec2> getBFSPath(vec2 start, vec2 goal, std::vector<std::vector<int>> grid, std::vector<vec2> path);
};