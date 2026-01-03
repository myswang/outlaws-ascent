#pragma once
#include "SDL_cpuinfo.h"
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

#include "../ext/nlohmann/json.hpp"

#include <iostream>
#include "entity.hpp"


// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE 

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	BACKGROUND = 0,
	PLAYER = BACKGROUND + 1,
	ENEMY = PLAYER + 1,
	CACTUS = ENEMY + 1,
	BARREL = CACTUS + 1,
	FENCE = BARREL + 1,
	BULLET = FENCE + 1,
	REVOLVER = BULLET + 1,
	REVOLVER_METALLIC = REVOLVER + 1,
	AMMO_COUNTER = REVOLVER_METALLIC + 1,
	R_KEY = AMMO_COUNTER + 1,
	LIMB = R_KEY + 1,
	LIMB2 = LIMB + 1,
	SHOTGUN = LIMB2 + 1,
	RIFLE = SHOTGUN + 1,
	ELIXIR = RIFLE + 1,
	POWDER = ELIXIR + 1,
	PADDING = POWDER + 1,
	GUNMOD = PADDING + 1,
	SYRINGE = GUNMOD + 1,
	CHICKEN = SYRINGE + 1,
	LANDMINE = CHICKEN + 1,
	TNT = LANDMINE + 1,
	EXPLODED_TNT = TNT + 1,
	TNTEXPLOSION = EXPLODED_TNT + 1,
	EXPLOSION = TNTEXPLOSION + 1,
	STEW = EXPLOSION + 1,
	BORDER = STEW + 1,
	FIRE_ENEMY = BORDER + 1,
	FIRE_BOTTLE = FIRE_ENEMY + 1,
	FIRE_BOTTLE_WEAPON = FIRE_BOTTLE + 1,
	KNIFE = FIRE_BOTTLE_WEAPON + 1,
	POTION = KNIFE + 1,
	BLANK_WEAPON = POTION + 1,
	DIRT_BALL = BLANK_WEAPON + 1,
	BLOOD_1 = DIRT_BALL + 1,
	BLOOD_2 = BLOOD_1 + 1,
	BLOOD_3 = BLOOD_2 + 1,
	BUSH = BLOOD_3 + 1,
	THORNY_BUSH = BUSH + 1,
	STUMP = THORNY_BUSH + 1,
	MUD = STUMP + 1,
	OASIS = MUD + 1,
	PUDDLE_1 = OASIS + 1,
	PUDDLE_2 = PUDDLE_1 + 1,
	PUDDLE_3 = PUDDLE_2 + 1,
	ROCK = PUDDLE_3 + 1,
	MINECART = ROCK + 1,
	MINECART_FULL = MINECART + 1,
	GRASS = MINECART_FULL + 1,
	STONE = GRASS + 1,
	I_MOVE = STONE + 1,
	I_DASH = I_MOVE + 1,
	I_SHOOT = I_DASH + 1,
	I_RELOAD = I_SHOOT + 1,
	I_CHANGE_WEAPON = I_RELOAD + 1,
	I_CONSUMABLE = I_CHANGE_WEAPON + 1,
	TITLESCREEN = I_CONSUMABLE + 1,
	TEXTURE_COUNT = TITLESCREEN + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	TEXTURED = 0,
	LOWHEALTH = TEXTURED + 1,
	COLOURED = LOWHEALTH + 1,
	FONT = COLOURED + 1,
	PARTICLE = FONT + 1,
    EFFECT_COUNT = PARTICLE + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	CACTUS_MESH = 0,
	SPRITE = CACTUS_MESH + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	BAR = SCREEN_TRIANGLE + 1,
	TEXT = BAR + 1,
	PARTICLE = TEXT + 1,
	GEOMETRY_COUNT = PARTICLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

// defines a list of entities to issue RenderRequests for,
// and their indices (order) in which they should be rendered
struct RenderGroup {
	std::vector<Entity> entities;
	std::vector<unsigned int> indices;
};

// defines a single particle and its parent entity
struct Particle
{
	Entity parent;
	vec2 dist;
	float counter_ms = 0.0f;
	float max_ms = 600.0f;
};

// Player component
struct Player
{
    // angle of mouse direction for aiming
    float angle = 0;
    // zoom level for camera (NOT sprite size)
    vec2 camera_scale = { 1.0f, 1.0f };
    vec2 camera_pos = { 0, 0 };
	vec2 camera_pos_old = { 0, 0 };

    Entity consumables[4];
    Effects worldEffect = Effects::NONE;

//    Entity worldEffect;
};

struct EffectTimer{
    float counter_ms = 3000;
};

// Arms component
struct Limb {

};

struct Skeleton
{
	Entity left_arm;
	Entity right_arm;
};

// components for health and stamina bars
struct HealthBar {

};

struct StaminaBar {

};

struct FPSCounter {

};

struct IsText {
	std::string text;
	bool doRender;
	bool follows_player;
	vec2 pos;
	vec3 color;
};

// component for enemy health bars (small), separate from player health bar (large)
struct EnemyHealthBar {

};

// components for enemies with their own health and stamina bars
struct HasHealthBar {
	Entity healthBar;
};

struct Fire {

};

struct LevelState {
	int whichLevel = 0;
};

struct WorldEffect{
    Effects worldEffect = Effects::NONE;
};

// Enemy component
struct Enemy
{
    float angle = 0;
    std::vector<vec2> path = {};
    EnemyType enemyType = EnemyType::DEFAULT;
    WEAPON_TYPE weaponType = WEAPON_TYPE::REVOLVER;
    Effects worldEffect = Effects::NONE;
};

struct Following {

};

struct Attacking {
    float counter_ms = 3000.f;
};

struct Fleeing {

};

// Weapon component
struct Weapon {
	bool being_held = false;
	WEAPON_TYPE type = WEAPON_TYPE::REVOLVER;
	TEXTURE_ASSET_ID tex = TEXTURE_ASSET_ID::REVOLVER;
	int cur_ammo = 6;
	int max_ammo = 6;
};

// This entity is a weapon that is currently equipped on the player (not to be confused with active weapon)
struct EquippedWeapon {

};

// struct for passive item render on the ground
struct PassiveItem {
	int id = P_HEALTHUP;
	TEXTURE_ASSET_ID tex = TEXTURE_ASSET_ID::ELIXIR;
};

struct ConsumableItem {
    Consumable id = Consumable::CHICKEN;
    TEXTURE_ASSET_ID tex = TEXTURE_ASSET_ID::ELIXIR;
    int keyBind = GLFW_KEY_1;
    bool player_collidable = true;
	bool placed = false;
};

// struct for a player with passive items
// NOTE: Edit this as we add more passive items!

struct HasPassiveItem {
	std::vector<int> items_list;
};

struct PassiveTextTimer {
	float counter_ms = 3000;
};

struct InventoryBG {
	float scale_x;
	float scale_y;
};

struct InventoryText {
	bool is_title;
	int index;
};

struct EnemyCounterText {

};

struct TutorialText {
    TutorialState state = TutorialState::WASD;
};

struct InventoryImage {
	float scale_x; 
	float scale_y;
	TEXTURE_ASSET_ID tex = TEXTURE_ASSET_ID::ELIXIR;
};

struct InventoryBorder {

};

struct PickedUpItemTimer {
	// should be the same as above
	float counter_ms = 3000;
};

struct PauseText {

};

// each individual 'ball' in the ammo counter
struct AmmoCounter {

};

// HoldingWeapon component
struct HoldingWeapon {
	Entity weapon;
};

// background image, can be resized according to world size
struct Background
{
    vec2 position = { 0, 0 };
    vec2 scale = { 1.0f, 1.0f };
    vec2 tiling_factor = { 1.0f, 1.0f };
};

// title screen, cannot be resized
struct TitleScreen
{
	vec2 position = { 0, 0 };
	vec2 scale = { 1.0f, 1.0f };
	vec2 tiling_factor = { 1.0f, 1.0f };
	bool doRender = false;
};

// Entity health
struct Health
{
	int val = 100;
};

// Entity stamina
struct Stamina
{
	int val = 100;
};

// a projectile, must have interpolated movement
struct Projectile
{

};

struct PrescribedMotion
{
	vec2 start;
	vec2 apex;
	vec2 end;
	float time_ms;
};

struct FireBomb 
{
	vec2 start;
	vec2 apex;
	vec2 end;
};

// an obstacle, must impede the player's movement when touched
struct Obstacle
{
	vec2 tiling_factor = {1.0f, 1.0f};
};

// something that slows player's movement when over it (water, mud, etc)
struct Slower
{

};

// anything that can damage the player on touch
struct Damaging
{
	int val = 10;
};
 
struct EnemyDamaging
{
	int val = 10;
};

struct Blinder
{

};

struct BlindTimer {
	float counter_ms = 5000.f;
};

struct BleedTimer {
	float counter_ms = 5000.0f;
	float blood_ms = 500.0f;
	float dmg_ms = 1000.0f;
};

struct PuddleTimer {
    float counter_ms = 5000.0f;
    float puddle_ms = 500.0f;
};

struct CausesBleeding {

};

struct CanMoveThrough {
	vec2 tiling_factor;
};


struct TNT {
	int hp;
	bool hasExploded = false;
};

struct Explosion {
	//1s lifetime
	float lifetime;
	float knockbackStrength;
	float damage;
	std::vector<Entity*> entitiesHit;
};


struct WorldObject {

};

// anything that, when collided with, knocks back the collider
struct HasKnockback
{
    bool is_enemy_affecting = false;
	bool is_player_affecting = false;
};

// data for current keyframe, and time since last frame
struct AnimationState
{
    float counter_ms = 0; // time since last frame
    float num_ms = 0;     // reset counter_ms to this value
    unsigned int row = 0;   // the set of keyframes to use
    unsigned int col = 0;   // which frame we are on
    unsigned int num_rows = 1;
    unsigned int num_cols = 1;
};

// timer determining knockback duration (object has normal velocity overridden temporarily)
struct KnockbackTimer
{
	float counter_ms = 200;
};

// timer for duration of temporary things such as explosions
struct LifeTimer
{
	float counter_ms = 3000;
};

// timer for cooldown of enemy shooting another bullet
struct BulletCooldownTimer
{
	float counter_ms = 1000;
};

// timer for player shooting bullets cooldown

struct AttackTimer {
	float counter_ms = 500;
};

struct AnimationTimer {
    float counter_ms = 1000;
};

// timer for reloading delay

struct ReloadTimer {
	float counter_ms = 1000;
};

struct HurtSoundTimer {
	float counter_ms = 100.0f;
};

// a bullet's life time since it was fired
struct BulletLifeTime
{
	float counter_ms = 0;
};

struct BackLayer {

};


// timer for 'mercy invincibility' after getting hit (for the player)
struct InvincibleTimer
{
	float counter_ms = 1000;
};

// timer for cooldown of dash
struct DashCooldownTimer 
{
	float counter_ms = DASH_TIME_MS;
};

// things that can be moved by knockback...
struct Movable
{

};

struct Placeable {

};

// catch-all struct for ui elements - things that MUST move with the player
struct UIElement {

};


struct HelpPopupBG {
	bool do_render = false;
	float scale_x;
	float scale_y;
};

struct HelpText {
	int page;
};

struct TitleScreenButton {
	bool do_render = false;
	float scale_x;
	float scale_y;
	TITLE_BUTTON_TYPE button = TITLE_BUTTON_TYPE::NEW_GAME;
};

struct TitleScreenButtonText {

};

struct ReloadIndicator {

};

struct WeaponIndicator {
	TEXTURE_ASSET_ID tex = TEXTURE_ASSET_ID::REVOLVER;
};

struct ConsumableIndicator {
    int key_bind = 0;
    TEXTURE_ASSET_ID tex = TEXTURE_ASSET_ID::CHICKEN;
    Entity text;
};

struct CollisionTimer
{
    float counter_ms = 2000;
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0, 0 };
	float angle = 0;
	vec2 velocity = { 0, 0 };
	vec2 scale = { 10, 10 };
    vec2 max_velocity = {0,0};
	bool can_collide = false;
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other;};
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
    Entity item_description_text;
//    Entity instruction_text;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying salmon
struct DeathTimer
{
	float counter_ms = 3000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & salmon.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

// component toJSON and fromJSON methods
using json = nlohmann::json;

// These generic functions work for empty structs and structs that dont need saving, Collisions for example
template <typename T>
void toJSON(json& j, const T& component) {
	// generic fallback: when no specific Component toJSON function is defined for that component
	// do nothing
};

template <typename T>
void fromJSON(const json& j, T& component) {
	// generic fallback: when no specific Component fromJSON function is defined for that component
	// do nothing
};

// Player
inline void toJSON(json& j, const Player& component) {
	json consumables_json = json::array();
	for (int i = 0; i < 4; i++) {
		consumables_json.push_back(static_cast<unsigned int>(component.consumables[i]));
	}
	j = json{
		{"angle", component.angle},
		{"camera_scale", {component.camera_scale.x, component.camera_scale.y}},
		{"camera_pos", {component.camera_pos.x, component.camera_pos.y}},
		{"camera_pos_old", {component.camera_pos_old.x, component.camera_pos_old.y}},
		{"consumables", consumables_json},
        {"worldEffect", component.worldEffect}
	};
}


inline void fromJSON(const json& j, Player& component) {
	j.at("angle").get_to(component.angle);
	component.camera_scale = vec2(j.at("camera_scale")[0], j.at("camera_scale")[1]);
	component.camera_pos = vec2(j.at("camera_pos")[0], j.at("camera_pos")[1]);
	component.camera_pos_old = vec2(j.at("camera_pos_old")[0], j.at("camera_pos_old")[1]);
	for (int i = 0; i < 4; i++) {
		unsigned int e_id = j.at("consumables")[i];
		Entity entity;
		if (map_saved_entity_new_entity.count(e_id) > 0) {
			entity = map_saved_entity_new_entity.at(e_id);
		}
		else {
			entity = Entity();
			map_saved_entity_new_entity[e_id] = entity;
		}
		component.consumables[i] = entity;
	}
    j.at("worldEffect").get_to(component.worldEffect);
}

// Motion
inline void toJSON(json& j, const Motion& component) {
	j = json{
		{"position", {component.position.x, component.position.y}},
		{"angle", component.angle},
		{"velocity", {component.velocity.x, component.velocity.y}},
		{"scale", {component.scale.x, component.scale.y}},
		{"max_velocity", {component.max_velocity.x, component.max_velocity.y}},
		{"can_collide", component.can_collide}
	};
}

inline void fromJSON(const json& j, Motion& component) {
	component.position = vec2(j.at("position")[0], j.at("position")[1]);
	j.at("angle").get_to(component.angle);
	component.velocity = vec2(j.at("velocity")[0], j.at("velocity")[1]);
	component.scale = vec2(j.at("scale")[0], j.at("scale")[1]);
	component.max_velocity = vec2(j.at("max_velocity")[0], j.at("max_velocity")[1]);
	j.at("can_collide").get_to(component.can_collide);
}

// RenderRequest
inline void toJSON(json& j, const RenderRequest& component) {
	j = json{
		{"used_texture", component.used_texture},
		{"used_effect", component.used_effect},
		{"used_geometry", component.used_geometry}
	};
}

inline void fromJSON(const json& j, RenderRequest& component) {
	j.at("used_texture").get_to(component.used_texture);
	j.at("used_effect").get_to(component.used_effect);
	j.at("used_geometry").get_to(component.used_geometry);
}

// RenderGroup
inline void toJSON(json& j, const RenderGroup& component) {
	// entities vector
	json entities_json = json::array();
	for (const Entity& e : component.entities) {
		entities_json.push_back(static_cast<unsigned int>(e));
	}

	// indices vector
	json indices_json = json::array();
	for (unsigned int index : component.indices) {
		indices_json.push_back(index);
	}

	// create json object for RenderGroup
	j = json{
		{"entities", entities_json},
		{"indices", indices_json}
	};
}

inline void fromJSON(const json& j, RenderGroup& component) {
	// Deserialize the entities vector
	component.entities.clear();
	for (const unsigned int& e_id : j.at("entities")) {
		Entity entity;
		if (map_saved_entity_new_entity.count(e_id) > 0) {
			entity = map_saved_entity_new_entity.at(e_id);
		}
		else {
			entity = Entity();
			map_saved_entity_new_entity[e_id] = entity;
		}
		component.entities.push_back(entity);
	}

	// Deserialize the indices vector
	component.indices.clear();
	for (const auto& index : j.at("indices")) {
		component.indices.push_back(index);
	}
}

// Skeleton
inline void toJSON(json& j, const Skeleton& component) {
	j = json{
		{"left_arm", static_cast<unsigned int>(component.left_arm)},
		{"right_arm", static_cast<unsigned int>(component.right_arm)}
	};
}


inline void fromJSON(const json& j, Skeleton& component) {
	unsigned int e_id = j.at("left_arm");
	Entity entity;
	if (map_saved_entity_new_entity.count(e_id) > 0) {
		entity = map_saved_entity_new_entity.at(e_id);
	}
	else {
		entity = Entity();
		map_saved_entity_new_entity[e_id] = entity;
	}
	component.left_arm = entity;

	unsigned int e_id2 = j.at("right_arm");
	Entity entity2;
	if (map_saved_entity_new_entity.count(e_id2) > 0) {
		entity2 = map_saved_entity_new_entity.at(e_id2);
	}
	else {
		entity2 = Entity();
		map_saved_entity_new_entity[e_id2] = entity2;
	}
	component.right_arm = entity2;
}

// IsText
inline void toJSON(json& j, const IsText& component) {
	j = json{
		{"text", component.text},
		{"doRender", component.doRender},
		{"follows_player", component.follows_player},
		{"pos", {component.pos.x, component.pos.y}},
		{"color", {component.color.x, component.color.y, component.color.z}}
	};
}

inline void fromJSON(const json& j, IsText& component) {
	j.at("text").get_to(component.text);
	j.at("doRender").get_to(component.doRender);
	j.at("follows_player").get_to(component.follows_player);
	component.pos = vec2(j.at("pos")[0], j.at("pos")[1]);
	component.color = vec3(j.at("color")[0], j.at("color")[1], j.at("color")[2]);
}

// HasHealthBar
inline void toJSON(json& j, const HasHealthBar& component) {
	j = json{
		{"healthBar", static_cast<unsigned int>(component.healthBar)}
	};
}


inline void fromJSON(const json& j, HasHealthBar& component) {
	unsigned int e_id = j.at("healthBar");
	Entity entity;
	if (map_saved_entity_new_entity.count(e_id) > 0) {
		entity = map_saved_entity_new_entity.at(e_id);
	}
	else {
		entity = Entity();
		map_saved_entity_new_entity[e_id] = entity;
	}
	component.healthBar = entity;
}

// Enemy updated with enemyType and weaponType
inline void toJSON(json& j, const Enemy& component) {
	// path vector of vec2
	json path_json = json::array();
	for (const auto& index : component.path) {
		path_json.push_back({index.x, index.y});
	}
	j = json{
		{"angle", component.angle},
		{"path", path_json},
		{"enemyType", component.enemyType},
		{"weaponType", component.weaponType},
        {"worldEffect", component.worldEffect}
    };
}

inline void fromJSON(const json& j, Enemy& component) {
	j.at("angle").get_to(component.angle);

	if (j.contains("path")) {
		// Deserialize the path vector
		component.path.clear();
		for (const auto& index : j.at("path")) {
			component.path.push_back(vec2(index[0], index[1]));
		}
	}
	j.at("enemyType").get_to(component.enemyType);
	j.at("weaponType").get_to(component.weaponType);
    j.at("worldEffect").get_to(component.worldEffect);
}

// Weapon
inline void toJSON(json& j, const Weapon& component) {
	j = json{
		{"being_held", component.being_held},
		{"type", component.type},
		{"tex", component.tex},
		{"cur_ammo", component.cur_ammo},
		{"max_ammo", component.max_ammo}
	};
}

inline void fromJSON(const json& j, Weapon& component) {
	j.at("being_held").get_to(component.being_held);
	j.at("type").get_to(component.type);
	j.at("tex").get_to(component.tex);
	j.at("cur_ammo").get_to(component.cur_ammo);
	j.at("max_ammo").get_to(component.max_ammo);
}

// PassiveItem
inline void toJSON(json& j, const PassiveItem& component) {
	j = json{
		{"id", component.id},
		{"tex", component.tex}
	};
}

inline void fromJSON(const json& j, PassiveItem& component) {
	j.at("id").get_to(component.id);
	j.at("tex").get_to(component.tex);
}

// ConsumableItem
inline void toJSON(json& j, const ConsumableItem& component) {
	j = json{
		{"id", component.id},
		{"tex", component.tex},
		{"keyBind", component.keyBind},
		{"player_collidable", component.player_collidable},
		{"placed", component.placed}
	};
}

inline void fromJSON(const json& j, ConsumableItem& component) {
	j.at("id").get_to(component.id);
	j.at("tex").get_to(component.tex);
	j.at("keyBind").get_to(component.keyBind);
	j.at("player_collidable").get_to(component.player_collidable);
	j.at("placed").get_to(component.placed);
}

// HasPassiveItem
inline void toJSON(json& j, const HasPassiveItem& component) {
	// items_list vector
	json items_list_json = json::array();
	for (unsigned int index : component.items_list) {
		items_list_json.push_back(index);
	}

	// create json object for RenderGroup
	j = json{
		{"items_list", items_list_json}
	};
}

inline void fromJSON(const json& j, HasPassiveItem& component) {
	// Deserialize the items_list vector
	component.items_list.clear();
	for (const auto& index : j.at("items_list")) {
		component.items_list.push_back(index);
	}
}

// PassiveTextTimer
inline void toJSON(json& j, const PassiveTextTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, PassiveTextTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// InventoryBG
inline void toJSON(json& j, const InventoryBG& component) {
	j = json{
		{"scale_x", component.scale_x},
		{"scale_y", component.scale_y}
	};
}

inline void fromJSON(const json& j, InventoryBG& component) {
	j.at("scale_x").get_to(component.scale_x);
	j.at("scale_y").get_to(component.scale_y);
}

// InventoryText
inline void toJSON(json& j, const InventoryText& component) {
	j = json{
		{"is_title", component.is_title},
		{"index", component.index}
	};
}

inline void fromJSON(const json& j, InventoryText& component) {
	j.at("is_title").get_to(component.is_title);
	j.at("index").get_to(component.index);
}

// InventoryImage
inline void toJSON(json& j, const InventoryImage& component) {
	j = json{
		{"scale_x", component.scale_x},
		{"scale_y", component.scale_y},
		{"tex", component.tex}
	};
}

inline void fromJSON(const json& j, InventoryImage& component) {
	j.at("scale_x").get_to(component.scale_x);
	j.at("scale_y").get_to(component.scale_y);
	j.at("tex").get_to(component.tex);
}

// PickedUpItemTimer
inline void toJSON(json& j, const PickedUpItemTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, PickedUpItemTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// HoldingWeapon
inline void toJSON(json& j, const HoldingWeapon& component) {
	j = json{
		{"weapon", static_cast<unsigned int>(component.weapon)}
	};
}


inline void fromJSON(const json& j, HoldingWeapon& component) {
	unsigned int e_id = j.at("weapon");
	Entity entity;
	if (map_saved_entity_new_entity.count(e_id) > 0) {
		entity = map_saved_entity_new_entity.at(e_id);
	}
	else {
		entity = Entity();
		map_saved_entity_new_entity[e_id] = entity;
	}
	component.weapon = entity;
}

// Background
inline void toJSON(json& j, const Background& component) {
	j = json{
		{"position", {component.position.x, component.position.y}},
		{"scale", {component.scale.x, component.scale.y}},
		{"tiling_factor", {component.tiling_factor.x, component.tiling_factor.y}}
	};
}

inline void fromJSON(const json& j, Background& component) {
	component.position = vec2(j.at("position")[0], j.at("position")[1]);
	component.scale = vec2(j.at("scale")[0], j.at("scale")[1]);
	component.tiling_factor = vec2(j.at("tiling_factor")[0], j.at("tiling_factor")[1]);
}

// Health
inline void toJSON(json& j, const Health& component) {
	j = json{
		{"val", component.val}
	};
}

inline void fromJSON(const json& j, Health& component) {
	j.at("val").get_to(component.val);
}

// Stamina
inline void toJSON(json& j, const Stamina& component) {
	j = json{
		{"val", component.val}
	};
}

inline void fromJSON(const json& j, Stamina& component) {
	j.at("val").get_to(component.val);
}

// Obstacle
inline void toJSON(json& j, const Obstacle& component) {
	j = json{
		{"tiling_factor", {component.tiling_factor.x, component.tiling_factor.y}}
	};
}

inline void fromJSON(const json& j, Obstacle& component) {
	component.tiling_factor = vec2(j.at("tiling_factor")[0], j.at("tiling_factor")[1]);
}

// Damaging
inline void toJSON(json& j, const Damaging& component) {
	j = json{
		{"val", component.val}
	};
}

inline void fromJSON(const json& j, Damaging& component) {
	j.at("val").get_to(component.val);
}

// EnemyDamaging
inline void toJSON(json& j, const EnemyDamaging& component) {
	j = json{
		{"val", component.val}
	};
}

inline void fromJSON(const json& j, EnemyDamaging& component) {
	j.at("val").get_to(component.val);
}

// HasKnockback
inline void toJSON(json& j, const HasKnockback& component) {
	j = json{
		{"is_enemy_affecting", component.is_enemy_affecting},
		{"is_player_affecting", component.is_player_affecting}
	};
}

inline void fromJSON(const json& j, HasKnockback& component) {
	j.at("is_enemy_affecting").get_to(component.is_enemy_affecting);
	j.at("is_player_affecting").get_to(component.is_player_affecting);
}

// AnimationState
inline void toJSON(json& j, const AnimationState& component) {
	j = json{
		{"counter_ms", component.counter_ms},
		{"num_ms", component.num_ms},
		{"row", component.row},
		{"col", component.col},
		{"num_rows", component.num_rows},
		{"num_cols", component.num_cols}
	};
}

inline void fromJSON(const json& j, AnimationState& component) {
	j.at("counter_ms").get_to(component.counter_ms);
	j.at("num_ms").get_to(component.num_ms);
	j.at("row").get_to(component.row);
	j.at("col").get_to(component.col);
	j.at("num_rows").get_to(component.num_rows);
	j.at("num_cols").get_to(component.num_cols);
}

// KnockbackTimer
inline void toJSON(json& j, const KnockbackTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, KnockbackTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// LifeTimer
inline void toJSON(json& j, const LifeTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, LifeTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// BulletCooldownTimer
inline void toJSON(json& j, const BulletCooldownTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, BulletCooldownTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// AttackTimer
inline void toJSON(json& j, const AttackTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, AttackTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// AnimationTimer
inline void toJSON(json& j, const AnimationTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, AnimationTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// ReloadTimer
inline void toJSON(json& j, const ReloadTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, ReloadTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// BulletLifeTime
inline void toJSON(json& j, const BulletLifeTime& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, BulletLifeTime& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// InvincibleTimer
inline void toJSON(json& j, const InvincibleTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, InvincibleTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// DashCooldownTimer
inline void toJSON(json& j, const DashCooldownTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, DashCooldownTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// HelpPopupBG
inline void toJSON(json& j, const HelpPopupBG& component) {
	j = json{
		{"do_render", component.do_render},
		{"scale_x", component.scale_x},
		{"scale_y", component.scale_y}
	};
}

inline void fromJSON(const json& j, HelpPopupBG& component) {
	j.at("do_render").get_to(component.do_render);
	j.at("scale_x").get_to(component.scale_x);
	j.at("scale_y").get_to(component.scale_y);
}

// HelpText
inline void toJSON(json& j, const HelpText& component) {
	j = json{
		{"page", component.page}
	};
}

inline void fromJSON(const json& j, HelpText& component) {
	j.at("page").get_to(component.page);
}

// WeaponIndicator
inline void toJSON(json& j, const WeaponIndicator& component) {
	j = json{
		{"tex", component.tex}
	};
}

inline void fromJSON(const json& j, WeaponIndicator& component) {
	j.at("tex").get_to(component.tex);
}

// ConsumableIndicator
inline void toJSON(json& j, const ConsumableIndicator& component) {
	j = json{
		{"key_bind", component.key_bind},
		{"tex", component.tex},
		{"text", static_cast<unsigned int>(component.text)}
	};
}


inline void fromJSON(const json& j, ConsumableIndicator& component) {
	j.at("key_bind").get_to(component.key_bind);
	j.at("tex").get_to(component.tex);

	// deserialize text entity
	unsigned int e_id = j.at("text");
	Entity entity;
	if (map_saved_entity_new_entity.count(e_id) > 0) {
		entity = map_saved_entity_new_entity.at(e_id);
	}
	else {
		entity = Entity();
		map_saved_entity_new_entity[e_id] = entity;
	}
	component.text = entity;
}

// CollisionTimer
inline void toJSON(json& j, const CollisionTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, CollisionTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// DeathTimer
inline void toJSON(json& j, const DeathTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, DeathTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// vec4 (for colors)
inline void toJSON(json& j, const vec4& component) {
	j = json{
		{"xyzw", {component.x, component.y, component.z, component.w}}
	};
}

inline void fromJSON(const json& j, vec4& component) {
	component = vec4(j.at("xyzw")[0], j.at("xyzw")[1], j.at("xyzw")[2], j.at("xyzw")[3]);
}

 // Mesh* for meshPtrs
inline void toJSON(json& j, const Mesh* component) {
	std::cout << "this ran" << std::endl;
	// vertices vector
	json vertices_json = json::array();
	for (const auto& vertex : component->vertices) {
		vertices_json.push_back({ vertex.position.x, vertex.position.y, vertex.position.z, vertex.color.x, vertex.color.y, vertex.color.z });
	}

	// vertex_indices vector
	json vertex_indices_json = json::array();
	for (const auto& index : component->vertex_indices) {
		vertex_indices_json.push_back(index);
	}

	// create json object for meshPtrs
	j = json{
		{"original_size", {component->original_size.x, component->original_size.y}},
		{"vertices", vertices_json},
		{"vertex_indices", vertex_indices_json}
	};
}

inline void fromJSON(const json& j, Mesh* component) {
	component->original_size = vec2(j.at("original_size")[0], j.at("original_size")[1]);

	// Deserialize the vertices vector of ColoredVertex
	component->vertices.clear();
	for (const auto& vertex_json : j.at("vertices")) {
		ColoredVertex vertex;
		vertex.position = vec3(vertex_json[0], vertex_json[1], vertex_json[2]);
		vertex.color = vec3(vertex_json[3], vertex_json[4], vertex_json[5]);
		component->vertices.push_back(vertex);
	}

	// Deserialize the vertex_indices vector of uint16_t
	component->vertex_indices.clear();
	for (const auto& index : j.at("vertex_indices")) {
		component->vertex_indices.push_back(index);
	}
}


// TNT
inline void toJSON(json& j, const TNT& component) {
	j = json{
		{"hp", component.hp},
		{"hasExploded", component.hasExploded}
	};
}

inline void fromJSON(const json& j, TNT& component) {
	j.at("hp").get_to(component.hp);
	j.at("hasExploded").get_to(component.hasExploded);
}

// Explosion
inline void toJSON(json& j, const Explosion& component) {
	// entitities* vector
	json entities_json = json::array();
	for (const Entity* e : component.entitiesHit) {
		if (e != nullptr) {
			entities_json.push_back(static_cast<unsigned int>(*e));
		}
	}


	j = json{
		{"lifetime", component.lifetime},
		{"knockbackStrength", component.knockbackStrength},
		{"damage", component.damage},
		{"entitiesHit", entities_json}
	};
}

inline void fromJSON(const json& j, Explosion& component) {
	j.at("lifetime").get_to(component.lifetime);
	j.at("knockbackStrength").get_to(component.knockbackStrength);
	j.at("damage").get_to(component.damage);

	// Deserialize the entitiesHit vector
	component.entitiesHit.clear();
	for (const unsigned int& e_id : j.at("entitiesHit")) {
//		Entity* entity = nullptr;
        Entity entity;
		if (map_saved_entity_new_entity.count(e_id) > 0) {
			entity = map_saved_entity_new_entity.at(e_id);
		}
		else {
			entity = Entity();
			map_saved_entity_new_entity[e_id] = entity;
		}
		component.entitiesHit.push_back(&entity);
	}
}

// Particle
inline void toJSON(json& j, const Particle& component) {
	j = json{
		{"parent", static_cast<unsigned int>(component.parent)},
		{"dist", {component.dist.x, component.dist.y}},
		{"counter_ms", component.counter_ms},
		{"max_ms", component.max_ms},
	};
}


inline void fromJSON(const json& j, Particle& component) {

	// deserialize parent entity
	unsigned int e_id = j.at("parent");
	Entity entity;
	if (map_saved_entity_new_entity.count(e_id) > 0) {
		entity = map_saved_entity_new_entity.at(e_id);
	}
	else {
		entity = Entity();
		map_saved_entity_new_entity[e_id] = entity;
	}
	component.parent = entity;

	component.dist = vec2(j.at("dist")[0], j.at("dist")[1]);
	j.at("counter_ms").get_to(component.counter_ms);
	j.at("max_ms").get_to(component.max_ms);
}

// Attacking
inline void toJSON(json& j, const Attacking& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, Attacking& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// CanMoveThrough
inline void toJSON(json& j, const CanMoveThrough& component) {
	j = json{
		{"tiling_factor", {component.tiling_factor.x, component.tiling_factor.y}}
	};
}

inline void fromJSON(const json& j, CanMoveThrough& component) {
	component.tiling_factor = vec2(j.at("tiling_factor")[0], j.at("tiling_factor")[1]);
}


// HurtSoundTimer
inline void toJSON(json& j, const HurtSoundTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, HurtSoundTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// BlindTimer
inline void toJSON(json& j, const BlindTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms}
	};
}

inline void fromJSON(const json& j, BlindTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
}

// BleedTimer

inline void toJSON(json& j, const BleedTimer& component) {
	j = json{
		{"counter_ms", component.counter_ms},
		{"blood_ms", component.blood_ms },
		{"dmg_ms", component.dmg_ms}
	};
}

inline void fromJSON(const json& j, BleedTimer& component) {
	j.at("counter_ms").get_to(component.counter_ms);
	j.at("blood_ms").get_to(component.blood_ms);
	j.at("dmg_ms").get_to(component.dmg_ms);
}

// World Effect
inline void toJSON(json& j, const WorldEffect& component) {
    j = json{
            {"worldEffect", component.worldEffect}
    };
}

inline void fromJSON(const json& j, WorldEffect& component) {
    j.at("worldEffect").get_to(component.worldEffect);
}

// Effect Timer

inline void toJSON(json& j, const EffectTimer& component) {
    j = json{
            {"counter_ms", component.counter_ms}
    };
}

inline void fromJSON(const json& j, EffectTimer& component) {
    j.at("counter_ms").get_to(component.counter_ms);
}

// PuddleTimer
inline void toJSON(json& j, const PuddleTimer& component) {
    j = json{
            {"counter_ms", component.counter_ms},
            {"puddle_ms", component.puddle_ms }
    };
}

inline void fromJSON(const json& j, PuddleTimer& component) {
    j.at("counter_ms").get_to(component.counter_ms);
    j.at("puddle_ms").get_to(component.puddle_ms);
}

inline void toJSON(json& j, const PrescribedMotion& component) {
	j = json{
		{"start", {component.start.x, component.start.y}},
		{"apex", {component.apex.x, component.apex.y}},
		{"end", {component.end.x, component.end.y}},
		{"time_ms", component.time_ms}
	};
}

inline void fromJSON(const json& j, PrescribedMotion& component) {
	component.start = vec2(j.at("start")[0], j.at("start")[1]);
	component.apex = vec2(j.at("apex")[0], j.at("apex")[1]);
	component.end = vec2(j.at("end")[0], j.at("end")[1]);
	component.time_ms = j.at("time_ms");
}

// TutorialState
inline void toJSON(json& j, const TutorialText& component) {
    j = json{
            {"state", (int) component.state}
    };
}

inline void fromJSON(const json& j, TutorialText& component) {
    j.at("state").get_to(component.state);
}

// TitleScreenButton
inline void toJSON(json& j, const TitleScreenButton& component) {
	j = json{
		{"do_render", component.do_render},
		{"scale_x", component.scale_x},
		{"scale_y", component.scale_y},
		{"button", component.button}
	};
}

inline void fromJSON(const json& j, TitleScreenButton& component) {
	j.at("do_render").get_to(component.do_render);
	j.at("scale_x").get_to(component.scale_x);
	j.at("scale_y").get_to(component.scale_y);
	j.at("button").get_to(component.button);
}

// TitleScreen
inline void toJSON(json& j, const TitleScreen& component) {
	j = json{
		{"position", {component.position.x, component.position.y}},
		{"scale", {component.scale.x, component.scale.y}},
		{"tiling_factor", {component.tiling_factor.x, component.tiling_factor.y}},
		{"doRender", {component.doRender}}
	};
}

inline void fromJSON(const json& j, TitleScreen& component) {
	component.position = vec2(j.at("position")[0], j.at("position")[1]);
	component.scale = vec2(j.at("scale")[0], j.at("scale")[1]);
	component.tiling_factor = vec2(j.at("tiling_factor")[0], j.at("tiling_factor")[1]);
	j.at("doRender").get_to(component.doRender);
}

// LevelState

inline void toJSON(json& j, const LevelState& component) {
	j = json{
		{"whichLevel", component.whichLevel}
	};
}

inline void fromJSON(const json& j, LevelState& component) {
	component.whichLevel = (int)j.at("whichLevel");
}
