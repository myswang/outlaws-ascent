#pragma once
#ifndef _INCL_GUARD
#define _INCL_GUARD

// standard libs
#include <string>
#include <tuple>
#include <vector>
#include <random>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
#include <glm/trigonometric.hpp>    // trig functions
using namespace glm;

#include "entity.hpp"

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE


// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};
inline std::string save_file_path() { return std::string(PROJECT_SOURCE_DIR) + "data/save.json"; };

const int window_width_px = 1280;
const int window_height_px = 720;

// if this is set to true, all entities will be rendered
// regardless of whether they are visible to the player.
// this may be desirable for things like zooming out for
// map testing, for example
//
// default value is false, to improve rendering performance
const bool OFFSCREEN_RENDERING = false;

const int SPRITE_SIZE_STANDARD = 90;
const int BULLET_SIZE = 30;

const float BAR_LENGTH_STANDARD = 500;
const float BAR_HEIGHT_STANDARD = 20;

const float HELP_POPUP_WIDTH = 1000;
const float HELP_POPUP_HEIGHT = 600;

const float BUTTON_WIDTH = 240;
const float BUTTON_HEIGHT = 60;

const float BAR_LENGTH_ENEMY = 90;
const float BAR_HEIGHT_ENEMY = 10;

const float SPRITE_SIZE_AMMO_COUNTER = 24;

const float BASIC_ENEMY_MAX_HEALTH = 100.0;

const int ENEMY_PATH_COMPUTE_FREQUENCY_MS = 1000;
const float GRID_AROUND_PLAYER_SIZE = 750.0;

const vec2 ENEMY_HEALTHBAR_OFFSET = -vec2(SPRITE_SIZE_STANDARD / 2.0f, (SPRITE_SIZE_STANDARD / 2.0f) + 20.0f);

// GAME CONFIGURATION:
static const float DEFAULT_SPEED = 200.0f;
// limit zoom between 0.125x and 8x
static const float MAX_ZOOM = 8.0f;
static const float MIN_ZOOM = 0.125f;
// weapon position offset from player/enemies
static const float WEAPON_OFFSET = 45.0f;
static const float LEFT_ARM_OFFSET = 18.0f;
static const float RIGHT_ARM_OFFSET = 16.0f;
static const float SHOULDER_OFFSET = 2.0f;
// world boundaries
// NOTE: these should be scaled by the obstacle sprite size
static const float WORLD_MIN_X = -60.0f;
static const float WORLD_MAX_X = 60.0f;
static const float WORLD_MIN_Y = -50.0f;
static const float WORLD_MAX_Y = 50.0f;
static const float NUM_CELLS_X = 8.0f;
static const float NUM_CELLS_Y = 10.0f;

// enum for title screen buttons
enum class TITLE_BUTTON_TYPE {NEW_GAME, RESUME_GAME, OPTIONS};

enum class TutorialState {
    WASD = 0,
    DASH = WASD+1,
    PICK_UP_WEAPON = DASH+1,
    SWITCH_WEAPON = PICK_UP_WEAPON+1,
    SHOOT = SWITCH_WEAPON+1,
    RELOAD = SHOOT+1,
    TNT = RELOAD+1,
    PASSIVE_ITEM = TNT+1,
    CONSUMABLE_ITEM = PASSIVE_ITEM+1,
    WET = CONSUMABLE_ITEM+1,
    DONE = WET+1
};

// enum for weapon IDs - which weapon is the user holding?
enum class WEAPON_TYPE { REVOLVER, SHOTGUN, RIFLE, KNIFE, NONE};
static std::unordered_map<WEAPON_TYPE, unsigned int> WEAPON_AMMO_COUNT = 
{
	{ WEAPON_TYPE::REVOLVER, 6 },
	{ WEAPON_TYPE::SHOTGUN, 2 },
	{ WEAPON_TYPE::RIFLE, 1 },
    { WEAPON_TYPE::KNIFE, 0 },
    { WEAPON_TYPE::NONE, 0 }
};

static std::unordered_map<WEAPON_TYPE, float> WEAPON_RELOAD_TIME =
{
    { WEAPON_TYPE::REVOLVER, 1000.0f},
    { WEAPON_TYPE::SHOTGUN, 1000.0f},
    { WEAPON_TYPE::RIFLE, 2000.0f},
    { WEAPON_TYPE::KNIFE, 0.0f},
    { WEAPON_TYPE::NONE, 0.0f},

};

static std::unordered_map<WEAPON_TYPE, unsigned int> WEAPON_SPEED =
        {
                { WEAPON_TYPE::REVOLVER, 500 },
                { WEAPON_TYPE::SHOTGUN, 500 },
                { WEAPON_TYPE::RIFLE, 750 },
                {WEAPON_TYPE::KNIFE, 0},
                {WEAPON_TYPE::NONE, 0}
        };

static std::unordered_map<WEAPON_TYPE, unsigned int> WEAPON_DAMAGE =
        {
                { WEAPON_TYPE::REVOLVER, 10 },
                { WEAPON_TYPE::SHOTGUN, 5 },
                { WEAPON_TYPE::RIFLE, 30 },
                {WEAPON_TYPE::KNIFE, 10},
                {WEAPON_TYPE::NONE, 0}
        };


// edit these as we add more passive items
static const int P_PADDING = 1;
static const int P_GUNMOD = 2;
static const int P_HEALTHUP = 3;
static const int P_STAMUP = 4;
static const int P_DMGUP = 5;


// enemy types

static const int E_STANDARD = 0;
static const int E_FIRE = 1;

static std::unordered_map<int, std::string> ITEM_NAME_MAP =
{
	{P_PADDING, "Protective Padding"},
	{P_GUNMOD, "Gun Mod Tool"},
	{P_HEALTHUP, "Miracle Elixir"},
	{P_STAMUP, "Miracle Powder"},
	{P_DMGUP, "Miracle Liquid"},
};

static std::unordered_map<int, std::string> ITEM_EFFECT_MAP =
{
	{P_PADDING, "Take less damage"},
	{P_GUNMOD, "Faster fire rate"},
	{P_HEALTHUP, "Health Up!"},
	{P_STAMUP, "Stamina Up!"},
	{P_DMGUP, "Damage Up!"}
};

enum class Consumable {
    CHICKEN = 0,
    LANDMINE = 1,
    HEARTY_STEW = 2,
    STAMINA_POTION = 3
};


enum class EnemyType { DEFAULT, SNIPER, KNIFE, FIRE, BLINDER};
enum class Effects {
    NONE = 0,
    MUD = 1,
    OASIS = 2
};

static std::unordered_map<EnemyType, float> SPEED_MAP = {
        { EnemyType::DEFAULT, 50},
        { EnemyType::SNIPER, 50 },
        { EnemyType::KNIFE,  150},
        { EnemyType::FIRE, 50 }
};

static std::unordered_map<EnemyType, float> ENEMY_ATTACK_COOLDOWN =
        {
                { EnemyType::DEFAULT, 1500},
                { EnemyType::SNIPER, 4000 },
                { EnemyType::KNIFE,  0}, 
                { EnemyType::FIRE, 5000 },
                { EnemyType::BLINDER, 5000}
        };

static std::unordered_map<int, std::string> CONSUMABLE_NAME_MAP =
        {
                {(int) Consumable::CHICKEN, "Chicken Leg"},
                {(int) Consumable::LANDMINE, "Landmine"},
                {(int) Consumable::HEARTY_STEW, "Hearty Stew"},
                {(int) Consumable::STAMINA_POTION, "Outlaw's Brew"}

        };

static std::unordered_map<int, std::string> CONSUMABLE_EFFECT_MAP =
        {
                {(int) Consumable::CHICKEN, "Heal 10 HP"},
                {(int) Consumable::LANDMINE, "Place on the ground to blow up enemies!"},
                {(int) Consumable::HEARTY_STEW, "Heal to max HP!"},
                {(int) Consumable::STAMINA_POTION, "Stamina boost to the max!"}

        };


// Max number of weapons the player can equip
const float MAX_WEAPONS = 4;

// Dash config
const float DASH_SPEED = DEFAULT_SPEED * 4;
const float DASH_TIME_MS = 500.f;

// for loading json
// when loading entities, maps saved entity ids to newly created entities
extern std::unordered_map<unsigned int, Entity> map_saved_entity_new_entity;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recomment making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

struct Character {
	unsigned int TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	unsigned int Advance;
	char character;
};


bool gl_has_errors();

#endif