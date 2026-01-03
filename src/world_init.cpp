#include "world_init.hpp"
#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include <iostream>
#include <string>
// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE and modified slightly

std::default_random_engine rng = std::default_random_engine(std::random_device()());
std::uniform_int_distribution<> which_sprite_dist = std::uniform_int_distribution<>(1, 3);

int enemy_count = 0;

Entity createMeshBasedCactus(RenderSystem* renderer, vec2 pos)
{
    auto entity = Entity();

    // Store a reference to the potentially re-used mesh object
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::CACTUS_MESH);
    registry.meshPtrs.emplace(entity, &mesh);

    // Setting initial motion values
    Motion& motion = registry.motions.emplace(entity);
    motion.position = pos;
    motion.velocity = { 0.f, 0.f };
    motion.scale = (float) 0.75 * mesh.original_size * (float) SPRITE_SIZE_STANDARD;
    motion.scale.y *= -1; // point front to the right
	motion.can_collide = true;

    registry.worldObjects.emplace(entity);
    registry.obstacles.emplace(entity);
    registry.damaging.emplace(entity);

    HasKnockback& kb = registry.knockbackers.emplace(entity);
    kb.is_player_affecting = true;
    kb.is_enemy_affecting = true;

    AnimationState& as = registry.animationStates.emplace(entity);

    registry.renderRequests.insert(
            entity,
            { TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
              EFFECT_ASSET_ID::COLOURED,
              GEOMETRY_BUFFER_ID::CACTUS_MESH });

    RenderGroup& rg = registry.renderGroups.emplace(entity);
    rg.entities.push_back(entity);
    rg.indices.push_back(0);

    return entity;
}

Entity createWorldEffect(vec2 position, vec2 size_scaling, Effects world_effect) {
    Entity entity = Entity();
    std::cout << entity << std::endl;

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.angle = 0.0f;
    motion.velocity = { 0, 0 };
    motion.scale = { size_scaling.x*SPRITE_SIZE_STANDARD, size_scaling.y*SPRITE_SIZE_STANDARD };
    motion.can_collide = true;

    WorldEffect &we = registry.worldEffects.emplace(entity);
    we.worldEffect = world_effect;

    if (world_effect == Effects::MUD){
        registry.renderRequests.insert(
                entity,
                { TEXTURE_ASSET_ID::MUD, // TEXTURE_COUNT indicates that no texture is needed
                  EFFECT_ASSET_ID::TEXTURED,
                  GEOMETRY_BUFFER_ID::SPRITE });
    } else if (world_effect == Effects::OASIS){
        registry.renderRequests.insert(
                entity,
                { TEXTURE_ASSET_ID::OASIS, // TEXTURE_COUNT indicates that no texture is needed
                  EFFECT_ASSET_ID::TEXTURED,
                  GEOMETRY_BUFFER_ID::SPRITE });
    }


    RenderGroup& rg = registry.renderGroups.emplace(entity);
    rg.entities.push_back(entity);
    rg.indices.push_back(0);

    return entity;

}

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();
    // Store a reference to the potentially re-used mesh object
//    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//    registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = { SPRITE_SIZE_STANDARD, SPRITE_SIZE_STANDARD };
	motion.can_collide = true;

	vec4& c = registry.colors.emplace(entity);
	c = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// create an empty Player component for our character
	registry.players.emplace(entity);
	registry.movables.emplace(entity);

    // define the keyframe data
    AnimationState& as = registry.animationStates.emplace(entity);
    as.num_ms = 50.0f;
    as.counter_ms = as.num_ms;
    as.num_rows = 8;
    as.num_cols = 4;
    as.col = 0;
    as.row = 0;

	Health& health = registry.healths.emplace(entity);
	Stamina& stamina = registry.staminas.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	// create limbs for the player
	auto limb = Entity();
	auto limb2 = Entity();
	registry.motions.emplace(limb);
	registry.motions.emplace(limb2);
	registry.limbs.emplace(limb);
	registry.limbs.emplace(limb2);
	Skeleton& skel = registry.skeletons.emplace(entity);
	skel.left_arm = limb;
	skel.right_arm = limb2;

	registry.renderRequests.insert(
		limb,
		{
			TEXTURE_ASSET_ID::LIMB,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	registry.renderRequests.insert(
		limb2,
		{
			TEXTURE_ASSET_ID::LIMB,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	// give the player a gun
	auto weapon = createWeapon(renderer, {0, 0}, false, TEXTURE_ASSET_ID::REVOLVER, WEAPON_TYPE::REVOLVER);
	HoldingWeapon& hw = registry.weaponHolders.emplace(entity);
	hw.weapon = weapon;
	Weapon& weapon_data = registry.weapons.get(weapon);
	weapon_data.being_held = true;

	registry.renderRequests.insert(
		weapon,
		{
			TEXTURE_ASSET_ID::REVOLVER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	// make the weapon equipped by the player
	registry.equippedWeapons.emplace(weapon);

	// give the player a map of the passive items they do/don't have. Start out with no passive items
	registry.hasPassiveItems.emplace(entity);

	// define a RenderGroup used for determing the order for which sprites should be rendered in
	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities = {entity, limb, limb2, weapon};
	rg.indices = {0, 1, 2, 3};

	// createParticleBurst(entity);

	return entity;
}

Entity createInstructionImage(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID tex) {
	auto entity = Entity();

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = { SPRITE_SIZE_STANDARD * 2, SPRITE_SIZE_STANDARD * 2 };
	motion.can_collide = false;

	registry.renderRequests.insert(
		entity,
		{
			tex,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);


	registry.backLayers.emplace(entity);

	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities.push_back(entity);
	rg.indices.push_back(0);

	return entity;
	
}

Entity createBloodSplotch(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = { SPRITE_SIZE_STANDARD * 0.5, SPRITE_SIZE_STANDARD * 0.5};
	motion.can_collide = false;

	LifeTimer& lt = registry.lifeTimers.emplace(entity);
	lt.counter_ms = 3000.0f;

	int which_sprite = which_sprite_dist(rng);

	if (which_sprite == 1) {
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::BLOOD_1,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);
	}
	else if (which_sprite == 2) {
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::BLOOD_2,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);
	}
	else if (which_sprite == 3) {
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::BLOOD_3,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);
	}
	else {
		// just in case, default to blood 1...
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::BLOOD_1,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);
	}

	registry.backLayers.emplace(entity);

	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities.push_back(entity);
	rg.indices.push_back(0);

	return entity;
}

Entity createPuddle(RenderSystem* renderer, vec2 pos) {
    auto entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = pos;
    motion.angle = 0.f;
    motion.velocity = { 0.f, 0.f };
    motion.scale = { SPRITE_SIZE_STANDARD * 0.5, SPRITE_SIZE_STANDARD * 0.5};
    motion.can_collide = false;

    LifeTimer& lt = registry.lifeTimers.emplace(entity);
    lt.counter_ms = 3000.0f;

    int which_sprite = which_sprite_dist(rng);

    if (which_sprite == 1) {
        registry.renderRequests.insert(
                entity,
                {
                        TEXTURE_ASSET_ID::PUDDLE_1,
                        EFFECT_ASSET_ID::TEXTURED,
                        GEOMETRY_BUFFER_ID::SPRITE
                }
        );
    }
    else if (which_sprite == 2) {
        registry.renderRequests.insert(
                entity,
                {
                        TEXTURE_ASSET_ID::PUDDLE_2,
                        EFFECT_ASSET_ID::TEXTURED,
                        GEOMETRY_BUFFER_ID::SPRITE
                }
        );
    }
    else if (which_sprite == 3) {
        registry.renderRequests.insert(
                entity,
                {
                        TEXTURE_ASSET_ID::PUDDLE_3,
                        EFFECT_ASSET_ID::TEXTURED,
                        GEOMETRY_BUFFER_ID::SPRITE
                }
        );
    }
    else {
        // just in case, default to blood 1...
        registry.renderRequests.insert(
                entity,
                {
                        TEXTURE_ASSET_ID::PUDDLE_2,
                        EFFECT_ASSET_ID::TEXTURED,
                        GEOMETRY_BUFFER_ID::SPRITE
                }
        );
    }

    registry.backLayers.emplace(entity);

    RenderGroup& rg = registry.renderGroups.emplace(entity);
    rg.entities.push_back(entity);
    rg.indices.push_back(0);

    return entity;
}

Entity createEnemy(RenderSystem* renderer, vec2 pos, EnemyType type)
{
	auto entity = Entity();
	enemy_count++;
	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = { SPRITE_SIZE_STANDARD, SPRITE_SIZE_STANDARD };
	motion.can_collide = true;

	vec4& c = registry.colors.emplace(entity);
	c = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	// create an empty Enemy component for our character
	registry.bulletCooldownTimers.emplace(entity);
	registry.damaging.emplace(entity);
	registry.knockbackers.emplace(entity);
	Enemy &enemy = registry.enemies.emplace(entity);
	registry.movables.emplace(entity);

    
    // define the keyframe data
    AnimationState& as = registry.animationStates.emplace(entity);
    as.num_ms = 100.0f;
    as.counter_ms = as.num_ms;
    as.num_rows = 8;
    as.num_cols = 4;
    as.col = 0;
    as.row = 0;

	Health& health = registry.healths.emplace(entity);

	if (type == EnemyType::DEFAULT) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::ENEMY,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::FIRE_ENEMY,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	}

	// create limbs for the entity
	auto limb = Entity();
	auto limb2 = Entity();
	registry.motions.emplace(limb);
	registry.motions.emplace(limb2);
	registry.limbs.emplace(limb);
	registry.limbs.emplace(limb2);
	Skeleton& skel = registry.skeletons.emplace(entity);
	skel.left_arm = limb;
	skel.right_arm = limb2;

	registry.renderRequests.insert(
		limb,
		{
			TEXTURE_ASSET_ID::LIMB2,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	registry.renderRequests.insert(
		limb2,
		{
			TEXTURE_ASSET_ID::LIMB2,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	// give the enemy a gun
	auto weapon = createWeapon(renderer, motion.position, false, TEXTURE_ASSET_ID::REVOLVER_METALLIC, WEAPON_TYPE::REVOLVER);
	HoldingWeapon& hw = registry.weaponHolders.emplace(entity);
	hw.weapon = weapon;
	Weapon& weapon_data = registry.weapons.get(weapon);
	weapon_data.being_held = true;

    if (type == EnemyType::DEFAULT){
        registry.renderRequests.insert(
                weapon,
                {
                        TEXTURE_ASSET_ID::REVOLVER,
                        EFFECT_ASSET_ID::TEXTURED,
                        GEOMETRY_BUFFER_ID::SPRITE
                }
        );
        enemy.weaponType = WEAPON_TYPE::REVOLVER;
    } else if (type == EnemyType::KNIFE){
        registry.renderRequests.insert(
                weapon,
                {
                        TEXTURE_ASSET_ID::KNIFE,
                        EFFECT_ASSET_ID::TEXTURED,
                        GEOMETRY_BUFFER_ID::SPRITE
                }
        );
        enemy.weaponType = WEAPON_TYPE::KNIFE;

    } else if (type == EnemyType::SNIPER){
        registry.renderRequests.insert(
                weapon,
                {
                        TEXTURE_ASSET_ID::RIFLE,
                        EFFECT_ASSET_ID::TEXTURED,
                        GEOMETRY_BUFFER_ID::SPRITE
                }
        );
        enemy.weaponType = WEAPON_TYPE::RIFLE;
    } else if (type == EnemyType::FIRE) {
		registry.renderRequests.insert(
			weapon,
			{
					TEXTURE_ASSET_ID::FIRE_BOTTLE_WEAPON,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE
			}
		);
	}
	else if (type == EnemyType::BLINDER) {
		registry.renderRequests.insert(
			weapon,
			{
				TEXTURE_ASSET_ID::BLANK_WEAPON,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);
	}

	enemy.enemyType = type;

	// knife enemies cause player to bleed
	if (type == EnemyType::KNIFE) {
		registry.bleedCausers.emplace(entity);
	}


    // define a RenderGroup used for determing the order for which sprites should be rendered in
	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities = {entity, limb, limb2, weapon};
	rg.indices = {0, 1, 2, 3};
	

	return entity;
}

// generic function to spawn an obstacle
Entity createObstacle(RenderSystem* renderer, 
					  vec2 pos, 
					  bool is_obstacle, 
					  bool damaging, 
					  bool knocks_back_player, 
					  bool knocks_back_enemy,
					  TEXTURE_ASSET_ID tex)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = {SPRITE_SIZE_STANDARD, SPRITE_SIZE_STANDARD };
	motion.can_collide = true;

	registry.worldObjects.emplace(entity);
	// create an empty Obstacle component for our character
	if (is_obstacle) {
		registry.obstacles.emplace(entity);
	}
	if (damaging) {
		registry.damaging.emplace(entity);
	}
	if (knocks_back_player || knocks_back_enemy) {
		HasKnockback& kb = registry.knockbackers.emplace(entity);
		if (knocks_back_player) {
			kb.is_player_affecting = true;
		}
		if (knocks_back_enemy) {
			kb.is_enemy_affecting = true;
		}
	}
	
    
    AnimationState& as = registry.animationStates.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ 	tex,  
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities.push_back(entity);
	rg.indices.push_back(0);

	return entity;
}

// some helper functions for common obstacles
Entity createCactus(RenderSystem* renderer, vec2 pos) {
	return createObstacle(renderer, pos, false, true, true, true, TEXTURE_ASSET_ID::CACTUS);
}

Entity createBarrel(RenderSystem* renderer, vec2 pos) {
	return createObstacle(renderer, pos, true, false, false, false, TEXTURE_ASSET_ID::BARREL);
}

Entity createMinecart(RenderSystem* renderer, vec2 pos) {
	return createObstacle(renderer, pos, true, false, false, false, TEXTURE_ASSET_ID::MINECART);
}

Entity createMinecartFull(RenderSystem* renderer, vec2 pos) {
	return createObstacle(renderer, pos, true, false, false, false, TEXTURE_ASSET_ID::MINECART_FULL);
}

Entity createStump(RenderSystem* renderer, vec2 pos) {
	return createObstacle(renderer, pos, true, false, false, false, TEXTURE_ASSET_ID::STUMP);
}

Entity createRock(RenderSystem* renderer, vec2 pos) {
	return createObstacle(renderer, pos, true, false, false, false, TEXTURE_ASSET_ID::ROCK);
}

Entity createThorny(RenderSystem* renderer, vec2 pos) {
	return createObstacle(renderer, pos, false, true, true, true, TEXTURE_ASSET_ID::THORNY_BUSH);
}

Entity createBush(RenderSystem* renderer, vec2 pos) {
	return createObstacle(renderer, pos, true, false, false, false, TEXTURE_ASSET_ID::BUSH);
}

Entity createFire(RenderSystem* renderer, vec2 pos) {
	Entity fire = createObstacle(renderer, pos, false, true, false, false, TEXTURE_ASSET_ID::EXPLOSION);
	Motion& fm = registry.motions.get(fire);
	fm.can_collide = true;

	AnimationState& as = registry.animationStates.get(fire);
	as.num_ms = 50.0f;
	as.counter_ms = as.num_ms;
	as.num_rows = 1;
	as.num_cols = 4;
	as.col = 0;
	as.row = 0;

	registry.canMoveThroughs.emplace(fire);
	registry.enemyDamaging.emplace(fire);
	registry.fires.emplace(fire);
    std::cout << "fire" << std::endl;

	return fire;
}


Entity createFireRing(RenderSystem* renderer, vec2 pos, int size) {
	Entity fire = createFire(renderer, pos);
	registry.motions.get(fire).scale = { SPRITE_SIZE_STANDARD * size, SPRITE_SIZE_STANDARD * size };
	registry.canMoveThroughs.get(fire).tiling_factor = vec2(3, 3);
	AnimationTimer& at = registry.animationTimers.emplace(fire);
	at.counter_ms = 3000.0f;
	return fire;
}

// NOTE: do not call this function directly!!! use createFenceWall instead
// (it is optimized for rendering "strips" or "blocks" of fences)
Entity createFence(RenderSystem* renderer, vec2 pos) {
	return createObstacle(renderer, pos, true, false, false, false, TEXTURE_ASSET_ID::FENCE);
}

// renders a strip of fence textures as a single entity
Entity createFenceWall(RenderSystem* renderer, vec2 pos, vec2 tiling_factor) {
	Entity fence_wall = createFence(renderer, pos);
	registry.motions.get(fence_wall).scale = {SPRITE_SIZE_STANDARD * tiling_factor.x, SPRITE_SIZE_STANDARD * tiling_factor.y};
    registry.obstacles.get(fence_wall).tiling_factor = tiling_factor;
	return fence_wall;
}

// renders a strip of fence textures as a single entity
Entity createStumpWall(RenderSystem* renderer, vec2 pos, vec2 tiling_factor) {
	Entity stump_wall = createStump(renderer, pos);
	registry.motions.get(stump_wall).scale = { SPRITE_SIZE_STANDARD * tiling_factor.x, SPRITE_SIZE_STANDARD * tiling_factor.y };
	registry.obstacles.get(stump_wall).tiling_factor = tiling_factor;
	return stump_wall;
}

// renders a strip of fence textures as a single entity
Entity createRockWall(RenderSystem* renderer, vec2 pos, vec2 tiling_factor) {
	Entity rock_wall = createRock(renderer, pos);
	registry.motions.get(rock_wall).scale = { SPRITE_SIZE_STANDARD * tiling_factor.x, SPRITE_SIZE_STANDARD * tiling_factor.y };
	registry.obstacles.get(rock_wall).tiling_factor = tiling_factor;
	return rock_wall;
}

Entity createFireBomb(RenderSystem* renderer, vec2 pos, float speed, float angle) {
	auto entity = Entity();
	registry.projectiles.emplace(entity);
	registry.bulletLifeTimes.emplace(entity);
	registry.fireBombs.emplace(entity);


	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = angle;
	motion.can_collide = true;

	// calculate direction of bullet
	float x = cosf(angle);
	float y = sinf(angle);
	vec2 dir_vec = vec2(x, y);

	if (dir_vec != vec2(0, 0)) {
		dir_vec = normalize(dir_vec);
	}

	motion.velocity = dir_vec * speed;
	motion.max_velocity = motion.velocity;

	motion.scale = { -SPRITE_SIZE_STANDARD * 0.5, SPRITE_SIZE_STANDARD * 0.5 };

	// generate bezier curve parameters
	PrescribedMotion& pm = registry.prescribedMotions.emplace(entity);
	pm.time_ms = 1000.0f;
	// starting point
	pm.start = pos;
	// ending point (player pos)
	vec2 player_pos = registry.motions.get(registry.players.entities[0]).position;
	pm.end = player_pos;
	// middle control point, forming an arcing trajectory.
	// Note that the effect is more pronounced in the east/west directions
	vec2 midpoint = {(pm.start.x + pm.end.x) / 2.0f,  (pm.start.y + pm.end.y) / 2.0f};
	float tweaked_angle = abs(angle);
	float height = 150 * abs(cos(tweaked_angle));
	pm.apex.x = midpoint.x + height * sin(tweaked_angle);
	pm.apex.y = midpoint.y - height * abs(cos(tweaked_angle));

	AnimationState& as = registry.animationStates.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FIRE_BOTTLE,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE }
	);

	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities.push_back(entity);
	rg.indices.push_back(0);

	return entity;
}

Entity createDirtBall(RenderSystem* renderer, vec2 pos, float speed, float angle) {
	auto entity = Entity();
	registry.projectiles.emplace(entity);
	registry.blinders.emplace(entity);
	registry.bulletLifeTimes.emplace(entity);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = angle;
	motion.can_collide = true;

	// calculate direction of bullet
	float x = cosf(angle);
	float y = sinf(angle);
	vec2 dir_vec = vec2(x, y);

	if (dir_vec != vec2(0, 0)) {
		dir_vec = normalize(dir_vec);
	}

	motion.velocity = dir_vec * speed;
	motion.max_velocity = motion.velocity;

	motion.scale = { BULLET_SIZE, BULLET_SIZE };

	// generate bezier curve parameters
	PrescribedMotion& pm = registry.prescribedMotions.emplace(entity);
	pm.time_ms = 350.0f;
	// starting point
	pm.start = pos;
	// ending point (player pos)
	vec2 player_pos = registry.motions.get(registry.players.entities[0]).position;
	pm.end = player_pos;
	// middle control point, forming an arcing trajectory.
	// Note that the effect is more pronounced in the east/west directions
	vec2 midpoint = {(pm.start.x + pm.end.x) / 2.0f,  (pm.start.y + pm.end.y) / 2.0f};
	float tweaked_angle = abs(angle);
	float height = 50 * abs(cos(tweaked_angle));
	pm.apex.x = midpoint.x + height * sin(tweaked_angle);
	pm.apex.y = midpoint.y - height * abs(cos(tweaked_angle));

	AnimationState& as = registry.animationStates.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DIRT_BALL,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE }
	);

	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities.push_back(entity);
	rg.indices.push_back(0);

	return entity;
}


Entity createBullet(RenderSystem* renderer, vec2 pos, float speed, float angle, bool enemyDamaging, int damage) {
	auto entity = Entity();
	registry.projectiles.emplace(entity);
	registry.bulletLifeTimes.emplace(entity);
	if (enemyDamaging) {
		registry.enemyDamaging.emplace(entity).val = damage;
        registry.knockbackers.emplace(entity);
        registry.knockbackers.get(entity).is_enemy_affecting = true;
		registry.knockbackers.get(entity).is_player_affecting = false;
    }
	else {
		registry.damaging.emplace(entity).val = damage;
		registry.knockbackers.emplace(entity);
        registry.knockbackers.get(entity).is_enemy_affecting = false;
		registry.knockbackers.get(entity).is_player_affecting = true;

    }

	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = angle;
	motion.can_collide = true;

	// calculate direction of bullet
	float x = cosf(angle);
	float y = sinf(angle);
	vec2 dir_vec = vec2(x, y);

	if (dir_vec != vec2(0, 0)) {
		dir_vec = normalize(dir_vec);
	}

	motion.velocity = dir_vec * speed;
    motion.max_velocity = motion.velocity;

	motion.scale = { -BULLET_SIZE, BULLET_SIZE };

    AnimationState& as = registry.animationStates.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BULLET,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE }
	);

	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities.push_back(entity);
	rg.indices.push_back(0);

	return entity;
}

Entity createDamagingBullet(RenderSystem* renderer, vec2 pos) {
	auto entity = Entity();
	registry.projectiles.emplace(entity);
	registry.bulletLifeTimes.emplace(entity);
	registry.damaging.emplace(entity);
	registry.knockbackers.emplace(entity);
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = { -BULLET_SIZE, BULLET_SIZE };
	motion.can_collide = true;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BULLET,
		EFFECT_ASSET_ID::COLOURED,
		GEOMETRY_BUFFER_ID::SPRITE }
	);

	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities.push_back(entity);
	rg.indices.push_back(0);

	return entity;
}

// Create the red health bar. Position should be fixed (in screen coords). Scale starts at an initial value, is updated with player health.
// Uses the 'Bar' geometry as a base.

Entity createHealthbar(vec2 position) {
	Entity entity = Entity();

	vec4& thisColor = registry.colors.emplace(entity);

	// color this red

	thisColor[0] = 1.0f;
	thisColor[1] = 0.0f;
	thisColor[2] = 0.0f;
	thisColor[3] = 1.0f;


	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.0f;
	motion.velocity = { 0, 0 };
	motion.scale = { BAR_LENGTH_STANDARD, BAR_HEIGHT_STANDARD};

	registry.healthbars.emplace(entity);
	registry.uiElements.emplace(entity);
	return entity;

}

Entity createTitleScreenButton(vec2 position, TITLE_BUTTON_TYPE button_type) {
	Entity entity = Entity();
	vec4& thisColor = registry.colors.emplace(entity);

	thisColor = vec4(0.6, 0.6, 0.6, 1.0);
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.0f;
	motion.velocity = { 0, 0 };
	motion.scale = { 0, 0 };

	TitleScreenButton& tsb = registry.titleScreenButtons.emplace(entity);
	tsb.scale_x = BUTTON_WIDTH;
	tsb.scale_y = BUTTON_HEIGHT;
	tsb.button = button_type;
	registry.uiElements.emplace(entity);

	registry.renderRequests.insert(
		entity, {
					TEXTURE_ASSET_ID::TEXTURE_COUNT,
					EFFECT_ASSET_ID::COLOURED,
					GEOMETRY_BUFFER_ID::BAR
		}
	);
	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities.push_back(entity);
	rg.indices.push_back(0);

	motion.scale.x = tsb.scale_x;
	motion.scale.y = tsb.scale_y;

	return entity;
}

Entity createHelpPopupBG(vec2 position) {
	Entity entity = Entity();
	vec4& thisColor = registry.colors.emplace(entity);

	thisColor = vec4(0.431, 0.361, 0.275, 0.9);
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.0f;
	motion.velocity = { 0, 0 };
	motion.scale = { 0, 0 };

	HelpPopupBG& hpb = registry.helpPopupBGs.emplace(entity);
	hpb.scale_x = HELP_POPUP_WIDTH;
	hpb.scale_y = HELP_POPUP_HEIGHT;
	registry.uiElements.emplace(entity);

	return entity;
}

Entity createInventoryBG(vec2 position) {
	Entity entity = Entity();
	vec4& thisColor = registry.colors.emplace(entity);
	thisColor = vec4(0.431, 0.361, 0.275, 0.9);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.0f;
	motion.velocity = { 0, 0 };
	motion.scale = { 0, 0 };

	InventoryBG& ibg = registry.inventoryBGs.emplace(entity);
	ibg.scale_x = HELP_POPUP_WIDTH;
	ibg.scale_y = HELP_POPUP_HEIGHT;
	registry.uiElements.emplace(entity);

	return entity;
}


Entity createStaminabar(vec2 position) {
	Entity entity = Entity();

	vec4& thisColor = registry.colors.emplace(entity);

	// color this green

	thisColor[0] = 0.0f;
	thisColor[1] = 1.0f;
	thisColor[2] = 0.0f;
	thisColor[3] = 1.0f;

	// this should go right underneath the health bar...

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.0f;
	motion.velocity = { 0, 0 };
	motion.scale = { BAR_LENGTH_STANDARD, BAR_HEIGHT_STANDARD };

	registry.staminabars.emplace(entity);
	registry.uiElements.emplace(entity);

	return entity;

}

Entity createAmmoCounter(vec2 position, vec4 color) {
	Entity entity = Entity();
	vec4& thisColor = registry.colors.emplace(entity);
	thisColor = color;

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.0f;
	motion.velocity = { 0, 0 };
	motion.scale = { SPRITE_SIZE_AMMO_COUNTER, SPRITE_SIZE_AMMO_COUNTER };

	registry.ammoCounters.emplace(entity);
	registry.uiElements.emplace(entity);
	return entity;
}



Entity createReloadIndicator(vec2 position) {
	Entity entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position; 
	motion.angle = 0.0f;
	motion.velocity = { 0, 0 };
	motion.scale = { 0, 0 };

	registry.reloadIndicators.emplace(entity);
	registry.uiElements.emplace(entity);
	registry.animationStates.emplace(entity);
	return entity;
}



Entity createConsumableIndicator(vec2 position, Entity entity, int keybind) {
    Motion& motion = registry.motions.get(entity);
    motion.position = position;
    motion.angle = 0.0f;
    motion.velocity = { 0, 0 };
    motion.scale = { SPRITE_SIZE_STANDARD/2 , SPRITE_SIZE_STANDARD/2 };

    if (!registry.consumableItems.has(entity)){
        ConsumableIndicator &ci = registry.consumableIndicators.emplace(entity);
    }
    if (!registry.uiElements.has(entity)){
        registry.uiElements.emplace(entity);
    }

    return entity;
}

Entity createWeaponIndicator(vec2 position) {
	Entity entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.0f;
	motion.velocity = { 0, 0 };
	motion.scale = { SPRITE_SIZE_STANDARD * 0.75f , SPRITE_SIZE_STANDARD * 0.75f };

	registry.weaponIndicators.emplace(entity);
	registry.uiElements.emplace(entity);

	AnimationState& as = registry.animationStates.emplace(entity);
	as.num_ms = 50.0f;
	as.counter_ms = as.num_ms;
	as.num_rows = 2;
	as.num_cols = 1;
	as.col = 0;
	as.row = 0;

	return entity;
}

const static int TNT_HP = 3;
Entity createTNT(vec2 position)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = { SPRITE_SIZE_STANDARD, SPRITE_SIZE_STANDARD };
	motion.can_collide = true;

	auto& tnt = registry.explodingTNTs.emplace(entity);
	tnt.hp = TNT_HP;
	tnt.hasExploded = false;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TNT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities.push_back(entity);
	rg.indices.push_back(0);

	return entity;
}

const static float EXPLOSION_SIZE = 4.0f;
const static float EXPLOSION_KNOCKBACK_STRENGTH = 15.0f;
const static float EXPLOSION_LIFETIME = 150.0f;
const static float EXPLOSION_DAMAGE_ENEMIES = 50.0f;
const static float EXPLOSION_DAMAGE_PLAYERS = 30.0f;
Entity createExplosion(vec2 position)
{
	auto entity = Entity();

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = { SPRITE_SIZE_STANDARD * EXPLOSION_SIZE, SPRITE_SIZE_STANDARD * EXPLOSION_SIZE };
	motion.can_collide = true;

	auto& explosion = registry.explosions.emplace(entity);
	explosion.knockbackStrength = EXPLOSION_KNOCKBACK_STRENGTH;
	explosion.lifetime = EXPLOSION_LIFETIME;

	auto& enemyDamage = registry.enemyDamaging.emplace(entity);
	enemyDamage.val = EXPLOSION_DAMAGE_ENEMIES;
	auto& damage = registry.damaging.emplace(entity);
	damage.val = EXPLOSION_DAMAGE_PLAYERS;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TNTEXPLOSION,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities.push_back(entity);
	rg.indices.push_back(0);

	return entity;
}

Entity createBorder(vec2 position) {
    Entity entity = Entity();
    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.angle = 0.0f;
    motion.velocity = { 0, 0 };
    motion.scale = { SPRITE_SIZE_STANDARD * 0.75f , SPRITE_SIZE_STANDARD * 0.75f };

    registry.uiElements.emplace(entity);

    registry.renderRequests.insert(
            entity,
            {
                    TEXTURE_ASSET_ID::BORDER,
                    EFFECT_ASSET_ID::TEXTURED,
                    GEOMETRY_BUFFER_ID::SPRITE
            }
    );

    // NOTE: Ensure that this component gets deleted on pickup
    RenderGroup& rg = registry.renderGroups.emplace(entity);
    rg.entities.push_back(entity);
    rg.indices.push_back(0);


    return entity;
}

Entity createText(std::string text, vec2 position, vec3 color, bool followsPlayer) {
	Entity entity = Entity();
	IsText& isText = registry.texts.emplace(entity);
	isText.text = text;
	isText.pos = position;
	isText.color = color;
	isText.doRender = false;
	isText.follows_player = followsPlayer;
	return entity;
}

// create an enemy health bar (small)

Entity createEnemyHealthBar(vec2 position) {
	Entity entity = Entity();

	vec4& thisColor = registry.colors.emplace(entity);

	// color this red

	thisColor[0] = 1.0f;
	thisColor[1] = 0.0f;
	thisColor[2] = 0.0f;
	thisColor[3] = 1.0f;


	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.angle = 0.0f;
	motion.velocity = { 0, 0 };
	motion.scale = { BAR_LENGTH_ENEMY, BAR_HEIGHT_ENEMY };

	registry.enemyHealthBars.emplace(entity);

	if (!registry.renderRequests.has(entity)) {
		registry.renderRequests.insert(
			entity, {
				TEXTURE_ASSET_ID::TEXTURE_COUNT,
				EFFECT_ASSET_ID::COLOURED,
				GEOMETRY_BUFFER_ID::BAR
			}
		);
	}

	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities.push_back(entity);
	rg.indices.push_back(0);

	return entity;

}

// NOTE: if dropped == false, it is assumed that it will get assigned to a player
// to get rendered later
Entity createWeapon(RenderSystem* renderer, vec2 pos, bool dropped, TEXTURE_ASSET_ID tex, WEAPON_TYPE type) {
	auto entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = { SPRITE_SIZE_STANDARD / 2, SPRITE_SIZE_STANDARD / 2 };

	 Weapon& weapon_data = registry.weapons.emplace(entity);
	// set the weapon texture
	weapon_data.tex = tex;
	// set the weapon type
	weapon_data.type = type;
	// set weapon max ammo
	weapon_data.max_ammo = WEAPON_AMMO_COUNT[type];
	weapon_data.cur_ammo = weapon_data.max_ammo;

    AnimationState& as = registry.animationStates.emplace(entity);
	as.num_rows = 2;

	if (dropped) {
		registry.renderRequests.insert(
			entity,
			{ tex,
			  EFFECT_ASSET_ID::TEXTURED,
			  GEOMETRY_BUFFER_ID::SPRITE }
		);

		// NOTE: Ensure that this component gets deleted on pickup
		RenderGroup& rg = registry.renderGroups.emplace(entity);
		rg.entities.push_back(entity);
		rg.indices.push_back(0);

		motion.can_collide = true;

	}

	return entity;
}

Entity createInventoryImage(vec2 pos, TEXTURE_ASSET_ID tex) {
	Entity entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.0f;
	motion.velocity = { 0, 0 };
	motion.scale = { 0 , 0 };

	InventoryImage& ii = registry.inventoryImages.emplace(entity);
	ii.scale_x = SPRITE_SIZE_STANDARD * 0.75f;
	ii.scale_y = SPRITE_SIZE_STANDARD * 0.75f;
	ii.tex = tex;

	registry.uiElements.emplace(entity);

	return entity;
}

Entity createPassiveItem(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID tex, int id) {
	auto entity = Entity();
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = vec2(SPRITE_SIZE_STANDARD / 2, SPRITE_SIZE_STANDARD / 2);
	motion.can_collide = true;

	PassiveItem& pi = registry.passiveItems.emplace(entity);
	pi.id = id;
	pi.tex = tex;

	registry.renderRequests.insert(
		entity,
		{
			tex,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	// NOTE: Ensure that this component gets deleted on pickup
	RenderGroup& rg = registry.renderGroups.emplace(entity);
	rg.entities.push_back(entity);
	rg.indices.push_back(0);
	

	return entity;
}

Entity createConsumableItem(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID tex, Consumable id) {
    auto entity = Entity();
    Motion& motion = registry.motions.emplace(entity);
    motion.position = pos;
    motion.scale = vec2(SPRITE_SIZE_STANDARD / 2, SPRITE_SIZE_STANDARD / 2);
    motion.can_collide = true;

    ConsumableItem& pi = registry.consumableItems.emplace(entity);
    pi.id = id;
    pi.tex = tex;

    registry.renderRequests.insert(
            entity,
            {
                    tex,
                    EFFECT_ASSET_ID::TEXTURED,
                    GEOMETRY_BUFFER_ID::SPRITE
            }
    );

    // NOTE: Ensure that this component gets deleted on pickup
    RenderGroup& rg = registry.renderGroups.emplace(entity);
    rg.entities.push_back(entity);
    rg.indices.push_back(0);

    return entity;
}

// background image
Entity createBackground(RenderSystem* renderer) {
    auto entity = Entity();
    registry.backgrounds.emplace(entity);
    registry.animationStates.emplace(entity);
    registry.renderRequests.insert(
            entity,
            { TEXTURE_ASSET_ID::BACKGROUND,
              EFFECT_ASSET_ID::TEXTURED,
              GEOMETRY_BUFFER_ID::SPRITE }
            );

	// RenderGroup& rg = registry.renderGroups.emplace(entity);
	// rg.entities.push_back(entity);
	// rg.indices.push_back(0);

    return entity;
}

// title screen image
Entity createTitleScreen(RenderSystem* renderer) {
	auto entity = Entity();
	registry.titleScreens.emplace(entity);
	registry.animationStates.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TITLESCREEN,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE }
	);

	// RenderGroup& rg = registry.renderGroups.emplace(entity);
	// rg.entities.push_back(entity);
	// rg.indices.push_back(0);

	return entity;
}

// background image
Entity createBackgroundGrass(RenderSystem* renderer) {
	auto entity = Entity();
	registry.backgrounds.emplace(entity);
	registry.animationStates.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GRASS,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE }
	);

	// RenderGroup& rg = registry.renderGroups.emplace(entity);
	// rg.entities.push_back(entity);
	// rg.indices.push_back(0);

	return entity;
}

// background image
Entity createBackgroundRock(RenderSystem* renderer) {
	auto entity = Entity();
	registry.backgrounds.emplace(entity);
	registry.animationStates.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::STONE,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE }
	);

	// RenderGroup& rg = registry.renderGroups.emplace(entity);
	// rg.entities.push_back(entity);
	// rg.indices.push_back(0);

	return entity;
}


Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity, {
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		});

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);

    RenderGroup& rg = registry.renderGroups.emplace(entity);
    rg.entities.push_back(entity);
    rg.indices.push_back(0);

	return entity;
}

Entity createParticle(Entity parent, vec2 start, vec2 dist, vec2 scale, float angle, vec4 color)
{
	Entity entity = Entity();
	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.scale = scale;
	motion.angle = angle;

	motion.position = start;

	Particle& part_data = registry.particles.emplace(entity);
	part_data.parent = parent;
	part_data.dist = dist;

	vec4& thisColor = registry.colors.emplace(entity);
	thisColor = color;


	return entity;
}

// create an "explosion" of particles (named burst to avoid confusion with barrel explosions?)
void createParticleBurst(Entity parent, unsigned int num_particles, vec2 dist_range, vec2 scale_range, vec4 color)
{
	RNG rng;
	Motion& parent_motion = registry.motions.get(parent);
	for (unsigned int i = 0; i < num_particles; ++i) {
		float rot_angle = rng.rand_float(radians(-180.0f), radians(180.0f));
		float dist_angle = rng.rand_float(radians(-180.0f), radians(180.0f));
		float dist = rng.rand_float(dist_range.x, dist_range.y);
		float dist_x = dist * sin(dist_angle);
		float dist_y = dist * cos(dist_angle);
		float scale = rng.rand_float(scale_range.x, scale_range.y);
		createParticle(parent, parent_motion.position, {dist_x, dist_y}, {scale, scale}, rot_angle, color);

	}
}

// convenience function to create a "blood" effect
void createColoredBurst(Entity parent, vec4 color)
{
	createParticleBurst(parent, 15, vec2(30, 50), vec2(5, 10), color);
}

