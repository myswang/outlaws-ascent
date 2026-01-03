// internal
#include "physics_system.hpp"
#include "common.hpp"
#include "components.hpp"
#include "world_init.hpp"
#include <vector>
#include <iostream>

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE


// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
    // abs is to avoid negative scale due to the facing direction.
    return { abs(motion.scale.x), abs(motion.scale.y) };
}

// returns an object's 'hitbox': vector consisting of top-left and bottom-right corner
// vec2 get_hitbox_top_left(const Motion& motion) {
// 	return { motion.position.x - abs(motion.scale.x) * 0.5f, motion.position.y - abs(motion.scale.y) * 0.5f };
// }

// vec2 get_hitbox_bottom_right(const Motion& motion) {
// 	return { motion.position.x + abs(motion.scale.x) * 0.5f, motion.position.y + abs(motion.scale.y) * 0.5f };
// }

// performs basic AABB calculation
bool collides(const Motion& m1, const Motion& m2)
{
	// precompte these for efficiency
	const float s1x = abs(m1.scale.x)*0.5f;
	const float s1y = abs(m1.scale.y)*0.5f;
	const float s2x = abs(m2.scale.x)*0.5f;
	const float s2y = abs(m2.scale.y)*0.5f;

	return m1.position.x-s1x <= m2.position.x+s2x && 
		   m1.position.y-s1y <= m2.position.y+s2y &&
		   m1.position.x+s1x >= m2.position.x-s2x && 
		   m1.position.y+s1y >= m2.position.y-s2y;
}

// Linear interpolation function lerp, interpolates within the range [start, end] based on parameter t, within a [0, 1] range
// https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/a-brief-introduction-to-lerp-r4954/
float lerp(float start, float end, float t) {
	return start * (1 - t) + t * end;
}

// cubic ease-off function, can be used to produce a more "dramatic" slowdown effect
// can be passed to time parameter in the lerp function
float cubic_easeout(float t) {
	return 1 - (1 - t) * (1 - t) * (1 - t);
}


void PhysicsSystem::step(float elapsed_ms)
{
	// Move fish based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	for (uint i = 0; i < motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		if (registry.prescribedMotions.has(entity)) continue;

		float step_seconds = elapsed_ms / 1000.f;

		// bullet interpolation check
		if (registry.bulletLifeTimes.has(entity)) {
			// handles t calculation, delta time / bullet life time
			float t_delta_life = registry.bulletLifeTimes.get(entity).counter_ms / 1000.f;
			// THIS is how long a bullet can exist
			float t_bullet_life = 5.f;
			float t = t_delta_life / t_bullet_life;

			// interpolation of velocity, end velocity factor may be moved as a constant somewhere else
			motion.velocity.x = lerp(motion.max_velocity.x, motion.max_velocity.x / 500, t);
			motion.velocity.y = lerp(motion.max_velocity.y, motion.max_velocity.y / 500, t);
		}

		// dash interpolation check
		if (registry.dashCooldownTimers.has(entity)) {
			float t_delta_life_dash = DASH_TIME_MS - registry.dashCooldownTimers.get(entity).counter_ms;
			float t_dash = t_delta_life_dash / DASH_TIME_MS;

			// interpolation of velocity
			motion.velocity.x = lerp(motion.max_velocity.x, 0, t_dash);
			motion.velocity.y = lerp(motion.max_velocity.y, 0, t_dash);
		}

		// move enemies following the player
		Entity& player = registry.players.entities[0];
		vec2 player_pos = registry.motions.get(player).position;
		auto& following_player = registry.following;
		for (uint i = 0; i < following_player.size(); i++) {
			// make the enemy following the player face the player
			Entity& entity = following_player.entities[i];
			Enemy& enemy = registry.enemies.get(entity);
			Motion& enemy_motion = registry.motions.get(entity);

			enemy.angle = atan2((player_pos.y - enemy_motion.position.y), ((player_pos.x - enemy_motion.position.x)));

			// if path was not computed in the number of steps, don't move at all
			// if we have performance issues with multiple enemies, have enemies just move towards player's direction

			if (enemy.path.empty()) {
				vec2 dir_vec = vec2(0, 0);
                if (enemy.enemyType == EnemyType::KNIFE){
                    float x = cosf(enemy.angle);
                    float y = sinf(enemy.angle);
                    dir_vec = vec2(x,y);
                }

				if (dir_vec != vec2(0, 0)) {
					dir_vec = normalize(dir_vec);
				}
				enemy_motion.velocity = dir_vec * SPEED_MAP[enemy.enemyType];
                if (enemy.worldEffect == Effects::MUD){
                    enemy_motion.velocity *= 0.5;
                }
			}
			else {
				vec2 curr_pos = enemy.path.front();
				vec2 enemy_cell = vec2(0, 0);

				// prevent enemy from cutting corners
				if (enemy_motion.velocity.y > 0) {
					enemy_cell.y = std::floor((enemy_motion.position.y / (float)SPRITE_SIZE_STANDARD));
				}
				else {
					enemy_cell.y = std::ceil((enemy_motion.position.y / (float)SPRITE_SIZE_STANDARD));
				}

				if (enemy_motion.velocity.x > 0) {
					enemy_cell.x = std::floor((enemy_motion.position.x / (float)SPRITE_SIZE_STANDARD));
				}
				else {
					enemy_cell.x = std::ceil((enemy_motion.position.x / (float)SPRITE_SIZE_STANDARD));
				}

				// if enemy cell is the same as current, move onto the next action
				if (enemy_cell == curr_pos) {
					enemy.path.erase(enemy.path.begin());
					if (!enemy.path.empty()) {
						curr_pos = enemy.path.front();
					}
				}

				vec2 dir_vec = curr_pos - enemy_cell;
				enemy_motion.velocity = dir_vec * SPEED_MAP[enemy.enemyType];
                if (enemy.worldEffect == Effects::MUD){
                    enemy_motion.velocity *= 0.5;
                }
			}
		}

        auto& fleeing_player = registry.fleeing;
        for (uint i = 0; i < fleeing_player.size(); i++) {
            // make the enemy following the player face the player
            Entity& entity = fleeing_player.entities[i];
            Enemy& enemy = registry.enemies.get(entity);
            Motion& enemy_motion = registry.motions.get(entity);

            enemy.angle = atan2((player_pos.y - enemy_motion.position.y), ((player_pos.x - enemy_motion.position.x)));
            float x = cosf(enemy.angle);
            float y = sinf(enemy.angle);
            vec2 dir_vec = vec2(-x,-y);

            if (dir_vec != vec2(0, 0)) {
                dir_vec = normalize(dir_vec);
            }
			
			enemy_motion.velocity = dir_vec * SPEED_MAP[enemy.enemyType];

            if (enemy.worldEffect == Effects::MUD){
                enemy_motion.velocity *= 0.5;
            }
			
        }

        auto& attacking_player = registry.attacking;
        for (uint i = 0; i < attacking_player.size(); i++) {
            // make the enemy following the player face the player
            Entity& entity = attacking_player.entities[i];
            Enemy& enemy = registry.enemies.get(entity);
            Motion& enemy_motion = registry.motions.get(entity);

            enemy.angle = atan2((player_pos.y - enemy_motion.position.y), ((player_pos.x - enemy_motion.position.x)));

            if (enemy.enemyType == EnemyType::KNIFE){
                float x = cosf(enemy.angle);
                float y = sinf(enemy.angle);
                vec2 dir_vec = vec2(x,y);
                if (dir_vec != vec2(0, 0)) {
                    dir_vec = normalize(dir_vec);
                }
                enemy_motion.velocity = dir_vec * SPEED_MAP[enemy.enemyType];
                if (enemy.worldEffect == Effects::MUD){
                    enemy_motion.velocity *= 0.5;
                }
            }


        }

		motion.position += motion.velocity * step_seconds;

        // remove mud effect
        if (registry.enemies.has(entity)){
            Enemy &enemy = registry.enemies.get(entity);
            if (enemy.worldEffect == Effects::MUD){
                enemy.worldEffect = Effects::NONE;
            }
        }

		// move enemy healthbars with enemies
		if (registry.hasHealthBars.has(entity)) {
			HasHealthBar hhb = registry.hasHealthBars.get(entity);
			Entity hb = hhb.healthBar;
			if (registry.motions.has(hb)) {
				Motion& hb_motion = registry.motions.get(hb);
				hb_motion.position = motion.position + ENEMY_HEALTHBAR_OFFSET;
			}
		}

	}

    

	// Check for collisions between all moving entities
	ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		if (!motion_i.can_collide)
			continue;
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j<motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			if (!motion_j.can_collide)
				continue;
			Entity entity_j = motion_container.entities[j];

			// exclude weapons from the collision system if they are held by a player/enemy AND equipped weapons, also exclude all UI elements
			// TODO: find a cleaner way to do this. add UIElement component? 
			
			if (collides(motion_i, motion_j))
			{
                if (registry.meshPtrs.has(entity_i)){
                    // precise collision
                    if (!is_mesh_collision(entity_i, entity_j)){
                        continue;
                    }
                }
                if (registry.meshPtrs.has(entity_j)){
                    // precise collision
                    if (!is_mesh_collision(entity_j, entity_i)){
                        continue;
                    }
                }
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
            }
		}
	}

    if (debugging.in_debug_mode) {

        // draw bounding box for player (ie. highlight vertices)
        Entity &player = registry.players.entities[0];
        Motion &player_motion = registry.motions.get(player);

        vec4 collision_color = vec4(1.f, 0.0, 0.0, 1.0);
        vec4 default_color = vec4(0.2, 0.0, 0.0, 1.0);

        vec2 bounding_box = get_bounding_box(player_motion);
        vec4 color = registry.collisionTimers.has(player) ? collision_color : default_color;

        PhysicsSystem::draw_bounding_box(player, color, bounding_box);


        // draw bounding box for obstacles
        ComponentContainer<WorldObject> objs = registry.worldObjects;
        for (uint i = 0; i < objs.components.size(); i++) {
            Entity &obj = registry.worldObjects.entities[i];
            Motion &motion = registry.motions.get(obj);
            vec2 bounding_box = get_bounding_box(motion);

            vec4 color = registry.collisionTimers.has(obj) ? collision_color : default_color;

            PhysicsSystem::draw_bounding_box(obj, color, bounding_box);
        }

        ComponentContainer<WorldEffect> effects = registry.worldEffects;
        for (uint i = 0; i < effects.components.size(); i++) {
            Entity &obj = registry.worldEffects.entities[i];
            Motion &motion = registry.motions.get(obj);
            vec2 bounding_box = get_bounding_box(motion);

            vec4 color = registry.collisionTimers.has(obj) ? collision_color : default_color;

            PhysicsSystem::draw_bounding_box(obj, color, bounding_box);
        }

        ComponentContainer<Enemy> enemies = registry.enemies;
        for (uint i = 0; i < enemies.components.size(); i++) {
            Entity &enemy = registry.enemies.entities[i];
            Motion &motion = registry.motions.get(enemy);
            vec2 bounding_box = get_bounding_box(motion);
            vec4 color = registry.collisionTimers.has(enemy) ? collision_color : default_color;

            PhysicsSystem::draw_bounding_box(enemy, default_color, bounding_box);
        }
    }
}

bool PhysicsSystem::is_mesh_collision(Entity &mesh_obj, Entity &other_obj){
    Motion &mesh_motion = registry.motions.get(mesh_obj);
    Mesh* mesh = registry.meshPtrs.get(mesh_obj);
    std::vector<ColoredVertex> vertices = mesh->vertices;
    std::vector<uint16_t> indices = mesh->vertex_indices;

    Transform trans;
    trans.translate(mesh_motion.position);
    trans.rotate(mesh_motion.angle);
    trans.scale(mesh_motion.scale);

    Motion &other_motion = registry.motions.get(other_obj);
    vec2 other_bb = get_bounding_box(other_motion);

    // get bounding box of other object
    float left = other_motion.position.x - abs(other_bb.x / 2);
    float right = other_motion.position.x + abs(other_bb.x / 2);
    float top = other_motion.position.y - abs(other_bb.y / 2);
    float bot = other_motion.position.y + abs(other_bb.y / 2);

    for (ColoredVertex& vertex: vertices) {
        vec2 vertex_position = trans.mat * vec3(vertex.position.x, vertex.position.y, 1);

        // if vertex is within bounding box, then return true
        if (vertex_position.x >= left && vertex_position.x <= right && vertex_position.y > top && vertex_position.y < bot){
            return true;
        }
    }

    return false;

}

void PhysicsSystem::draw_bounding_box(Entity& entity, vec4 color, vec2 bounding_box){
    int line_thickness = 3;

    Motion &motion = registry.motions.get(entity);
    vec2 scale = motion.scale;
    vec2 pos = motion.position;

    // draw bounding box around the entity
    // top and bottom boundaries
    Entity top = createLine(motion.position - vec2({0, bounding_box.y / 2}),
                            vec2({bounding_box.x, line_thickness}));
    Entity bot = createLine(motion.position + vec2({0, bounding_box.y / 2}),
                            vec2({bounding_box.x, line_thickness}));
    // left and right boundaries
    Entity left = createLine(motion.position - vec2({bounding_box.x / 2, 0}),
                             vec2({line_thickness, bounding_box.y}));
    Entity right = createLine(motion.position + vec2({bounding_box.x / 2, 0}),
                              vec2({line_thickness, bounding_box.y}));

    registry.colors.insert(top, color);
    registry.colors.insert(bot, color);
    registry.colors.insert(left, color);
    registry.colors.insert(right, color);

    if (registry.meshPtrs.has(entity)){
        Transform trans;
        trans.translate(motion.position);
        trans.rotate(motion.angle);
        trans.scale(motion.scale);

        auto &vertices = registry.meshPtrs.get(entity)->vertices;
        for (ColoredVertex &vertex: vertices) {
            vec2 vertex_position = trans.mat * vec3(vertex.position.x, vertex.position.y, 1);
            Entity line = createLine({vertex_position.x, vertex_position.y}, {4, 4});
            registry.colors.insert(line, color);
        }
    }

}

// Citation: D. Rose
// https://danceswithcode.net/engineeringnotes/rotations_in_2d/rotations_in_2d.html
vec2 PhysicsSystem::rotate_pivot(vec2 point, vec2 pivot, float angle)
{
	float x = (point.x - pivot.x) * cos(angle) - (point.y - pivot.y) * sin(angle) + pivot.x;
	float y = (point.x - pivot.x) * sin(angle) - (point.y - pivot.y) * cos(angle) + pivot.y;
	return vec2(x, y);
}

// apply transformations to make limb follow the weapon
void PhysicsSystem::transform_limb(Motion& limb, vec2 start, vec2 end)
{
	vec2 midpoint = {(start.x+end.x)/2.0f, (start.y+end.y)/2.0f};
	float sx = sqrt(pow(end.x-start.x,2) + pow(end.y-start.y, 2));
	limb.angle = atan2(end.y-start.y, end.x-start.x);
	float adjust_x = -10 * cos(limb.angle);
	float adjust_y = -10 * sin(limb.angle);
	limb.position = {midpoint.x + adjust_x, midpoint.y + adjust_y};
	limb.scale = {sx, SPRITE_SIZE_STANDARD * 0.5f};
}

// move entities that don't depend on velocity, but rather modify position directly
// TODO: put more entities that move this way here, so they aren't affected by the collision system
void PhysicsSystem::move_fixed_entities(float elapsed_ms) {
	for (Entity& entity : registry.weaponHolders.entities) {
		Entity& weapon = registry.weaponHolders.get(entity).weapon;
		Motion& motion = registry.motions.get(entity);
		Motion& weapon_motion = registry.motions.get(weapon);
		vec2 offset = vec2(WEAPON_OFFSET, 0);
		weapon_motion.position = rotate_pivot(motion.position+offset, motion.position, weapon_motion.angle);
		if (motion.scale.x < 0) {
			weapon_motion.scale.x = abs(weapon_motion.scale.x);
			weapon_motion.scale.y = -abs(weapon_motion.scale.y);

		} else {
			weapon_motion.scale.x = abs(weapon_motion.scale.x);
			weapon_motion.scale.y = abs(weapon_motion.scale.y);
		}
		// transform limbs for player/enemy
		if (registry.skeletons.has(entity)) {
			// left arm
			Entity& limb = registry.skeletons.get(entity).left_arm;
			Motion& limb_motion = registry.motions.get(limb);
			vec2 start = {motion.position.x - LEFT_ARM_OFFSET, motion.position.y - 3.0f};
			vec2 end = weapon_motion.position;
			transform_limb(limb_motion, start, end);

			// right arm
			Entity& limb2 = registry.skeletons.get(entity).right_arm;
			Motion& limb2_motion = registry.motions.get(limb2);
			vec2 start2 = {motion.position.x + RIGHT_ARM_OFFSET, motion.position.y - 3.0f};
			transform_limb(limb2_motion, start2, end);
		}
	}

	// move particles via lerp function + cubic ease out function
	for (Entity& entity : registry.particles.entities) {
		Particle& part_data = registry.particles.get(entity);
		if (!registry.motions.has(part_data.parent)) {
			registry.remove_all_components_of(entity);
			continue;
		}
		Motion& parent_motion = registry.motions.get(part_data.parent);
		Motion& part_motion = registry.motions.get(entity);
		float t = part_data.counter_ms/part_data.max_ms;
		float adjusted_t = cubic_easeout(t);
		part_motion.position.x = lerp(parent_motion.position.x, parent_motion.position.x+part_data.dist.x, adjusted_t);
		part_motion.position.y = lerp(parent_motion.position.y, parent_motion.position.y+part_data.dist.y, adjusted_t);
		// update particle alpha channel with lerp
		vec4& color = registry.colors.get(entity);
		color.w = lerp(1.0f, 0.0f, t);
	}

	// move firebombs according to bezier curve equation
	for (Entity& entity : registry.prescribedMotions.entities) {
		Motion& motion = registry.motions.get(entity);
		// get bezier curve parameter data
		PrescribedMotion& pm = registry.prescribedMotions.get(entity);
		// normalize time
		float t = registry.bulletLifeTimes.get(entity).counter_ms;
		float t_end = pm.time_ms / 1000.0f;
		t /= 1000.0f;
		t /= t_end;
		// bezier curve equations for x and y position components
		motion.position.x = (1 - t)*(1 - t) * pm.start.x + 2 * (1 - t) * t * pm.apex.x + t * t * pm.end.x;
		motion.position.y = (1 - t)*(1 - t) * pm.start.y + 2 * (1 - t) * t * pm.apex.y + t * t * pm.end.y;
	}
}
