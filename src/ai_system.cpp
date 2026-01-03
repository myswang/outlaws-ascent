#pragma once
#ifndef _AI_SYS
#define _AI_SYS

// internal
#include "ai_system.hpp"
#include "world_init.hpp"
#include <iostream>
#include <queue>
#include <algorithm>
#include <map>


// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE

vec2 player_pos;
float path_computation_timer = 0;

const int FOLLOWING_RADIUS = 400;
const int STOP_FOLLOWING_RADIUS = 600;
const int MAX_CALCULATION_STEPS = 1000;
int x_dimensions = WORLD_MAX_X - WORLD_MIN_X;
int y_dimensions = WORLD_MAX_Y - WORLD_MIN_Y;

// detect if enemy is in range of the player
// if enemy is in range, make enemy pursue the player
void in_range(Entity &enemy, Entity &other){
    Enemy &en = registry.enemies.get(enemy);
    Motion &enemy_motion = registry.motions.get(enemy);
    Motion &other_motion = registry.motions.get(other);
    float x = enemy_motion.position.x - other_motion.position.x;
    float y = enemy_motion.position.y - other_motion.position.y;
    float distance = sqrt(pow(x,2) + pow(y,2));

    if (en.enemyType == EnemyType::DEFAULT || en.enemyType == EnemyType::KNIFE || en.enemyType == EnemyType::BLINDER){
        if (distance < FOLLOWING_RADIUS){
            if (!registry.following.has(enemy)){
                registry.following.emplace(enemy);
            }
        } else if (distance > STOP_FOLLOWING_RADIUS){
            if (registry.following.has(enemy)){
                registry.following.remove(enemy);
                enemy_motion.velocity = vec2(0,0);
            }
        }
    } else if (en.enemyType == EnemyType::SNIPER || en.enemyType == EnemyType::FIRE){
        if (distance < STOP_FOLLOWING_RADIUS-100 && distance > FOLLOWING_RADIUS){
            if (!registry.following.has(enemy)){
                registry.following.emplace(enemy);
            }
            if (registry.fleeing.has(enemy)){
                registry.fleeing.remove(enemy);
            }
        } else if (distance > STOP_FOLLOWING_RADIUS-100){
            if (registry.following.has(enemy)){
                registry.following.remove(enemy);
                enemy_motion.velocity = vec2(0,0);
            }
        } else if (distance < FOLLOWING_RADIUS+100){
            if (!registry.fleeing.has(enemy)){
                registry.fleeing.emplace(enemy);
            }
        }
    }

}

//void in_range_of_healer(Entity &enemy, Entity &healer){
//    Enemy &en = registry.enemies.get(enemy);
//    Motion &enemy_motion = registry.motions.get(enemy);
//    Motion &healer_motion = registry.motions.get(healer);
//    float x = enemy_motion.position.x - healer_motion.position.x;
//    float y = enemy_motion.position.y - healer_motion.position.y;
//    float distance = sqrt(pow(x,2) + pow(y,2));
//    Healer &hl = registry.healers.get(healer);
//
//    if (distance < FOLLOWING_RADIUS){
//        hl.enemy = enemy;
//    } else if (distance > STOP_FOLLOWING_RADIUS){
//    }
//
//}

// returns the top, bottom, left, and right in that order of the square around the player's current position
vec4 getSquareAroundPlayer(bool until_touching) {
    vec4 toReturn = vec4(0.0, 0.0, 0.0, 0.0);
    if (registry.players.entities.size() > 0) {
        Entity player = registry.players.entities[0];
        if (registry.motions.has(player)) {
            Motion player_motion = registry.motions.get(player);
            if (until_touching){
                toReturn =  vec4(floor((player_motion.position.y) / SPRITE_SIZE_STANDARD), ceil((player_motion.position.y) / SPRITE_SIZE_STANDARD), floor((player_motion.position.x) / SPRITE_SIZE_STANDARD), ceil((player_motion.position.x) / SPRITE_SIZE_STANDARD));
            } else {
                toReturn =  vec4(floor((player_motion.position.y - GRID_AROUND_PLAYER_SIZE / 2.0f) / SPRITE_SIZE_STANDARD), ceil((player_motion.position.y + GRID_AROUND_PLAYER_SIZE / 2.0f) / SPRITE_SIZE_STANDARD), floor((player_motion.position.x - GRID_AROUND_PLAYER_SIZE / 2.0f) / SPRITE_SIZE_STANDARD), ceil((player_motion.position.x + GRID_AROUND_PLAYER_SIZE / 2.0f) / SPRITE_SIZE_STANDARD));
            }
        }
    }
    return toReturn;
}


void AISystem::step(float elapsed_ms)
{

    (void)elapsed_ms; // placeholder to silence unused warning until implemented

    for (int i = 0; i < registry.enemies.size(); i++){
        Entity &entity = registry.enemies.entities[i];
        Enemy &enemy = registry.enemies.components[i];
        in_range(entity, registry.players.entities[0]);
    }

    // make healer follow enemies that are hurt
//    for (int i = 0; i < registry.healers.size(); i++){
//        Entity &healer = registry.healers.entities[i];
//
//        for (int i = 0; i < registry.enemies.size(); i++){
//            Entity &entity = registry.enemies.entities[i];
//            Enemy &enemy = registry.enemies.components[i];
//            if (enemy.enemyType != EnemyType::HEALER){
//                in_range(entity, registry.players.entities[0]);
//            }
//        }
//    }

    path_computation_timer -= elapsed_ms;
    if (path_computation_timer <= 0){
        path_computation_timer = 500;

        // construct a grid depending on world dimensions, technically this only ever needs to be done once?
        std::vector<std::vector<int>> grid = {};

        std::vector<vec2> obstacle_locations;
        for (int i = 0; i <= x_dimensions; i++) {
            std::vector<int> col;
            for (int j = 0; j <= y_dimensions; j++) {
                col.push_back(0);
            }
            grid.push_back(col);
        }

        // indicate locations of obstacles on the grid
        for (int i = 0; i < registry.worldObjects.size(); i++){
            Entity &entity = registry.worldObjects.entities[i];
            Motion &motion = registry.motions.get(entity);

            vec2 pos = motion.position;

            // to try and fix mesh based cactus pathfinding, make these absolute values
            // but this causes performance issues?
            float width = (motion.scale.x);
            float height = (motion.scale.y);

            float cell_leftmost = (pos.x - width / 2.0) / (float)SPRITE_SIZE_STANDARD;
            float cell_rightmost = (pos.x + width / 2.0) / (float)SPRITE_SIZE_STANDARD;
            float cell_upmost = (pos.y - height / 2.0) / (float)SPRITE_SIZE_STANDARD;
            float cell_downmost = (pos.y + height / 2.0) / (float)SPRITE_SIZE_STANDARD;

            vec2 location = motion.position / (float) SPRITE_SIZE_STANDARD;

            // if there is an obstacle in that location, then put a 1
            // get all locations that are spanned by an obstacle (for now, an obstacle is at most sprite standard size)
            std::vector<int> possible_x_vals;
            std::vector<int> possible_y_vals;

            for (int i = std::floor(cell_leftmost); i <= std::ceil(cell_rightmost); i++) {
                possible_x_vals.push_back(i - WORLD_MIN_X);
            }

            for (int i = std::floor(cell_upmost); i <= std::ceil(cell_downmost); i++) {
                possible_y_vals.push_back(i - WORLD_MIN_Y);
            }

            /*if (location.x != round(location.x - WORLD_MIN_X)){
                possible_x_vals.push_back(std::ceil(location.x) - WORLD_MIN_X);
                possible_x_vals.push_back(std::floor(location.x) - WORLD_MIN_X);
            }

            if (location.y != round(location.y - WORLD_MIN_Y)){
                possible_y_vals.push_back(std::ceil(location.y) - WORLD_MIN_Y);
                possible_y_vals.push_back(std::floor(location.y) - WORLD_MIN_Y);
            }*/

            for (int x_loc: possible_x_vals){
                for (int y_loc: possible_y_vals){
                    if (x_loc >= 0 && x_loc < x_dimensions && y_loc >= 0 && y_loc < y_dimensions ) {
                        grid[x_loc][y_loc] = 1;
                    }
                }
            }

        }

        Entity &player = registry.players.entities[0];
        player_pos = registry.motions.get(player).position;
        vec2 player_grid_location = player_pos/(float) SPRITE_SIZE_STANDARD;

            // calculate path for each entity following the player
        for (int i = 0; i < registry.following.size(); i++) {
            // get path for each entity
            Entity& following_entity = registry.following.entities[i];
            Enemy& enemy = registry.enemies.get(following_entity);
            Motion& motion = registry.motions.get(following_entity);
            vec2 enemy_grid_location = motion.position / (float)SPRITE_SIZE_STANDARD;
            // a* calculation or bfs calculation
//            std::vector<vec2> path = getBFSPath(round(enemy_grid_location), round(player_grid_location), grid);
            std::vector<vec2> path;
            if (enemy.enemyType == EnemyType::KNIFE){
                path = getAStarPath(round(enemy_grid_location), round(player_grid_location), grid, true);
            } else {
                path = getAStarPath(round(enemy_grid_location), round(player_grid_location), grid, false);
            }
            enemy.path = path;
        }
        
    }
}

std::vector<vec2> AISystem::getNeighboursAsVec2(vec2 curr_pos, std::vector<std::vector<int>> grid){
    vec2 adjacent_movements[8] = {vec2(0,1), vec2(0,-1),
                                  vec2(1,1), vec2(1,-1),
                                  vec2(1,0), vec2(-1,0),
                                  vec2(-1,1), vec2(-1,-1)};

    std::vector<vec2> neighbours;

    for (vec2 movement_dir : adjacent_movements){
        vec2 possible_neighbour = curr_pos + movement_dir;
        if ((possible_neighbour.x < WORLD_MAX_X) && (possible_neighbour.x > WORLD_MIN_X)
            && (possible_neighbour.y < WORLD_MAX_Y) && (possible_neighbour.y > WORLD_MIN_Y)){
            // only make it a possible neighbour if there are no obstacles in the way (ie. grid value is 0)
            int x_loc = (int) round(possible_neighbour.x - WORLD_MIN_X);
            int y_loc = (int) round(possible_neighbour.y - WORLD_MIN_Y);
            if (grid[x_loc][y_loc] == 0){
                neighbours.push_back(possible_neighbour);
            }
        }

    }
    return neighbours;
}

std::vector<AISystem::Node*> AISystem::getNeighboursAsNode(Node* curr_node, std::vector<std::vector<int>> grid, bool until_touching){
//    vec2 adjacent_movements[8] = {vec2(0,1), vec2(0,-1),
//                                  vec2(1,1), vec2(1,-1),
//                                  vec2(1,0), vec2(-1,0),
//                                  vec2(-1,1), vec2(-1,-1)};

    vec2 adjacent_movements[4] = {vec2(0,1), vec2(0,-1),
                                  vec2(1,0), vec2(-1,0)};

    std::vector<Node *> neighbours;

    vec2 curr_pos = curr_node->position;
    for (vec2 movement_dir : adjacent_movements){
        vec2 possible_neighbour = curr_pos + movement_dir;
        if ((possible_neighbour.x < WORLD_MAX_X) && (possible_neighbour.x > WORLD_MIN_X)
            && (possible_neighbour.y < WORLD_MAX_Y) && (possible_neighbour.y > WORLD_MIN_Y)){
            // only make it a possible neighbour if there are no obstacles in the way (ie. grid value is 0)
            int x_loc = (int) floor(possible_neighbour.x - WORLD_MIN_X);
            int y_loc = (int) floor(possible_neighbour.y - WORLD_MIN_Y);
            if (grid[x_loc][y_loc] == 0){
                // t, b, l, r
                vec4 playerSquare = getSquareAroundPlayer(until_touching);

                // only consider nodes that are within a certain square surrounding player's position
                if (possible_neighbour.x >= playerSquare[2] && possible_neighbour.x <= playerSquare[3] && possible_neighbour.y >= playerSquare[0] && possible_neighbour.y <= playerSquare[1]) {
                    Node* node = new Node;
                    node->position = possible_neighbour;
                    neighbours.push_back(node);
                    
                }
            }
        }

    }
    return neighbours;
}

// manhattan distance
float manhattan_distance_heuristic(vec2 location_1, vec2 location_2){
    return abs(location_1.x - location_2.x) + abs(location_1.y + location_2.y);
}

// https://www.redblobgames.com/pathfinding/a-star/implementation.html
std::vector<vec2> AISystem::getBFSPath(vec2 start, vec2 goal, std::vector<std::vector<int>> grid){

    std::vector<vec2> initial_path;
    initial_path.push_back(start);

    std::vector<std::vector<vec2>> frontier;
    frontier.push_back(initial_path);

    int max_steps = MAX_CALCULATION_STEPS;

    while (!frontier.empty() && max_steps >= 0){
        max_steps--;
        std::vector<vec2> path = frontier.front();
        frontier.erase(frontier.begin());

        vec2 curr_pos = path.back();
        if (curr_pos == goal){
            return path;
        }

        // get neighbours of curr
        std::vector<vec2> neighbours = getNeighboursAsVec2(curr_pos, grid);
        for (vec2 neighbour : neighbours){
            std::vector<vec2> forked_path = path;
            forked_path.push_back(neighbour);
            frontier.push_back(forked_path);
        }

    }

    // if no path found within max calculation steps, maybe have some default behaviour in physics_system.cpp
    // just making them move in general direction of player
    return {};
}

class NodeComparator {

public: 
    int operator() (std::pair<AISystem::Node*, std::vector<AISystem::Node*>> n1, std::pair<AISystem::Node*, std::vector<AISystem::Node*>> n2) {
        return n1.first->f_cost > n2.first->f_cost;
    }
};

// inspired by
// https://medium.com/@nicholas.w.swift/easy-a-star-pathfinding-7e6689c7f7b2
std::vector<vec2> AISystem::getAStarPath(vec2 start, vec2 goal, std::vector<std::vector<int>> grid, bool until_touching){

    std::priority_queue<std::pair<Node*, std::vector<Node*>>, std::vector<std::pair<Node*, std::vector<Node*>>>, NodeComparator> open_pq;
    std::map<std::vector<float>, float> g_cost_map;
    std::vector<Node*> closed;
    std::vector<vec2> path;


    Node* start_node = new Node;
    start_node->position = start;
    start_node->f_cost = 0;
    start_node->g_cost = 0;
    g_cost_map[{start_node->position.x, start_node->position.y}] = 0;
    open_pq.push({start_node, std::vector<Node*>()});


    int max_steps = MAX_CALCULATION_STEPS;
    while (!open_pq.empty() && max_steps >= 0){
        max_steps--;
        std::pair<Node*, std::vector<Node*>> curr_node_pair = open_pq.top();
        Node* curr_node = curr_node_pair.first;
        open_pq.pop();
        closed.push_back(curr_node_pair.first);

        if (curr_node->position == goal){
            for (Node* node: curr_node_pair.second) {
                vec2 pos = node->position;
                path.push_back(pos);
            }
            return path;
        }

        std::vector<Node*> children = getNeighboursAsNode(curr_node, grid, until_touching);
        for (Node* child : children){
            std::vector<Node*> thisPath = curr_node_pair.second;
            for(Node* closed_child : closed){
                if (child == closed_child){
                    goto next_child;
                }
            }

            child->g_cost = curr_node->g_cost + 1;
            child->f_cost = child->g_cost + manhattan_distance_heuristic(child->position, goal);


            if (g_cost_map.find({child->position.x, child->position.y}) != g_cost_map.end() && child->g_cost > g_cost_map[{child->position.x, child->position.y}]) {
                goto next_child;
            }

            g_cost_map[{child->position.x, child->position.y}] = child->g_cost;
            thisPath.push_back(curr_node);
            open_pq.push({ child, thisPath });
            next_child:;

        }
    }

    // if no path found within max calculation steps, maybe have some default behaviour in physics_system.cpp
    // just making them move in general direction of player
    return {};
}

#endif