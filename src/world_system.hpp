#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
extern bool isHealthy;
extern bool isBlinded;
extern bool isWet;

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE

class WorldSystem
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Check for collisions
	void handle_collisions(float elapsed_ms);

	// Should the game be over ?
	bool is_over()const;
	bool game_paused;
	bool initialized = 0;

	bool game_in_title_screen;
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_click(int button, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_scroll_wheel(vec2 offset);

	// reload 
	void reloadPlayerAmmo();

	// restart level
	void restart_game(bool carry_over_passives);
	void title_screen();
	
	// load game
	void load_game();

	void create_world_one(float cell_x, float cell_y, int middle_x, int middle_y, std::vector<Entity>& obstacles, std::vector<Entity>& enemies);
	void create_world_two(float cell_x, float cell_y, int middle_x, int middle_y, std::vector<Entity>& obstacles, std::vector<Entity>& enemies);
	void create_world_three(float cell_x, float cell_y, int middle_x, int middle_y, std::vector<Entity>& obstacles, std::vector<Entity>& enemies);
	void create_tutorial(float cell_x, float cell_y, int middle_x, int middle_y, std::vector<Entity>& obstacles, std::vector<Entity>& enemies);
	void initPassiveItem(int id);

	// OpenGL window handle
	GLFWwindow* window;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;
	bool currLevelComplete;
	bool inCombat;

	// Game state
	RenderSystem* renderer;
	float current_speed;
	Entity player;
	// obstacle list
	std::vector<Entity> obstacles;
	// enemy list
	std::vector<Entity> enemies;
	// weapon list
	std::vector<Entity> weapons;
	// passive item list
	std::vector<Entity> passiveItems;
    // consumable item list
    std::vector<Entity> consumableItems;


    bool fps_visible;
	bool help_popup_visible;
	bool inventory_visible;
	int help_popup_page;
	int elapsed_frames_since_last_second;
	float total_elapsed_ms;
	int fps;
	int inventory_index = 0;

	float damage_multiplier = 1.0f;
	float attack_multiplier = 1.0f;
	float fire_delay = 500.0f;

	float PLAYER_MAX_HEALTH = 100.0f;
	float PLAYER_MAX_STAMINA = 100.0f;

	// music references
	Mix_Music* background_music;
	Mix_Music* combat_music;
	Mix_Chunk* dead_sound;
	Mix_Chunk* pistol_shot_sound;
	Mix_Chunk* reload_begin_sound;
	Mix_Chunk* reload_sound;
	Mix_Chunk* hurt_sound;
	Mix_Chunk* heartbeat;
	Mix_Chunk* dash_sound;
	Mix_Chunk* weapon_switch;
	Mix_Chunk* weapon_pick;
	Mix_Chunk* bullet_hit;
	Mix_Chunk* enemy_dead;
	Mix_Chunk* error_sound;
    Mix_Chunk* explosion_sound;
	Mix_Chunk* tnt_explosion;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
	std::uniform_int_distribution<> consumable_dist;
	std::uniform_int_distribution<> consumable_drop_dist;

	void updateMaxAmmo();

	void updateAmmoUI();

	void handle_fire(vec2 bullet_position, float angle);

    bool isMovementKey(int key);

    void handle_tutorial_state(bool keep_curr);

    void handle_movement_key_release(int key, vec2 &velocity, vec2 &max_velocity);

    void handle_movement_key_press(int key, vec2 &velocity, vec2 &max_velocity);

	void step_animation(Entity &entity, float elapsed_ms_since_last_update);
  
	void handle_change_weapon(bool is_next);

    void handle_consumable(Entity &consumable);

	bool HasBeenHitByExplosion(Explosion& explosion, Entity& e);


//    enum class TutorialState {
//        WASD = 0,
//        PICK_UP_WEAPON = 1,
//        SWITCH_WEAPON = 2,
//        SHOOT = 3,
//        RELOAD = 4,
//        TNT = 5,
//        PASSIVE_ITEM = 6,
//        CONSUMABLE_ITEM = 7,
//        WET = 8,
//        DONE = 9
//    };
//
//    std::unordered_map<int, std::string> TUTORIAL_TEXT_MAP =
//            {
//                    {(int) TutorialState::WASD, "Use W/A/S/D to move."},
//                    {(int) TutorialState::PICK_UP_WEAPON, "Walk towards weapons to touch them and pick them up!"},
//                    {(int) TutorialState::SWITCH_WEAPON, "Switch weapon with Q/E keys or scroll wheel."},
//                    {(int) TutorialState::SHOOT, "Left click to shoot!"},
//                    {(int) TutorialState::RELOAD, "Reload by pressing R or right click."},
//                    {(int) TutorialState::TNT, "Shoot at TNT to blow them up!"},
//                    {(int) TutorialState::PASSIVE_ITEM, "You picked up a passive item, check inventory by pressing ESC"},
//                    {(int) TutorialState::CONSUMABLE_ITEM, "You picked up a consumable item, use it with 1/2/3/4 keys."},
//                    {(int) TutorialState::WET, "Wet status provides fire immunity!"},
//                    {(int) TutorialState::DONE, ""}
//            };

    std::unordered_map<int, std::string> TUTORIAL_TEXT_MAP =
            {
                    {(int) TutorialState::WASD, "Use W/A/S/D to move."},
                    {(int) TutorialState::DASH, "Press SHIFT with movement keys to dash"},
                    {(int) TutorialState::PICK_UP_WEAPON, "Walk towards weapons to touch them and pick them up!"},
                    {(int) TutorialState::SWITCH_WEAPON, "Switch weapon with Q/E keys or scroll wheel."},
                    {(int) TutorialState::SHOOT, "Left click to shoot!"},
                    {(int) TutorialState::RELOAD, "Reload by pressing R or right click."},
                    {(int) TutorialState::TNT, "Shoot at TNT to blow them up!"},
                    {(int) TutorialState::PASSIVE_ITEM, "You picked up a passive item, check inventory by pressing ESC"},
                    {(int) TutorialState::CONSUMABLE_ITEM, "You picked up a consumable item, use it with 1/2/3/4 keys."},
                    {(int) TutorialState::WET, "Wet status provides fire immunity!"},
                    {(int) TutorialState::DONE, ""}
            };

    TutorialState curr_state = TutorialState::WASD;

};
