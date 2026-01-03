#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE


// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	void step(float elapsed_ms);
	vec2 rotate_pivot(vec2 point, vec2 pivot, float angle);
	void transform_limb(Motion& motion, vec2 start, vec2 end);
	void move_fixed_entities(float elapsed_ms);

	PhysicsSystem()
	{
	}

	void draw_bounding_box(Entity &entity, vec4 color, vec2 bounding_box);

    bool is_mesh_collision(Entity &mesh_obj, Entity &other_obj);
};