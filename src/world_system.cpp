// Header
#include "world_system.hpp"
#include "common.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE
bool isBasic = true;
bool isSecond = false;
bool isThird = false;
bool isHealthy = true;
bool isBlinded = false;
bool isWet = false;
bool isBleeding = false;

bool isPlaying = false;
int channel = -1;

float blood_splotch_frequency_ms = 500.0f;
float bleed_frequency_ms = 1000.0f;

float enemy_health_multiplier = 1.0f;
float enemy_damage_multiplier = 1.0f;

int inventory_items_per_row = 10;

float INVENTORY_TEXT_YPOS = (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 50.0f;
float INVENTORY_TEXT_XPOS = (-window_width_px / 2) + ((window_width_px - HELP_POPUP_WIDTH) / 2) + 50.0f;

float CURR_INVENTORY_XPOS = INVENTORY_TEXT_XPOS + 50.0f;
float CURR_INVENTORY_YPOS = (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 150.0f;


float item_xpos_start = INVENTORY_TEXT_XPOS + 50.0f;
float item_xpos_end = -(INVENTORY_TEXT_XPOS + 50.0f);
float item_ypos_start = (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 150.0f;
float item_ypos_end = -((-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 150.0f);

int items_per_row = (item_xpos_end - item_xpos_start) / SPRITE_SIZE_STANDARD;

int help_popup_num_pages = 3;

float ENEMY_MAX_HEALTH = BASIC_ENEMY_MAX_HEALTH;

extern int enemy_count;

std::vector<int> currentPassiveList;

// create the underwater world
WorldSystem::WorldSystem()
        : points(0),
          fps_visible(false), 
          elapsed_frames_since_last_second(0),
          total_elapsed_ms(0.0),
          fps(0),
          help_popup_visible(false),
          help_popup_page(0), 
          inventory_visible(false),
          game_paused(false),
          game_in_title_screen(false),
          currLevelComplete(false),
          inCombat(false),
          initialized(false) {
    // Seeding rng with random device
    rng = std::default_random_engine(std::random_device()());
    consumable_dist = std::uniform_int_distribution<>(1, CONSUMABLE_NAME_MAP.size());
    consumable_drop_dist = std::uniform_int_distribution<>(1, 2);

}

WorldSystem::~WorldSystem() {

    // destroy music components
    if (background_music != nullptr)
        Mix_FreeMusic(background_music);
    if (combat_music != nullptr)
        Mix_FreeMusic(combat_music);
    if (dead_sound != nullptr)
        Mix_FreeChunk(dead_sound);
    if (pistol_shot_sound != nullptr)
        Mix_FreeChunk(pistol_shot_sound);
    if (reload_begin_sound != nullptr)
        Mix_FreeChunk(reload_begin_sound);
    if (reload_sound != nullptr)
        Mix_FreeChunk(reload_sound);
	if (hurt_sound != nullptr)
		Mix_FreeChunk(hurt_sound);
	if (heartbeat != nullptr)
		Mix_FreeChunk(heartbeat);
	if (dash_sound != nullptr)
		Mix_FreeChunk(dash_sound);
	if (weapon_switch != nullptr)
		Mix_FreeChunk(weapon_switch);
	if (weapon_pick != nullptr)
		Mix_FreeChunk(weapon_pick);
	if (bullet_hit != nullptr)
		Mix_FreeChunk(bullet_hit);
	if (enemy_dead != nullptr)
		Mix_FreeChunk(enemy_dead);
    if (error_sound != nullptr)
        Mix_FreeChunk(error_sound);
	if (explosion_sound != nullptr)
		Mix_FreeChunk(explosion_sound);
	if (tnt_explosion != nullptr)
		Mix_FreeChunk(tnt_explosion);
    Mix_CloseAudio();

    // Destroy all created components
    registry.clear_all_components();

    // Close the window
    glfwDestroyWindow(window);
}

// Debugging
namespace {
    void glfw_err_cb(int error, const char *desc) {
        fprintf(stderr, "%d: %s", error, desc);
    }
}

void window_focus_redirect(GLFWwindow* window, int focused) {
    WorldSystem* this_world_system = (WorldSystem*)glfwGetWindowUserPointer(window);

    if (focused) {
        this_world_system->game_paused = false;
    }
    else {
        this_world_system->game_paused = true;
    }
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow *WorldSystem::create_window() {
    ///////////////////////////////////////
    // Initialize GLFW
    glfwSetErrorCallback(glfw_err_cb);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW");
        return nullptr;
    }

    //-------------------------------------------------------------------------
    // If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
    // enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
    // GLFW / OGL Initialization
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, 0);

    // Create the main window (for rendering, keyboard, and mouse input)
    window = glfwCreateWindow(window_width_px, window_height_px, "Team 14: Outlaw's Ascent", nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr, "Failed to glfwCreateWindow");
        return nullptr;
    }

    // Setting callbacks to member functions (that's why the redirect is needed)
    // Input is handled using GLFW, for more info see
    // http://www.glfw.org/docs/latest/input_guide.html
    glfwSetWindowUserPointer(window, this);
    auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3) {
        ((WorldSystem *) glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3);
    };
    auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1) {
        ((WorldSystem *) glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1});
    };
    auto mouse_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) {
        ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_click(_0, _1, _2 );
    };
    auto mouse_scroll_redirect = [](GLFWwindow* wnd, double _0, double _1) {
        ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_scroll_wheel({_0, _1});
    };
    glfwSetKeyCallback(window, key_redirect);
    glfwSetCursorPosCallback(window, cursor_pos_redirect);
    glfwSetMouseButtonCallback(window, mouse_button_redirect);
    glfwSetScrollCallback(window, mouse_scroll_redirect);
    glfwSetWindowFocusCallback(window, window_focus_redirect);

    //////////////////////////////////////
    // Loading music and sounds with SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Failed to initialize SDL Audio");
        return nullptr;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        fprintf(stderr, "Failed to open audio device");
        return nullptr;
    }

    background_music = Mix_LoadMUS(audio_path("peaceful_music.wav").c_str());
    combat_music = Mix_LoadMUS(audio_path("music.wav").c_str());
    dead_sound = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
    pistol_shot_sound = Mix_LoadWAV(audio_path("pistol.wav").c_str());
    reload_begin_sound = Mix_LoadWAV(audio_path("reload_begin.wav").c_str());
    reload_sound = Mix_LoadWAV(audio_path("reload.wav").c_str());
	hurt_sound = Mix_LoadWAV(audio_path("hurt.wav").c_str());
    heartbeat = Mix_LoadWAV(audio_path("heartbeat.wav").c_str());
	dash_sound = Mix_LoadWAV(audio_path("dash.wav").c_str());
	weapon_switch = Mix_LoadWAV(audio_path("switch.wav").c_str());
	weapon_pick = Mix_LoadWAV(audio_path("weapon_pick.wav").c_str());
	bullet_hit = Mix_LoadWAV(audio_path("bullet_hit.wav").c_str());
    enemy_dead = Mix_LoadWAV(audio_path("enemy_fall.wav").c_str());
    error_sound = Mix_LoadWAV(audio_path("error.wav").c_str());
      explosion_sound = Mix_LoadWAV(audio_path("explosion.wav").c_str());
	tnt_explosion = Mix_LoadWAV(audio_path("tnt_explosion.wav").c_str());


    if (background_music == nullptr || combat_music == nullptr || dead_sound == nullptr || pistol_shot_sound == nullptr
        || reload_sound == nullptr || hurt_sound == nullptr || heartbeat == nullptr || dash_sound == nullptr
        || weapon_switch == nullptr || weapon_pick == nullptr || bullet_hit == nullptr || enemy_dead == nullptr
        || explosion_sound == nullptr || tnt_explosion == nullptr) {
        fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
            audio_path("peaceful_music.wav").c_str(),
            audio_path("music.wav").c_str(),
            audio_path("death_sound.wav").c_str(),
            audio_path("pistol.wav").c_str(),
            audio_path("reload.wav").c_str(),
            audio_path("hurt.wav").c_str(),
			audio_path("heartbeat.wav").c_str(),
			audio_path("dash.wav").c_str(),
			audio_path("switch.wav").c_str(),
			audio_path("weapon_pick.wav").c_str(),
			audio_path("bullet_hit.wav").c_str(),
			audio_path("enemy_fall.wav").c_str(),
            audio_path("explosion.wav").c_str(),
			audio_path("tnt_explosion.wav").c_str());
        return nullptr;
    }

    return window;
}

void WorldSystem::init(RenderSystem *renderer_arg) {
    this->renderer = renderer_arg;
    // Playing background music indefinitely
    Mix_PlayMusic(background_music, -1);
    fprintf(stderr, "Loaded music\n");

    // Set all states to default

    title_screen();
}

vec2 grid_align(vec2 position) {
    int rounded_x = std::floor((position.x / (float)SPRITE_SIZE_STANDARD));
    int rounded_y = std::floor((position.y / (float)SPRITE_SIZE_STANDARD));

    return vec2((rounded_x * SPRITE_SIZE_STANDARD) + (SPRITE_SIZE_STANDARD / 2), (rounded_y * SPRITE_SIZE_STANDARD) + SPRITE_SIZE_STANDARD / 2);
}

void editHealthBar(float health_val, float PLAYER_MAX_HEALTH) {
    if (registry.healthbars.entities.size() > 0) {
        for (Entity healthbar : registry.healthbars.entities) {
            if (registry.motions.has(healthbar)) {
                Motion& hbMotion = registry.motions.get(healthbar);
                hbMotion.scale.x = BAR_LENGTH_STANDARD - (BAR_LENGTH_STANDARD * (1 - (health_val / PLAYER_MAX_HEALTH)));
                if (hbMotion.scale.x < 0) {
                    hbMotion.scale.x = 0;
                }
            }
        }
    }
}

void editStaminaBar(float stam_val, float PLAYER_MAX_STAMINA) {
    if (registry.staminabars.entities.size() > 0) {
        for (Entity stmbar : registry.staminabars.entities) {
            if (registry.motions.has(stmbar)) {
                Motion& hbMotion = registry.motions.get(stmbar);
                hbMotion.scale.x = BAR_LENGTH_STANDARD - (BAR_LENGTH_STANDARD * (1 - (stam_val / PLAYER_MAX_STAMINA)));
                if (hbMotion.scale.x < 0) {
                    hbMotion.scale.x = 0;
                }
            }
        }
    }
}

// immediately add and apply one passive item to the player
void WorldSystem::initPassiveItem(int id) {

    if (!registry.hasPassiveItems.has(player)) {
        registry.hasPassiveItems.emplace(player);
    }

    Motion collider_motion = registry.motions.get(player);

    HasPassiveItem& hpi = registry.hasPassiveItems.get(player);
    hpi.items_list.push_back(id);
    std::string item_string = ITEM_NAME_MAP[id] + ": " + ITEM_EFFECT_MAP[id];

    // create inventory text and image at appropriate position
    Entity test_inventory_text = createText(item_string, vec2(INVENTORY_TEXT_XPOS, INVENTORY_TEXT_YPOS), vec3(0.0f, 0.0f, 0.0f), true);
    InventoryText& it = registry.inventoryTexts.emplace(test_inventory_text);
    it.index = hpi.items_list.size() - 1;
    it.is_title = false;

    TEXTURE_ASSET_ID thisTexture;

    if (id == P_HEALTHUP) {
        thisTexture = TEXTURE_ASSET_ID::ELIXIR;
    }
    else if (id == P_STAMUP) {
        thisTexture = TEXTURE_ASSET_ID::POWDER;
    }
    else if (id == P_DMGUP) {
        thisTexture = TEXTURE_ASSET_ID::SYRINGE;
    }
    else if (id == P_GUNMOD) {
        thisTexture = TEXTURE_ASSET_ID::GUNMOD;
    }
    else if (id == P_PADDING) {
        thisTexture = TEXTURE_ASSET_ID::PADDING;
    }

    Entity inventory_item = createInventoryImage(vec2(CURR_INVENTORY_XPOS, CURR_INVENTORY_YPOS), thisTexture);
    if (inventory_visible) {
        InventoryImage& ii = registry.inventoryImages.get(inventory_item);
        Motion& iim = registry.motions.get(inventory_item);
        iim.scale = vec2(ii.scale_x, ii.scale_y);
    }


    CURR_INVENTORY_XPOS += SPRITE_SIZE_STANDARD;
    if (CURR_INVENTORY_XPOS > item_xpos_end) {
        CURR_INVENTORY_XPOS = item_xpos_start;
        CURR_INVENTORY_YPOS += SPRITE_SIZE_STANDARD;
    }

    // apply 'immediate' passive item effects

    if (id == P_HEALTHUP) {
        PLAYER_MAX_HEALTH += 30;
        if (registry.healths.has(player)) {
            Health& h = registry.healths.get(player);
            h.val += 30;
            editHealthBar(h.val, PLAYER_MAX_HEALTH);
        }

    }
    else if (id == P_STAMUP) {
        PLAYER_MAX_STAMINA += 30;
        if (registry.staminas.has(player)) {
            Stamina& h = registry.staminas.get(player);
            h.val += 30;
            editStaminaBar(h.val, PLAYER_MAX_STAMINA);
        }
    }
    else if (id == P_GUNMOD) {
        fire_delay = 250.0f;
    }
    else if (id == P_PADDING) {
        damage_multiplier -= 0.25f;
        if (damage_multiplier < 0.25f) {
            damage_multiplier = 0.25f;
        }
    }
    else if (id == P_DMGUP) {
        attack_multiplier += 0.5f;
        if (attack_multiplier > 4.0f) {
            attack_multiplier = 4.0f;
        }
    }
}


// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

    if (!game_paused) {
        // Updating window title with points
        std::stringstream title_ss;
        title_ss << "Points: " << points;
        glfwSetWindowTitle(window, title_ss.str().c_str());

        // Remove debug info from the last step
        while (registry.debugComponents.entities.size() > 0)
            registry.remove_all_components_of(registry.debugComponents.entities.back());

        auto& tntComponents = registry.explodingTNTs.components;
        auto& tntEntities = registry.explodingTNTs.entities;
        for (int i = 0; i < tntComponents.size(); ++i) {
            if (tntComponents[i].hp > 0 || tntComponents[i].hasExploded) continue;
            if (registry.renderRequests.has(tntEntities[i])) {
                auto& tntRender = registry.renderRequests.get(tntEntities[i]);
                tntRender.used_texture = TEXTURE_ASSET_ID::EXPLODED_TNT;
            }
            if (registry.motions.has(tntEntities[i])) {
                auto& motion = registry.motions.get(tntEntities[i]);
                if (isBasic && curr_state == TutorialState::TNT){
                    handle_tutorial_state(false);
                }
                createExplosion(motion.position);
                createParticleBurst(tntEntities[i], 30, vec2(100, 150), vec2(20, 50), vec4(0.2, 0.2, 0.2, 1.0));
                Mix_PlayChannel(-1, tnt_explosion, 0);
            }
            tntComponents[i].hasExploded = true;
        }

        auto& explosionComponents = registry.explosions.components;
        auto& explosionEntities = registry.explosions.entities;
        std::vector<Entity*> toRemove;
        for (int i = 0; i < explosionComponents.size(); ++i) {
            explosionComponents[i].lifetime -= elapsed_ms_since_last_update;
            if (explosionComponents[i].lifetime <= 0) {
                toRemove.push_back(&explosionEntities[i]);
            }
        }
        for (auto& it : toRemove) {
            registry.remove_all_components_of(*it);
        }

        // detect holding down keys

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            handle_movement_key_press(GLFW_KEY_A, registry.motions.get(player).velocity, registry.motions.get(player).max_velocity);
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            handle_movement_key_press(GLFW_KEY_W, registry.motions.get(player).velocity, registry.motions.get(player).max_velocity);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            handle_movement_key_press(GLFW_KEY_S, registry.motions.get(player).velocity, registry.motions.get(player).max_velocity);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            handle_movement_key_press(GLFW_KEY_D, registry.motions.get(player).velocity, registry.motions.get(player).max_velocity);
        }

        // Processing the salmon state
        assert(registry.screenStates.components.size() <= 1);
        ScreenState& screen = registry.screenStates.components[0];

        float min_counter_ms = 3000.f;
        for (Entity entity : registry.deathTimers.entities) {
            // progress timer
            DeathTimer& counter = registry.deathTimers.get(entity);
            counter.counter_ms -= elapsed_ms_since_last_update;
            if (counter.counter_ms < min_counter_ms) {
                min_counter_ms = counter.counter_ms;
            }

            // restart the game once the death timer expired
            if (counter.counter_ms < 0) {
                // stop dying thing from moving at all

                registry.deathTimers.remove(entity);
                screen.darken_screen_factor = 0;


                // close help popup if open
                help_popup_visible = false;
                for (Entity hpbg : registry.helpPopupBGs.entities) {
                    if (registry.motions.has(hpbg)) {
                        Motion& hpbgm = registry.motions.get(hpbg);
                        hpbgm.scale.x = 0;
                        hpbgm.scale.y = 0;
                    }
                }

                restart_game(false);
                return true;
            }
        }

        // THIS is how long a bullet can exist in s
        float t_bullet_life = 5.f;
        // progress bullet life time counter, how long the bullet has existed
        for (Entity entity : registry.bulletLifeTimes.entities) {
            BulletLifeTime& blt = registry.bulletLifeTimes.get(entity);
            blt.counter_ms += elapsed_ms_since_last_update;

            // despawns bullet if counter is more than 5000 ms CHANGE LATER
            // bombs can only stay on screen for 1500 ms
            if (!registry.prescribedMotions.has(entity)) {
                if (blt.counter_ms > (t_bullet_life * 1000)) {

                    registry.remove_all_components_of(entity);
                }
            }
            else {
                PrescribedMotion& pm = registry.prescribedMotions.get(entity);
                if (blt.counter_ms > pm.time_ms) {
                    // also create a ring of fire where it lands
                    if (registry.fireBombs.has(entity)) {
                        Motion& bomb = registry.motions.get(entity);
                        createFireRing(renderer, bomb.position, 3);
                        Mix_PlayChannel(-1, explosion_sound, 0);
                    }
                    registry.remove_all_components_of(entity);
                }
            }
        }

        for (Entity entity : registry.collisionTimers.entities) {
            // progress timer
            CollisionTimer& counter = registry.collisionTimers.get(entity);

            counter.counter_ms -= elapsed_ms_since_last_update;

            // restart the game once the death timer expired
            if (counter.counter_ms < 0) {
                registry.collisionTimers.remove(entity);
            }
        }

        for (Entity entity : registry.attacking.entities) {
            // progress timer
            Attacking& attacking = registry.attacking.get(entity);

            attacking.counter_ms -= elapsed_ms_since_last_update;

            // restart the game once the death timer expired
            if (attacking.counter_ms < 0) {
                registry.attacking.remove(entity);
            }
        }

        // Manages entities that has a shooting cool down for now, shoots when cooldown is 0
        for (Entity entity : registry.bulletCooldownTimers.entities) {
            BulletCooldownTimer& bcd = registry.bulletCooldownTimers.get(entity);

            Enemy& ed = registry.enemies.get(entity);
            Entity& wh = registry.weaponHolders.get(entity).weapon;

            if (registry.following.has(entity) || registry.fleeing.has(entity) || registry.attacking.has(entity)) {
                bcd.counter_ms -= elapsed_ms_since_last_update;
            }
            else {
                bcd.counter_ms = ENEMY_ATTACK_COOLDOWN[ed.enemyType];
            }

            if (bcd.counter_ms < 0) {
                // shoots a bullet, reset timer
                Motion& weapon_motion = registry.motions.get(wh);
                // randomness
                float aim_offset = uniform_dist(rng) / 3.0f;
                int pos_or_neg = consumable_drop_dist(rng); 

                if (pos_or_neg != 1) {
                    aim_offset = -aim_offset;
                }

                float new_angle = ed.angle + aim_offset;

                if (ed.enemyType != EnemyType::KNIFE) {
                    if (ed.enemyType == EnemyType::FIRE) {
                        createFireBomb(renderer, weapon_motion.position, 300, ed.angle);
                        bcd.counter_ms = ENEMY_ATTACK_COOLDOWN[ed.enemyType];
                    }
                    else if (ed.enemyType == EnemyType::BLINDER) {
                        createDirtBall(renderer, weapon_motion.position, 500, ed.angle);
                        bcd.counter_ms = ENEMY_ATTACK_COOLDOWN[ed.enemyType];
                    }
                    else if (ed.enemyType == EnemyType::SNIPER) {
                        Entity eBullet = createBullet(renderer, weapon_motion.position, WEAPON_SPEED[ed.weaponType], ed.angle, false, WEAPON_DAMAGE[ed.weaponType]);
                        bcd.counter_ms = ENEMY_ATTACK_COOLDOWN[ed.enemyType];
                        Damaging& d = registry.damaging.get(eBullet);
                        d.val *= enemy_damage_multiplier;
                    }
                    else {
                        // 'normal' enemies have randomness applied to their shots
                        Entity eBullet = createBullet(renderer, weapon_motion.position, WEAPON_SPEED[ed.weaponType], new_angle, false, WEAPON_DAMAGE[ed.weaponType]);
                        bcd.counter_ms = ENEMY_ATTACK_COOLDOWN[ed.enemyType];
                        Damaging& d = registry.damaging.get(eBullet);
                        d.val *= enemy_damage_multiplier;

                    }
                }
            }

        }

        // manage bleeding entities, create a blood splotch every so often

        for (Entity entity : registry.bleedTimers.entities) {
            BleedTimer& bt = registry.bleedTimers.get(entity);
            if (registry.motions.has(entity)) {
                Motion bm = registry.motions.get(entity);
                bt.blood_ms -= elapsed_ms_since_last_update;
                if (bt.blood_ms < 0) {
                    // create blood splotch, reset timer
                    createBloodSplotch(renderer, bm.position);
                    bt.blood_ms = blood_splotch_frequency_ms;
                }
            }

            // take damage and/or die

            bt.dmg_ms -= elapsed_ms_since_last_update;
            if (bt.dmg_ms < 0 && registry.healths.has(entity)) {
                createColoredBurst(entity, vec4(1.0f, 0.0f, 0.0f, 1.0f));
                Health& h = registry.healths.get(entity);
                h.val -= 5.0f;
                editHealthBar(h.val, PLAYER_MAX_HEALTH);

                if (h.val <= 0 && !registry.deathTimers.has(entity)) {

                    if (registry.motions.has(entity)) {
                        Motion& dm = registry.motions.get(entity);
                        dm.velocity = { 0.0f, 0.0f };
                    }

                    Mix_PlayChannel(-1, dead_sound, 0);
                    registry.deathTimers.emplace(entity);
                }
                bt.dmg_ms = bleed_frequency_ms;
            }

            // stop bleeding 
            bt.counter_ms -= elapsed_ms_since_last_update;
            if (bt.counter_ms < 0) {
                registry.bleedTimers.remove(entity);
            }
        }
      
       for (Entity entity : registry.puddleTimers.entities) {
        PuddleTimer& pt = registry.puddleTimers.get(entity);
        if (registry.motions.has(entity)) {
            Motion bm = registry.motions.get(entity);
            pt.puddle_ms -= elapsed_ms_since_last_update;
            if (pt.puddle_ms < 0) {
                // create blood splotch, reset timer
                createPuddle(renderer, bm.position);
                pt.puddle_ms = blood_splotch_frequency_ms;
            }
        }

        // stop puddle
        pt.counter_ms -= elapsed_ms_since_last_update;
        if (pt.counter_ms < 0) {
            registry.puddleTimers.remove(entity);
        }
    }
      
      for (Entity entity: registry.effectTimers.entities) {
        // progress timer
        EffectTimer &counter = registry.effectTimers.get(entity);

        counter.counter_ms -= elapsed_ms_since_last_update;

        if (counter.counter_ms < 0) {
            registry.effectTimers.remove(entity);
            if (registry.players.has(entity)){
                Player &p = registry.players.get(entity);
                p.worldEffect = Effects::NONE;
                isWet = false;
                if (isBasic && curr_state == TutorialState::WET){
                    handle_tutorial_state(false);
                }
            } else if (registry.enemies.has(entity)){
                Enemy &e = registry.enemies.get(entity);
                e.worldEffect = Effects::NONE;
            }
        }
    }

        for (Entity entity : registry.knockbackTimers.entities) {
            if (registry.knockbackTimers.has(entity)) {
                KnockbackTimer& kt = registry.knockbackTimers.get(entity);
                Motion& motion = registry.motions.get(entity);
                kt.counter_ms -= elapsed_ms_since_last_update;

                if (kt.counter_ms < 0) {
                    registry.knockbackTimers.remove(entity);
                    if (registry.motions.has(entity)) {
                        // stop movement after knockback wears off
                        motion.velocity[0] = 0;
                        motion.velocity[1] = 0;
                    }
                }
            }
        }

        for (Entity entity : registry.attackTimers.entities) {
            if (registry.attackTimers.has(entity)) {
                AttackTimer& at = registry.attackTimers.get(entity);
                at.counter_ms -= elapsed_ms_since_last_update;
                if (at.counter_ms < 0) {
                    registry.attackTimers.remove(entity);
                }
            }
        }

        for (Entity entity : registry.hurtSoundTimers.entities) {
            if (registry.hurtSoundTimers.has(entity)) {
                HurtSoundTimer& at = registry.hurtSoundTimers.get(entity);
                at.counter_ms -= elapsed_ms_since_last_update;
                if (at.counter_ms < 0) {
                    registry.hurtSoundTimers.remove(entity);
                }
            }
        }

        for (Entity entity : registry.reloadTimers.entities) {
            if (registry.reloadTimers.has(entity)) {

                for (Entity r : registry.reloadIndicators.entities) {
                    if (registry.motions.has(r)) {
                        Motion& m = registry.motions.get(r);
                        m.scale.x = SPRITE_SIZE_AMMO_COUNTER;
                        m.scale.y = SPRITE_SIZE_AMMO_COUNTER;
                    }
                }

                ReloadTimer& rt = registry.reloadTimers.get(entity);
                rt.counter_ms -= elapsed_ms_since_last_update;
                if (rt.counter_ms < 0) {
                    registry.reloadTimers.remove(entity);

                    if (registry.ammoCounters.entities.size() > 0) {
                        // turn all ammo counters back to normal
                        for (Entity ac : registry.ammoCounters.entities) {
                            if (registry.colors.has(ac)) {
                                vec4& col = registry.colors.get(ac);
                                col = vec4(1.0f, 1.0f, 1.0f, 1.0f);
                            }
                        }
                    }

                    // Turn off reload indicator
                    for (Entity r : registry.reloadIndicators.entities) {
                        if (registry.motions.has(r)) {
                            Motion& m = registry.motions.get(r);
                            m.scale.x = 0;
                            m.scale.y = 0;
                        }
                    }

                    Weapon& weapon_data = registry.weapons.get(registry.weaponHolders.get(player).weapon);
                    weapon_data.cur_ammo = weapon_data.max_ammo;
                    printf("Reloaded! Ammo: %d / %d\n", weapon_data.cur_ammo, weapon_data.max_ammo);

                    Mix_PlayChannel(-1, reload_sound, 0);
                }
            }
        }

        for (Entity entity : registry.invincibleTimers.entities) {
            InvincibleTimer& inv = registry.invincibleTimers.get(entity);
            inv.counter_ms -= elapsed_ms_since_last_update;

            if (inv.counter_ms < 0) {
                vec4& c = registry.colors.get(entity);
                c = vec4(1.0f, 1.0f, 1.0f, 1.0f);
                registry.invincibleTimers.remove(entity);
            }
        }

        for (Entity entity : registry.dashCooldownTimers.entities) {
            DashCooldownTimer& dcd = registry.dashCooldownTimers.get(entity);
            dcd.counter_ms -= elapsed_ms_since_last_update;

            if (dcd.counter_ms < 0) {
                registry.dashCooldownTimers.remove(entity);
                registry.motions.get(player).velocity = vec2(0, 0);
            }
        }

        for (Entity entity : registry.blindTimers.entities) {
            BlindTimer& dcd = registry.blindTimers.get(entity);
            dcd.counter_ms -= elapsed_ms_since_last_update;

            if (dcd.counter_ms < 0) {
                registry.blindTimers.remove(entity);
                isBlinded = false;
            }
        }

        for (Entity entity : registry.passiveTextTimers.entities) {
            PassiveTextTimer& ptt = registry.passiveTextTimers.get(entity);
            ptt.counter_ms -= elapsed_ms_since_last_update;

            if (ptt.counter_ms < 0) {
                registry.remove_all_components_of(entity);
            }
        }

        for (Entity entity : registry.lifeTimers.entities) {
            LifeTimer& ptt = registry.lifeTimers.get(entity);
            ptt.counter_ms -= elapsed_ms_since_last_update;

            if (ptt.counter_ms < 0) {
                registry.remove_all_components_of(entity);
            }
        }

        for (Entity entity : registry.pickedUpItemTimers.entities) {
            PickedUpItemTimer& ptt = registry.pickedUpItemTimers.get(entity);
            ptt.counter_ms -= elapsed_ms_since_last_update;

            if (ptt.counter_ms < 0) {
                registry.pickedUpItemTimers.remove(entity);
            }
        }

        for (Entity entity : registry.animationTimers.entities) {
            AnimationTimer& at = registry.animationTimers.get(entity);
            at.counter_ms -= elapsed_ms_since_last_update;

            if (at.counter_ms < 0) {
                registry.remove_all_components_of(entity);
            }
        }

        // reduce window brightness if the salmon is dying
        screen.darken_screen_factor = 1 - min_counter_ms / 3000;

        // render health and stamina bars.
        // these have to be rendered separately since they don't use the same texture asset id

        for (Entity hb : registry.healthbars.entities) {
            if (!registry.renderRequests.has(hb)) {
                registry.renderRequests.insert(
                    hb, {
                        TEXTURE_ASSET_ID::TEXTURE_COUNT,
                        EFFECT_ASSET_ID::COLOURED,
                        GEOMETRY_BUFFER_ID::BAR
                    }
                );
                RenderGroup& rg = registry.renderGroups.emplace(hb);
                rg.entities.push_back(hb);
                rg.indices.push_back(0);
            }
        }

        for (Entity sb : registry.staminabars.entities) {
            if (!registry.renderRequests.has(sb)) {
                registry.renderRequests.insert(
                    sb, {
                        TEXTURE_ASSET_ID::TEXTURE_COUNT,
                        EFFECT_ASSET_ID::COLOURED,
                        GEOMETRY_BUFFER_ID::BAR
                    }
                );
                RenderGroup& rg = registry.renderGroups.emplace(sb);
                rg.entities.push_back(sb);
                rg.indices.push_back(0);
            }
        }

        for (Entity hp : registry.helpPopupBGs.entities) {
            if (!registry.renderRequests.has(hp)) {
                registry.renderRequests.insert(
                    hp, {
                        TEXTURE_ASSET_ID::TEXTURE_COUNT,
                        EFFECT_ASSET_ID::COLOURED,
                        GEOMETRY_BUFFER_ID::BAR
                    }
                );
                RenderGroup& rg = registry.renderGroups.emplace(hp);
                rg.entities.push_back(hp);
                rg.indices.push_back(0);
            }
        }

        for (Entity ip : registry.inventoryBGs.entities) {
            if (!registry.renderRequests.has(ip)) {
                registry.renderRequests.insert(
                    ip, {
                        TEXTURE_ASSET_ID::TEXTURE_COUNT,
                        EFFECT_ASSET_ID::COLOURED,
                        GEOMETRY_BUFFER_ID::BAR
                    }
                );
                RenderGroup& rg = registry.renderGroups.emplace(ip);
                rg.entities.push_back(ip);
                rg.indices.push_back(0);
            }
        }


        // render ammo counters
        for (Entity ac : registry.ammoCounters.entities) {
            if (!registry.renderRequests.has(ac)) {
                registry.renderRequests.insert(
                    ac, {
                        TEXTURE_ASSET_ID::AMMO_COUNTER,
                        EFFECT_ASSET_ID::TEXTURED,
                        GEOMETRY_BUFFER_ID::SPRITE
                    }
                );
                RenderGroup& rg = registry.renderGroups.emplace(ac);
                rg.entities.push_back(ac);
                rg.indices.push_back(0);
            }
        }

        // render inventory images
        for (Entity ii : registry.inventoryImages.entities) {
            InventoryImage& invi = registry.inventoryImages.get(ii);
            if (!registry.renderRequests.has(ii)) {
                registry.renderRequests.insert(
                    ii, {
                        invi.tex,
                        EFFECT_ASSET_ID::TEXTURED,
                        GEOMETRY_BUFFER_ID::SPRITE
                    }
                );
                RenderGroup& rg = registry.renderGroups.emplace(ii);
                rg.entities.push_back(ii);
                rg.indices.push_back(0);
            }
        }

        // render weapon indicator
        for (Entity wi : registry.weaponIndicators.entities) {
            WeaponIndicator& weap = registry.weaponIndicators.get(wi);
            if (!registry.renderRequests.has(wi)) {
                registry.renderRequests.insert(
                    wi, {
                        weap.tex,
                        EFFECT_ASSET_ID::TEXTURED,
                        GEOMETRY_BUFFER_ID::SPRITE
                    }
                );

                RenderGroup& rg = registry.renderGroups.emplace(wi);
                rg.entities.push_back(wi);
                rg.indices.push_back(0);

            }
        }

        for (Entity ci : registry.consumableIndicators.entities) {
            ConsumableItem& c = registry.consumableItems.get(ci);
            if (!registry.renderRequests.has(ci)) {
                registry.renderRequests.insert(
                    ci, {
                            c.tex,
                            EFFECT_ASSET_ID::TEXTURED,
                            GEOMETRY_BUFFER_ID::SPRITE
                    }
                );

                RenderGroup& rg = registry.renderGroups.emplace(ci);
                rg.entities.push_back(ci);
                rg.indices.push_back(0);
            }
        }

for (Entity& entity : registry.particles.entities) {
    Particle& part_data = registry.particles.get(entity);
    part_data.counter_ms += elapsed_ms_since_last_update;
    if (part_data.counter_ms > part_data.max_ms)
        registry.remove_all_components_of(entity);
}

// Check level progression
if (enemy_count == 0 && !isBasic && !isSecond && !isThird) {
    isSecond = true;
    isThird = false;
    isBasic = false;

    if (registry.levelStates.components.size() > 0) {
        LevelState& ls = registry.levelStates.components[0];
        ls.whichLevel = 2;
    }
    restart_game(true);
}

if (enemy_count == 0 && !isBasic && isSecond && !isThird) {
    isThird = true;
    isSecond = false;
    isBasic = false;

    if (registry.levelStates.components.size() > 0) {
        LevelState& ls = registry.levelStates.components[0];
        ls.whichLevel = 3;
    }
    restart_game(true);
}

// Render reload indicator
for (Entity ri : registry.reloadIndicators.entities) {
    if (!registry.renderRequests.has(ri)) {
        registry.renderRequests.insert(
            ri, {
                TEXTURE_ASSET_ID::R_KEY,
                EFFECT_ASSET_ID::TEXTURED,
                GEOMETRY_BUFFER_ID::SPRITE
            }
        );
        RenderGroup& rg = registry.renderGroups.emplace(ri);
        rg.entities.push_back(ri);
        rg.indices.push_back(0);
    }
    registry.motions.get(ri).angle += 0.1;
}


        for (Entity e : registry.weaponHolders.entities) {
            float angle;
            if (registry.players.has(e)) {
                angle = registry.players.get(e).angle;
            }

            if (registry.enemies.has(e)) {
                angle = registry.enemies.get(e).angle;
            }
            AnimationState& as = registry.animationStates.get(e);
            RenderGroup& rg = registry.renderGroups.get(e);

            // change sprite and rendering order based on faced direction
            if (angle >= radians(-22.5f) && angle < radians(22.5f)) { // east
                as.row = 0;
                rg.indices = { 2, 0, 3, 1 };
            }
            else if (angle >= radians(157.5) || angle <= radians(-157.5)) { // west
                as.row = 0;
                rg.indices = { 1, 0, 3, 2 };
            }
            else if (angle >= radians(22.5f) && angle < radians(67.5f)) { // southeast
                as.row = 1;
                rg.indices = { 0, 2, 3, 1 };
            }
            else if (angle >= radians(112.5f) && angle < radians(157.5f)) { // southwest
                as.row = 1;
                rg.indices = { 0, 1, 3, 2 };
            }
            else if (angle >= radians(67.5f) && angle < radians(112.5f)) { // north
                as.row = 2;
                rg.indices = { 0, 1, 2, 3 };
            }
            else if (angle >= radians(-112.5f) && angle < radians(-67.5f)) { // south
                as.row = 3;
                rg.indices = { 1, 2, 3, 0 };
            }
            else if (angle >= radians(-67.5f) && angle < radians(-22.5f)) { // northeast
                as.row = 4;
                rg.indices = { 1, 3, 2, 0 };
            }
            else { // northwest
                as.row = 4;
                rg.indices = { 2, 3, 1, 0 };
            }

            // adjust angle and sprite of weapon
            Entity weapon = registry.weaponHolders.get(e).weapon;
            Motion& weapon_motion = registry.motions.get(weapon);
            weapon_motion.angle = angle;

            AnimationState& as2 = registry.animationStates.get(weapon);
            if ((angle > radians(45.0f) && angle < radians(135.0f)) ||
                (angle > radians(-135.0f) && angle < radians(-45.0f))) {
                as2.row = 1;
            }
            else {
                as2.row = 0;
            }
        }

        // update particle timers
        for (Entity& entity : registry.particles.entities) {
            Particle& part_data = registry.particles.get(entity);
            part_data.counter_ms += elapsed_ms_since_last_update;
            if (part_data.counter_ms > part_data.max_ms)
                registry.remove_all_components_of(entity);
        }


        elapsed_frames_since_last_second++;
        total_elapsed_ms += elapsed_ms_since_last_update;
        if (round(total_elapsed_ms) >= 1000) {
            // update current FPS and reset counters
            fps = elapsed_frames_since_last_second;
            total_elapsed_ms = 0.0;
            elapsed_frames_since_last_second = 0;
            // if we have a player with less than full stamina who is not in dash state, recover stamina by 10.0f

            if (!registry.knockbackTimers.has(player) && registry.staminas.has(player)) {
                Stamina& s = registry.staminas.get(player);
                s.val += 5.0;
                if (s.val > PLAYER_MAX_STAMINA) {
                    s.val = PLAYER_MAX_STAMINA;
                }

                if (registry.staminabars.entities.size() > 0) {
                    for (Entity stmbar : registry.staminabars.entities) {
                        if (registry.motions.has(stmbar)) {
                            Motion& hbMotion = registry.motions.get(stmbar);
                            hbMotion.scale.x = BAR_LENGTH_STANDARD - (BAR_LENGTH_STANDARD * (1 - ((float)s.val / PLAYER_MAX_STAMINA)));
                            if (hbMotion.scale.x < 0) {
                                hbMotion.scale.x = 0;
                            }
                        }
                    }
                }
            }
        }


        if (fps_visible) {
            for (Entity& e : registry.texts.entities) {
                if (registry.fpsCounters.has(e)) {
                    IsText& it = registry.texts.get(e);
                    it.text = std::string("FPS: " + std::to_string(fps));
                    it.doRender = true;
                }
            }
        }
        else {
            for (Entity& e : registry.texts.entities) {
                if (registry.fpsCounters.has(e)) {
                    IsText& it = registry.texts.get(e);
                    it.text = std::string("FPS: " + std::to_string(fps));
                    it.doRender = false;
                }
            }
        }

        // render help popup texts if we should
        if (help_popup_visible) {


            for (Entity& e : registry.texts.entities) {
                if (registry.helpTexts.has(e)) {
                    HelpText& ht = registry.helpTexts.get(e);
                    IsText& it = registry.texts.get(e);
                    if (ht.page == help_popup_page) {
                        it.doRender = true;
                    }
                    else {
                        it.doRender = false;
                    }
                }
            }
        }
        else {
            for (Entity& e : registry.texts.entities) {
                if (registry.helpTexts.has(e)) {
                    IsText& it = registry.texts.get(e);
                    it.doRender = false;
                }
            }
        }



        for (Entity entity : registry.animationStates.entities) {
            step_animation(entity, elapsed_ms_since_last_update);
        }

        if (registry.healths.get(player).val > 40) {
            isHealthy = true;
            isPlaying = false;
            Mix_HaltChannel(7);
        }
        else {
            isHealthy = false;
            if (isPlaying == false) {
                channel = Mix_PlayChannel(7, heartbeat, -1);
                isPlaying = true;
            }
        }

        // update inCombat and change music based on that
        if (registry.fleeing.entities.size() > 0 || registry.following.entities.size() > 0) {
            // enemies chasing / fleeing from player

            // if not in combat yet, update to true, change to combat theme
            if (!inCombat) {
                inCombat = true;
                //Mix_FadeOutMusic(1);
                Mix_FadeInMusic(combat_music, -1, 2000);
                //Mix_PlayMusic(combat_music, -1);
            }
        }
        else {
            if (inCombat) {
                inCombat = false;
                //Mix_FadeOutMusic(1);
                Mix_FadeInMusic(background_music, -1, 2000);
                //Mix_PlayMusic(background_music, -1);
            }
        }

    }

    if (inventory_visible) {
        for (Entity& e : registry.texts.entities) {
            if (registry.inventoryTexts.has(e)) {
                InventoryText& ht = registry.inventoryTexts.get(e);
                IsText& it = registry.texts.get(e);
                if (ht.is_title || ht.index == inventory_index) {
                    it.doRender = true;
                }
                else {
                    it.doRender = false;
                }
            }
        }
    }
    else {
        for (Entity& e : registry.texts.entities) {
            if (registry.inventoryTexts.has(e)) {
                IsText& it = registry.texts.get(e);
                it.doRender = false;
            }
        }
    }

    
    return true;
}

void WorldSystem::title_screen() {
    // Debugging for memory/component leaks
    registry.list_all_components();
    printf("Initializing to Title Screen\n");
    registry.texts.clear();
    registry.clear_most_components(false);

    // Reset the game speed
    current_speed = 1.f;
    isHealthy = true;
    isBlinded = false;
    isWet = false;
    isPlaying = false;
    inventory_visible = false;
    help_popup_visible = false;
    help_popup_page = 0;

    // clear these
    enemies.clear();
    obstacles.clear();
    weapons.clear();
    passiveItems.clear();
    consumableItems.clear();

    if (channel != -1) {
        Mix_HaltChannel(channel);
        channel = -1;
    }

    // Reset points & things that passive items affect
    points = 0;
    PLAYER_MAX_HEALTH = 100.0f;
    PLAYER_MAX_STAMINA = 100.0f;
    fire_delay = 500.0f;
    damage_multiplier = 1.0f;

    // Remove all entities that we created
    // All that have a motion, we could also iterate over all fish, eels, ... but that would be more cumbersome
    registry.clear_most_components(true);

    // Debugging for memory/component leaks;
    registry.list_all_components();

    // generate the background and scale it accordingly
    Entity background = createBackground(renderer);
    Background& back_data = registry.backgrounds.get(background);
    float tile_x = abs(WORLD_MIN_X) + abs(WORLD_MAX_X) + 1;
    float tile_y = abs(WORLD_MIN_Y) + abs(WORLD_MAX_Y) + 1;
    back_data.scale.x = tile_x * SPRITE_SIZE_STANDARD;
    back_data.scale.y = tile_y * SPRITE_SIZE_STANDARD;
    back_data.tiling_factor.x = tile_x;
    back_data.tiling_factor.y = tile_y;

    Entity title_screen = createTitleScreen(renderer);
    TitleScreen& title_data = registry.titleScreens.get(title_screen);
    title_data.scale.x = 1280;
    title_data.scale.y = 720;
    title_data.doRender = true;

    game_in_title_screen = true;

    vec2 player_pos = { 0, 0 };


    // create a new Player
    player = createPlayer(renderer, player_pos);

    //Entity title_button_new_game = createTitleScreenButton(vec2((-window_width_px / 2) + (window_width_px - BUTTON_WIDTH) / 2,
    //                                            (-window_height_px / 2) + (window_height_px - BUTTON_HEIGHT) / 2), TITLE_BUTTON_TYPE::NEW_GAME);
    Entity title_button_new_game = createTitleScreenButton(vec2((window_width_px/2) - BUTTON_WIDTH - 100,
                                                ((-window_height_px / 2) + (window_height_px - BUTTON_HEIGHT) / 2) - BUTTON_HEIGHT), TITLE_BUTTON_TYPE::NEW_GAME);
    Entity title_button_resume_game = createTitleScreenButton(vec2((window_width_px / 2) - BUTTON_WIDTH - 100,
                                                ((-window_height_px / 2) + (window_height_px - BUTTON_HEIGHT) / 2) + BUTTON_HEIGHT), TITLE_BUTTON_TYPE::RESUME_GAME);

    float button_text_x = (window_width_px / 2) - BUTTON_WIDTH - 100 + 25.f;
    std::vector<float> button_text_ys = { ((-window_height_px / 2) + (window_height_px - BUTTON_HEIGHT) / 2) + BUTTON_HEIGHT + 15.f,
        ((-window_height_px / 2) + (window_height_px - BUTTON_HEIGHT) / 2) - BUTTON_HEIGHT + 15.f};

    std::vector<std::string> button_texts = { "NEW GAME",  "RESUME GAME" };

    for (int i = button_texts.size() - 1; i >= 0; i--) {
        Entity button_text = createText(button_texts[i], vec2(button_text_x, button_text_ys[i]), vec3(1.0f, 1.0f, 1.0f), true);
        TitleScreenButtonText& tsbt = registry.titleScreenButtonTexts.emplace(button_text);
        IsText& it = registry.texts.get(button_text);
        it.doRender = true;
    }

    if (registry.levelStates.entities.size() == 0) {
        Entity level_state = Entity();
        registry.levelStates.emplace(level_state);
    }

}

// Reset the world state to its initial state
void WorldSystem::restart_game(bool carry_over_passives) {

    // if we have passive items, save them 
    if (registry.hasPassiveItems.has(player) && carry_over_passives) {
        HasPassiveItem hpi = registry.hasPassiveItems.get(player);
        currentPassiveList.clear();
        for (int id : hpi.items_list) {
            currentPassiveList.push_back(id);
        }
    }

    initialized = false;
    // Debugging for memory/component leaks
    ScreenState &screenState = registry.screenStates.components[0];

    registry.list_all_components();
    printf("Restarting\n");
    registry.texts.clear();
    registry.clear_most_components(false);

    // Reset the game speed
    current_speed = 1.f;
    isHealthy = true;
    isBlinded = false;
    isWet = false;
    isPlaying = false;
    inventory_visible = false;
    help_popup_visible = false;
    help_popup_page = 0;
  enemy_count = 0;
  currLevelComplete = false;
    curr_state = TutorialState::WASD;

    // Clear these
  enemies.clear();
  obstacles.clear();
  weapons.clear();
  passiveItems.clear();
  consumableItems.clear();

    Entity item_description_text = createText("", vec2(-window_width_px / 2.0f + 20.0f, 150.0f), vec3(0.0f, 0.0f, 0.0f), true);
    screenState.item_description_text = item_description_text;

    CURR_INVENTORY_XPOS = INVENTORY_TEXT_XPOS + 50.0f;
    CURR_INVENTORY_YPOS = (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 150.0f;

    if (channel != -1) {
		Mix_HaltChannel(channel);
		channel = -1;
    }

    // Reset points & things that passive items affect
    points = 0;
    PLAYER_MAX_HEALTH = 100.0f;
    PLAYER_MAX_STAMINA = 100.0f;
    fire_delay = 500.0f;
    damage_multiplier = 1.0f;

    // Remove all entities that we created
    // All that have a motion, we could also iterate over all fish, eels, ... but that would be more cumbersome
    while (registry.motions.entities.size() > 0)
        registry.remove_all_components_of(registry.motions.entities.back());

    // Debugging for memory/component leaks;
    registry.list_all_components();

    // generate the background and scale it accordingly

    Entity background;
    if (isSecond) {
        background = createBackgroundGrass(renderer);
        std::cout << "Creating grass background\n";
	}
	else if (isThird) {
		background = createBackgroundRock(renderer);
        std::cout << "Creating rock background\n";
    }
    else {
        background = createBackground(renderer);
    }
    Background& back_data = registry.backgrounds.get(background);
    float tile_x = abs(WORLD_MIN_X) + abs(WORLD_MAX_X) + 1;
    float tile_y = abs(WORLD_MIN_Y) + abs(WORLD_MAX_Y) + 1;
    back_data.scale.x = tile_x * SPRITE_SIZE_STANDARD;
    back_data.scale.y = tile_y * SPRITE_SIZE_STANDARD;
    back_data.tiling_factor.x = tile_x;
    back_data.tiling_factor.y = tile_y;



    //
    // IMPORTANT: place entities in WORLD coordinates
    // so (0,0) is the centre of the world,
    // NOT the upper left corner of the screen
    // (as would be for screen coordinates)
    //
    vec2 enemy_pos = { 15 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD };
    vec2 player_pos = { -7.5 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD };

    // create potential help popup either way
    float help_popup_text_x = (-window_width_px / 2) + ((window_width_px - HELP_POPUP_WIDTH) / 2) + 50.0f;

    std::vector<float> help_popup_text_ys = {(- window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 550.0f, 
        (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 450.0f,
        (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 400.0f, 
        (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 350.0f, 
        (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 300.0f,  
        (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 250.0f, 
        (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 200.0f,  
        (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 100.0f,
        (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 50.0f };

    std::vector<std::string> help_popup_texts = { "CONTROLS",  "W/A/S/D: Move", "Shift: Dash (uses stamina)", "Mouse: Aim",  "Left click: Shoot", "Right click / R: Reload", "Scroll wheel / Q / E: Switch weapon", "Press H to close", "Press -> to advance"};
    std::vector<std::string> help_popup_texts_1 = { "CONTROLS (page 2)", "Esc: Open / close inventory (pause game)", "1/2/3/4: Use consumable", "F5: Restart game", "F3: Save game", "F4: Load game", "L: toggle between tutorial and level", "Press -> to advance", "Press <- to go back" };
    std::vector<std::string> help_popup_texts_2 = { "INSTRUCTIONS", "Find your way to the end of each level!", "Barrels and fences block your way", "Cacti, enemies, and bullets hurt to touch", "Enemies will chase and shoot at you", "Walk over guns to pick them up", "", "Press H to close", "Press <- to go back" };

    Entity help_popup_bg = createHelpPopupBG(vec2((- window_width_px / 2) + (window_width_px - HELP_POPUP_WIDTH) / 2, (- window_height_px / 2) + (window_height_px - HELP_POPUP_HEIGHT) / 2));

    Entity inventory_bg = createInventoryBG(vec2((-window_width_px / 2) + (window_width_px - HELP_POPUP_WIDTH) / 2, (-window_height_px / 2) + (window_height_px - HELP_POPUP_HEIGHT) / 2));
    Entity inventory_title = createText("INVENTORY", vec2(INVENTORY_TEXT_XPOS, -((-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 50.0f)), vec3(0.0f, 0.0f, 0.0f), true);
    if (!registry.inventoryTexts.has(inventory_title)) {
        InventoryText& it = registry.inventoryTexts.emplace(inventory_title);
        it.index = -1;
        it.is_title = true;
    }
    Entity inventory_border = createInventoryImage(vec2(item_xpos_start, item_ypos_start), TEXTURE_ASSET_ID::BORDER);
    if (!registry.inventoryBorders.has(inventory_border)) {
        registry.inventoryBorders.emplace(inventory_border);
    }


    for (int i = help_popup_texts.size() - 1; i >= 0; i--) {
        Entity help_text = createText(help_popup_texts[i], vec2(help_popup_text_x, help_popup_text_ys[i]), vec3(1.0f, 1.0f, 1.0f), true);
        HelpText& ht = registry.helpTexts.emplace(help_text);
        ht.page = 0;
    }

    for (int i = help_popup_texts_1.size() - 1; i >= 0; i--) {
        Entity help_text = createText(help_popup_texts_1[i], vec2(help_popup_text_x, help_popup_text_ys[i]), vec3(1.0f, 1.0f, 1.0f), true);
        HelpText& ht = registry.helpTexts.emplace(help_text);
        ht.page = 1;
    }

    for(int i = help_popup_texts_2.size() - 1; i >= 0; i--) {
        Entity help_text = createText(help_popup_texts_2[i], vec2(help_popup_text_x, help_popup_text_ys[i]), vec3(1.0f, 1.0f, 1.0f), true);
        HelpText& ht = registry.helpTexts.emplace(help_text);
        ht.page = 2;
    }

    if (isBasic) {
        Entity help_reminder = createText("Welcome to the tutorial! Press H for help", vec2(-100.0f, window_height_px / 2.0 - 40.0f), vec3(0.0f, 0.0f, 0.0f), false);
        IsText& hrit = registry.texts.get(help_reminder);
        hrit.doRender = true;
    }

    // create pause text
    Entity gamePauseText = createText("PAUSED", vec2(-50.0f, -100.0f), vec3(0.0f, 0.0f, 0.0f), true);
    if (!registry.pauseTexts.has(gamePauseText)) {
        registry.pauseTexts.emplace(gamePauseText);
    }


    

    if (isBasic) {
        int min_x = -10;
		    int max_x = 10;
	    	int min_y = -7;
		    int max_y = 7;

        enemy_pos = { (max_x - 2) * SPRITE_SIZE_STANDARD, (max_y-2) * SPRITE_SIZE_STANDARD };
        player_pos = { (min_x+1)*SPRITE_SIZE_STANDARD, (max_y-1)*SPRITE_SIZE_STANDARD };

        Entity enemy_entity = createEnemy(renderer, enemy_pos, EnemyType::BLINDER);
        enemies.push_back(enemy_entity);

        obstacles.push_back(createFenceWall(renderer, {0, min_y*SPRITE_SIZE_STANDARD}, {21, 1}));
        obstacles.push_back(createFenceWall(renderer, {0, max_y*SPRITE_SIZE_STANDARD}, {21, 1}));
        obstacles.push_back(createFenceWall(renderer, {min_x*SPRITE_SIZE_STANDARD, 0}, {1, 13}));
        obstacles.push_back(createFenceWall(renderer, {max_x*SPRITE_SIZE_STANDARD, 0}, {1, 13}));


        // create some cactus obstacles for testing purposes
        obstacles.push_back(createCactus(renderer, { -400, 100 }));

        createTNT({ 100, 100 });
        createTNT({ 130, 100 });
        obstacles.push_back(createBarrel(renderer, { 600, 300 }));

        // create some passive items for testing purposes
        passiveItems.push_back(createPassiveItem(renderer, { -200, -100 }, TEXTURE_ASSET_ID::SYRINGE, P_DMGUP));
        passiveItems.push_back(createPassiveItem(renderer, { -200, -200 }, TEXTURE_ASSET_ID::ELIXIR, P_HEALTHUP));
        passiveItems.push_back(createPassiveItem(renderer, { 0, -100 }, TEXTURE_ASSET_ID::POWDER, P_STAMUP));
        passiveItems.push_back(createPassiveItem(renderer, { 0, -200 }, TEXTURE_ASSET_ID::GUNMOD, P_GUNMOD));
        passiveItems.push_back(createPassiveItem(renderer, { 200, -100 }, TEXTURE_ASSET_ID::PADDING, P_PADDING));

        // create consumable item
        consumableItems.push_back(createConsumableItem(renderer, { -700, -100 }, TEXTURE_ASSET_ID::CHICKEN, Consumable::CHICKEN));
        consumableItems.push_back(createConsumableItem(renderer, { -700, -200 }, TEXTURE_ASSET_ID::LANDMINE, Consumable::LANDMINE));
        consumableItems.push_back(createConsumableItem(renderer, { -500, -100 }, TEXTURE_ASSET_ID::STEW, Consumable::HEARTY_STEW));
        consumableItems.push_back(createConsumableItem(renderer, { -500, -200 }, TEXTURE_ASSET_ID::POTION, Consumable::CHICKEN));

        // testing
        // create a weapon for the pickup
        vec2 weapon_pos3 = { player_pos.x + 400, player_pos.y };
        Entity weapon3 = createWeapon(renderer, weapon_pos3, true, TEXTURE_ASSET_ID::SHOTGUN, WEAPON_TYPE::SHOTGUN);
        weapons.push_back(weapon3);

        // create a weapon for the pickup
        vec2 weapon_pos4 = { player_pos.x + 600, player_pos.y };
        Entity weapon4 = createWeapon(renderer, weapon_pos4, true, TEXTURE_ASSET_ID::RIFLE, WEAPON_TYPE::RIFLE);
        weapons.push_back(weapon4);

        // create a weapon for the pickup
        vec2 weapon_pos5 = { player_pos.x + 800, player_pos.y };
        Entity weapon5 = createWeapon(renderer, weapon_pos5, true, TEXTURE_ASSET_ID::REVOLVER, WEAPON_TYPE::REVOLVER);
        weapons.push_back(weapon5);

        // create a weapon for the pickup
        vec2 weapon_pos6 = { player_pos.x + 1000, player_pos.y };
        Entity weapon6 = createWeapon(renderer, weapon_pos6, true, TEXTURE_ASSET_ID::REVOLVER_METALLIC, WEAPON_TYPE::REVOLVER);
        weapons.push_back(weapon6);

        createWorldEffect(vec2(SPRITE_SIZE_STANDARD * 7, -SPRITE_SIZE_STANDARD * 4), vec2(4, 3), Effects::OASIS);
        createWorldEffect(vec2(SPRITE_SIZE_STANDARD * 7, -SPRITE_SIZE_STANDARD * 1), vec2(4, 3), Effects::MUD);

//        enemiesLeft = enemies.size();
    }
    else {
        if (isSecond) {
            goto second;
        }
        if (isThird) {
			goto third;
        }

        float cell_x = (WORLD_MAX_X - WORLD_MIN_X) / NUM_CELLS_X; // should be 15
        float cell_y = (WORLD_MAX_Y - WORLD_MIN_Y) / NUM_CELLS_Y; // should be 10
        int middle_x = (cell_x) / 2; //should be 7
        int middle_y = (cell_y) / 2; //should be 5

        create_world_one(cell_x, cell_y, middle_x, middle_y, obstacles, enemies);

        enemies.push_back(createEnemy(renderer, { 20 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));
        enemies.push_back(createEnemy(renderer, { 8 * SPRITE_SIZE_STANDARD, -14 * SPRITE_SIZE_STANDARD }));

        enemies.push_back(createEnemy(renderer, { 19 * SPRITE_SIZE_STANDARD, -12 * SPRITE_SIZE_STANDARD }, EnemyType::BLINDER));
        enemies.push_back(createEnemy(renderer, { 23 * SPRITE_SIZE_STANDARD, -15 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 6 * SPRITE_SIZE_STANDARD, -3 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { 22 * SPRITE_SIZE_STANDARD, -7 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));
        enemies.push_back(createEnemy(renderer, { 12 * SPRITE_SIZE_STANDARD, -4 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 27 * SPRITE_SIZE_STANDARD, -18 * SPRITE_SIZE_STANDARD }, EnemyType::BLINDER));
        enemies.push_back(createEnemy(renderer, { 8 * SPRITE_SIZE_STANDARD, -8 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { 23 * SPRITE_SIZE_STANDARD, -1 * SPRITE_SIZE_STANDARD }));

        enemies.push_back(createEnemy(renderer, { 23 * SPRITE_SIZE_STANDARD, 12 * SPRITE_SIZE_STANDARD }, EnemyType::FIRE));
        enemies.push_back(createEnemy(renderer, { 27 * SPRITE_SIZE_STANDARD, 16 * SPRITE_SIZE_STANDARD }));

        enemies.push_back(createEnemy(renderer, { -19 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }, EnemyType::BLINDER));
        enemies.push_back(createEnemy(renderer, { -22 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));

        enemies.push_back(createEnemy(renderer, { -42 * SPRITE_SIZE_STANDARD, -8 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -38 * SPRITE_SIZE_STANDARD, -5 * SPRITE_SIZE_STANDARD }, EnemyType::FIRE));
        enemies.push_back(createEnemy(renderer, { -20 * SPRITE_SIZE_STANDARD, -3 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -18 * SPRITE_SIZE_STANDARD, -8 * SPRITE_SIZE_STANDARD }));

        enemies.push_back(createEnemy(renderer, { -23 * SPRITE_SIZE_STANDARD, 13 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));
        enemies.push_back(createEnemy(renderer, { -21 * SPRITE_SIZE_STANDARD, 27 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -19 * SPRITE_SIZE_STANDARD, 23 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -20 * SPRITE_SIZE_STANDARD, 19 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -17 * SPRITE_SIZE_STANDARD, 17 * SPRITE_SIZE_STANDARD }));

        enemies.push_back(createEnemy(renderer, { -5 * SPRITE_SIZE_STANDARD, 21 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));
        enemies.push_back(createEnemy(renderer, { -7 * SPRITE_SIZE_STANDARD, 22 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -5 * SPRITE_SIZE_STANDARD, 23 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -7 * SPRITE_SIZE_STANDARD, 24 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -5 * SPRITE_SIZE_STANDARD, 25 * SPRITE_SIZE_STANDARD }, EnemyType::BLINDER));
        enemies.push_back(createEnemy(renderer, { -7 * SPRITE_SIZE_STANDARD, 26 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -5 * SPRITE_SIZE_STANDARD, 27 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -7 * SPRITE_SIZE_STANDARD, 28 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -5 * SPRITE_SIZE_STANDARD, 29 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));

        // testing
        // create a weapon for the pickup
        vec2 weapon_pos3 = { -27 * SPRITE_SIZE_STANDARD, -6 * SPRITE_SIZE_STANDARD };
        Entity weapon3 = createWeapon(renderer, weapon_pos3, true, TEXTURE_ASSET_ID::SHOTGUN, WEAPON_TYPE::SHOTGUN);
        weapons.push_back(weapon3);

        // create a weapon for the pickup
        vec2 weapon_pos4 = { 27 * SPRITE_SIZE_STANDARD, -18 * SPRITE_SIZE_STANDARD };
        Entity weapon4 = createWeapon(renderer, weapon_pos4, true, TEXTURE_ASSET_ID::RIFLE, WEAPON_TYPE::RIFLE);
        weapons.push_back(weapon4);

        // create a weapon for the pickup
        vec2 weapon_pos6 = { 26 * SPRITE_SIZE_STANDARD, 16 * SPRITE_SIZE_STANDARD };
        Entity weapon6 = createWeapon(renderer, weapon_pos6, true, TEXTURE_ASSET_ID::REVOLVER_METALLIC, WEAPON_TYPE::REVOLVER);
        weapons.push_back(weapon6);

    }

    second:
    if (isSecond && !isBasic && !isThird) {
        float cell_x = (WORLD_MAX_X - WORLD_MIN_X) / NUM_CELLS_X; // should be 15
        float cell_y = (WORLD_MAX_Y - WORLD_MIN_Y) / NUM_CELLS_Y; // should be 10
        int middle_x = (cell_x) / 2; //should be 7
        int middle_y = (cell_y) / 2; //should be 5

        create_world_two(cell_x, cell_y, middle_x, middle_y, obstacles, enemies);

        enemies.push_back(createEnemy(renderer, { -29 * SPRITE_SIZE_STANDARD, -12 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));
        enemies.push_back(createEnemy(renderer, { -17 * SPRITE_SIZE_STANDARD, -17 * SPRITE_SIZE_STANDARD }, EnemyType::FIRE));
        enemies.push_back(createEnemy(renderer, { -22 * SPRITE_SIZE_STANDARD, -15 * SPRITE_SIZE_STANDARD }, EnemyType::BLINDER));
        enemies.push_back(createEnemy(renderer, { -24 * SPRITE_SIZE_STANDARD, -12 * SPRITE_SIZE_STANDARD }));

        enemies.push_back(createEnemy(renderer, { -25 * SPRITE_SIZE_STANDARD, -7 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));
        enemies.push_back(createEnemy(renderer, { -22 * SPRITE_SIZE_STANDARD, -4 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));

        enemies.push_back(createEnemy(renderer, { -29 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -23 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }, EnemyType::FIRE));
        enemies.push_back(createEnemy(renderer, { -25 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }, EnemyType::BLINDER));
        //enemies.push_back(createEnemy(renderer, { -19 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));

        enemies.push_back(createEnemy(renderer, { -27 * SPRITE_SIZE_STANDARD, 11 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -21 * SPRITE_SIZE_STANDARD, 15 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -18 * SPRITE_SIZE_STANDARD, 18 * SPRITE_SIZE_STANDARD }, EnemyType::FIRE));

        enemies.push_back(createEnemy(renderer, { -8 * SPRITE_SIZE_STANDARD, -13 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -14 * SPRITE_SIZE_STANDARD, -18 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { -4 * SPRITE_SIZE_STANDARD, -12 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -13 * SPRITE_SIZE_STANDARD, -17 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));

        enemies.push_back(createEnemy(renderer, { -10 * SPRITE_SIZE_STANDARD, -9 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -7 * SPRITE_SIZE_STANDARD, -4 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));


        enemies.push_back(createEnemy(renderer, { 1 * SPRITE_SIZE_STANDARD, -9 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { 4 * SPRITE_SIZE_STANDARD, -3 * SPRITE_SIZE_STANDARD }, EnemyType::FIRE));
        enemies.push_back(createEnemy(renderer, { 9 * SPRITE_SIZE_STANDARD, -6 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { 14 * SPRITE_SIZE_STANDARD, -2 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));

        enemies.push_back(createEnemy(renderer, { 25 * SPRITE_SIZE_STANDARD, -18 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));
        enemies.push_back(createEnemy(renderer, { 16 * SPRITE_SIZE_STANDARD, -27 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 11 * SPRITE_SIZE_STANDARD, -19 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { 14 * SPRITE_SIZE_STANDARD, -21 * SPRITE_SIZE_STANDARD }, EnemyType::FIRE));
        enemies.push_back(createEnemy(renderer, { 26 * SPRITE_SIZE_STANDARD, -12 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 22 * SPRITE_SIZE_STANDARD, -25 * SPRITE_SIZE_STANDARD }, EnemyType::FIRE));
        enemies.push_back(createEnemy(renderer, { 20 * SPRITE_SIZE_STANDARD, -17 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { 27 * SPRITE_SIZE_STANDARD, -24 * SPRITE_SIZE_STANDARD }, EnemyType::BLINDER));
        enemies.push_back(createEnemy(renderer, { 19 * SPRITE_SIZE_STANDARD, -16 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { 28 * SPRITE_SIZE_STANDARD, -11 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 25 * SPRITE_SIZE_STANDARD, -29 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 12 * SPRITE_SIZE_STANDARD, -26 * SPRITE_SIZE_STANDARD }));
        enemies.push_back(createEnemy(renderer, { 22 * SPRITE_SIZE_STANDARD, -13 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));

        enemies.push_back(createEnemy(renderer, { 12 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 5 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));
        enemies.push_back(createEnemy(renderer, { 21 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 16 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }, EnemyType::BLINDER));
        enemies.push_back(createEnemy(renderer, { 8 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }, EnemyType::FIRE));
        enemies.push_back(createEnemy(renderer, { 23 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }, EnemyType::SNIPER));
        enemies.push_back(createEnemy(renderer, { 14 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }, EnemyType::FIRE));
        enemies.push_back(createEnemy(renderer, { 19 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }, EnemyType::BLINDER));

            // testing
        // create a weapon for the pickup
        vec2 weapon_pos3 = { -4 * SPRITE_SIZE_STANDARD, -6 * SPRITE_SIZE_STANDARD };
        Entity weapon3 = createWeapon(renderer, weapon_pos3, true, TEXTURE_ASSET_ID::SHOTGUN, WEAPON_TYPE::SHOTGUN);
        weapons.push_back(weapon3);

        // create a weapon for the pickup
        vec2 weapon_pos4 = { 22 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD };
        Entity weapon4 = createWeapon(renderer, weapon_pos4, true, TEXTURE_ASSET_ID::RIFLE, WEAPON_TYPE::RIFLE);
        weapons.push_back(weapon4);

        // create a weapon for the pickup
        vec2 weapon_pos6 = { -27 * SPRITE_SIZE_STANDARD, 18 * SPRITE_SIZE_STANDARD };
        Entity weapon6 = createWeapon(renderer, weapon_pos6, true, TEXTURE_ASSET_ID::REVOLVER_METALLIC, WEAPON_TYPE::REVOLVER);
        weapons.push_back(weapon6);
    }
    third: 
    if (!isSecond && !isBasic && isThird) {
        float cell_x = (WORLD_MAX_X - WORLD_MIN_X) / NUM_CELLS_X; // should be 15
        float cell_y = (WORLD_MAX_Y - WORLD_MIN_Y) / NUM_CELLS_Y; // should be 10
        int middle_x = (cell_x) / 2; //should be 7
        int middle_y = (cell_y) / 2; //should be 5

        create_world_three(cell_x, cell_y, middle_x, middle_y, obstacles, enemies);

        enemies.push_back(createEnemy(renderer, { 19 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 12 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 10 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 6 * SPRITE_SIZE_STANDARD, 9 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 8 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 3 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 14 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 7 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 13 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 9 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 4 * SPRITE_SIZE_STANDARD, 9 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 11 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 2 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 5 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { 13 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -28 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -25 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -22 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -18 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -10 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -17 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -12 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -26 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -19 * SPRITE_SIZE_STANDARD, 9 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -15 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -11 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -20 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -13 * SPRITE_SIZE_STANDARD, 9 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));
        enemies.push_back(createEnemy(renderer, { -16 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }, EnemyType::KNIFE));

        vec2 weapon_pos3 = { -4 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD };
        Entity weapon3 = createWeapon(renderer, weapon_pos3, true, TEXTURE_ASSET_ID::SHOTGUN, WEAPON_TYPE::SHOTGUN);
        weapons.push_back(weapon3);

        // create a weapon for the pickup
        vec2 weapon_pos4 = { -5 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD };
        Entity weapon4 = createWeapon(renderer, weapon_pos4, true, TEXTURE_ASSET_ID::RIFLE, WEAPON_TYPE::RIFLE);
        weapons.push_back(weapon4);

    }

    // axis align all obstacles
    for (Entity obst : obstacles) {
        if (registry.motions.has(obst)) {
            Motion& obst_m = registry.motions.get(obst);
            obst_m.position = grid_align(obst_m.position);
        }
    }

    // this creates a FenceWall that causes the AI to get stuck (because it thinks it only occupies a single square, as opposed to a strip)
    // obstacles.push_back(createFenceWall(renderer, {enemy_pos.x-SPRITE_SIZE_STANDARD*2, enemy_pos.y}, {1, 3}));


    // create an enemy for testing purposes

    Entity enemy = createEnemy(renderer, enemy_pos, EnemyType::FIRE);
    enemies.push_back(enemy);
    //Entity enemy2 = createEnemy(renderer, enemy_pos, EnemyType::KNIFE);
    //enemies.push_back(enemy2);

    // enemy health scaling

    if (isSecond) {
        enemy_health_multiplier = 1.3f;
        enemy_damage_multiplier = 1.3f;
    }
    else if (isThird) {
        enemy_health_multiplier = 1.5f;
        enemy_damage_multiplier = 1.5f;
    }
    else {
        enemy_health_multiplier = 1.0f;
        enemy_damage_multiplier = 1.0f;
    }

    ENEMY_MAX_HEALTH = BASIC_ENEMY_MAX_HEALTH * enemy_health_multiplier;

    for (Entity en : enemies) {
        // apply health / damage (for knife guys) scaling, then create healthbars
        Health& ehealth = registry.healths.get(en);
        ehealth.val = ENEMY_MAX_HEALTH;

        Enemy& enn = registry.enemies.get(en);
        if (enn.enemyType == EnemyType::KNIFE) {
            Damaging& edmg = registry.damaging.get(en);
            edmg.val *= enemy_damage_multiplier;
        }
        
        if (!registry.hasHealthBars.has(en)) {
            HasHealthBar& hhb = registry.hasHealthBars.emplace(en);
            if (registry.motions.has(en)) {
                Motion en_motion = registry.motions.get(en);
                hhb.healthBar = createEnemyHealthBar(en_motion.position - vec2(SPRITE_SIZE_STANDARD / 2.0f, (SPRITE_SIZE_STANDARD / 2.0f) + 20.0f));
            }
        }
    }


    // create a new Player
    player = createPlayer(renderer, player_pos);

    // give our player all saved passive items

    for (int id : currentPassiveList) {
        initPassiveItem(id);
    }
    

    int max_ammo = registry.weapons.get(registry.weaponHolders.get(player).weapon).max_ammo;

    // M4TODO: give player AND all enemies 4 debuff indicators in a loop

    // create the health and stamina bars
    vec2 healthbar_pos = { -window_width_px / 2.0f + 10.0f, -window_height_px / 2.0f + 10.0f };
    vec2 stmbar_pos = { -window_width_px / 2.0f + 10.0f, -window_height_px / 2.0f + 10.0f + BAR_HEIGHT_STANDARD };

    Entity healthbar = createHealthbar(healthbar_pos);
    Entity stmbar = createStaminabar(stmbar_pos);

    // initialize ammo counters, according to the 'default' ammo amount for now, i.e. 6

    registry.ammoCounters.clear();
    float ammo_counter_y_pos = -window_height_px / 2.0f + 10.0f + (BAR_HEIGHT_STANDARD * 2) + 20.0f;
    float ammo_counter_initial_x_pos = -window_width_px / 2.0f + 100.0f;

    for (int i = 0; i < max_ammo; i++) {
        createAmmoCounter(vec2(ammo_counter_initial_x_pos, ammo_counter_y_pos), vec4(1.0f, 1.0f, 1.0f, 1.0f));
        ammo_counter_initial_x_pos += SPRITE_SIZE_AMMO_COUNTER + 5.0f;
    }

    // create reload indicator
    registry.reloadIndicators.clear();
    createReloadIndicator(vec2(-window_width_px / 2.0f + 100.0f + (SPRITE_SIZE_AMMO_COUNTER + 5.0f) * max_ammo, -window_height_px / 2.0f + 10.0f + (BAR_HEIGHT_STANDARD * 2) + 20.0f));

    // create weapon indicator
    registry.weaponIndicators.clear();
    createWeaponIndicator(vec2(-window_width_px / 2.0f + 40.0f, -window_height_px / 2.0f + 10.0f + (BAR_HEIGHT_STANDARD * 2) + 20.0f));

    registry.consumableIndicators.clear();
    for (int i = 0; i < 4; i++){
        vec2 loc = vec2(-window_width_px/2 + 40 + 75*i, -window_height_px/2 + 125);
        Entity border = createBorder(loc);
    }


    // create FPS counter
    Entity fpsText = createText("FPS: 0",
        vec2((window_width_px / 2.0f) - 150.0f, (-window_height_px / 2.0f) + 50.0f), vec3(0.0, 0.0, 0.0), true);
    registry.fpsCounters.emplace(fpsText);

    // initalize enemy counter
    Entity enemyText = createText(std::string("Enemies left: ") + std::to_string(enemy_count), vec2(-((window_width_px / 2.0f) - 50.0f), (-window_height_px / 2.0f) + 50.0f),
        vec3(0.0, 0.0, 0.0), true);
    IsText& eText = registry.texts.get(enemyText);
    eText.doRender = true;
    registry.enemyCounters.emplace(enemyText);
    printf("%i\n", registry.enemyCounters.size());
    initialized = true;

    if (isBasic){
        if (registry.tutorialTexts.size() < 1 || !registry.texts.has(registry.tutorialTexts.entities[0])) {
            Entity instructionText = createText("", vec2(-window_width_px / 2.0f + 20.0f, 100.0f),vec3(0.0f, 0.0f, 0.0f), true);
            registry.tutorialTexts.clear();
            registry.tutorialTexts.emplace(instructionText);
            IsText &eText = registry.texts.get(instructionText);
            eText.doRender = true;
            handle_tutorial_state(true);
        }
    }
}

// Reset world without creating any entities, fromJSON takes care of rebuilding ECS
void WorldSystem::load_game() {
    // check if the file exists before proceeding
    std::ifstream file(save_file_path());
    if (!file.is_open()) {
        printf("ERROR: Failed to read save file: %s\n", save_file_path().c_str());
        return;
    }

    // Debugging for memory/component leaks
    registry.list_all_components();
    printf("Loading game\n");
//    ScreenState &screenState = registry.screenStates.components[0];
//    registry.remove_all_components_of(screenState.instruction_text);
//    for (Entity e: registry.texts.entities){
//        if (!registry.tutorialTexts.has(e)){
//            registry.remove_all_components_of(e);
//        }
//    }
    registry.texts.clear();
    registry.clear_most_components(true);
    
    // Reset the game speed
    current_speed = 1.f;
    isHealthy = true;
    isBlinded = false;
    isWet = false;
    isPlaying = false;
    inventory_visible = false;
    help_popup_visible = false;
    help_popup_page = 0;

//    Entity instruction_text = screenState.instruction_text;
//    if (!registry.texts.has(instruction_text)){
//        Entity instructionText = createText("", vec2(-window_width_px / 2.0f + 20.0f, 100.0f), vec3(0.0f, 0.0f, 0.0f), true);
//        screenState.instruction_text = instructionText;
//    }

    // clear these
    enemies.clear();
    obstacles.clear();
    weapons.clear();
    passiveItems.clear();
    consumableItems.clear();

    if (channel != -1) {
        Mix_HaltChannel(channel);
        channel = -1;
    }

    // Reset points & things that passive items affect
    points = 0;
    PLAYER_MAX_HEALTH = 100.0f;
    PLAYER_MAX_STAMINA = 100.0f;
    fire_delay = 500.0f;
    damage_multiplier = 1.0f;

    // Remove all entities that we created
    // All that have a motion, we could also iterate over all fish, eels, ... but that would be more cumbersome
    /*while (registry.motions.entities.size() > 0)
        registry.remove_all_components_of(registry.motions.entities.back());*/

    // Debugging for memory/component leaks;
    registry.list_all_components();

    // load JSON
    json loadData;
    file >> loadData;
    registry.fromJSON(loadData);
    
    // clear previous loaded map
    map_saved_entity_new_entity.clear();

    // loaded game
    std::cout << "Game Loaded !" << std::endl;

   /* for (Entity& e : registry.texts.entities) {
        if (registry.tutorialTexts.has(e)){
            std::cout << std::to_string(e) << std::endl;
        }
    }*/

    if (registry.levelStates.components.size() > 0) {
        LevelState& ls = registry.levelStates.components[0];
        // std::cout << ls.whichLevel << std::endl;
        if (ls.whichLevel == 0) {
            isBasic = true;
            isSecond = false;
            isThird = false;
        }
        else if (ls.whichLevel == 1) {
            isBasic = false;
            isSecond = false;
            isThird = false;
        }
        else if (ls.whichLevel == 2) {
            isBasic = false;
            isSecond = true;
            isThird = false;
        }
        else {
            isBasic = false;
            isSecond = false;
            isThird = true;
        }
    }

    // re-apply enemy health/dmg multipliers
    if (isSecond) {
        enemy_health_multiplier = 1.3f;
        enemy_damage_multiplier = 1.3f;
    }
    else if (isThird) {
        enemy_health_multiplier = 1.5f;
        enemy_damage_multiplier = 1.5f;
    }
    else {
        enemy_health_multiplier = 1.0f;
        enemy_damage_multiplier = 1.0f;
    }

    // re-assign world system variables
    for (Entity& e : registry.players.entities) {
        player = e;
    }

    for (Entity& e : registry.enemies.entities) {
        enemies.push_back(e);
    }

    for (Entity& e : registry.obstacles.entities) {
        obstacles.push_back(e);
    }

    for (Entity& e : registry.passiveItems.entities) {
        passiveItems.push_back(e);
    }
    
    for (Entity& e : registry.consumableItems.entities) {
        consumableItems.push_back(e);
    }

    if (registry.blindTimers.has(player)) {
        isBlinded = true;
    }

    // re-apply passive item effects

    CURR_INVENTORY_XPOS = INVENTORY_TEXT_XPOS + 50.0f;
    CURR_INVENTORY_YPOS = (-window_height_px / 2) + ((window_height_px - HELP_POPUP_HEIGHT) / 2) + 150.0f;

    if (registry.hasPassiveItems.has(player)) {
        HasPassiveItem hpi = registry.hasPassiveItems.get(player);
        for (int id : hpi.items_list) {
            CURR_INVENTORY_XPOS += SPRITE_SIZE_STANDARD;
            if (CURR_INVENTORY_XPOS > item_xpos_end) {
                CURR_INVENTORY_XPOS = item_xpos_start;
                CURR_INVENTORY_YPOS += SPRITE_SIZE_STANDARD;
            }
           if (id == P_HEALTHUP) {
                PLAYER_MAX_HEALTH += 30;
                /*if (registry.healths.has(player)) {
                    Health& h = registry.healths.get(player);
                    std::cout << h.val << std::endl;
                    h.val += 30;
                    std::cout << h.val << std::endl;
                    editHealthBar(h.val, PLAYER_MAX_HEALTH);
                }*/

            }
            else if (id == P_STAMUP) {
                PLAYER_MAX_STAMINA += 30;
                /*if (registry.staminas.has(player)) {
                    Stamina& h = registry.staminas.get(player);
                    h.val += 30;
                    editStaminaBar(h.val, PLAYER_MAX_STAMINA);
                }*/
            }
            else if (id == P_GUNMOD) {
                fire_delay = 250.0f;
            }
            else if (id == P_PADDING) {
                damage_multiplier -= 0.25f;
                if (damage_multiplier < 0.25f) {
                    damage_multiplier = 0.25f;
                }
            }
            else if (id == P_DMGUP) {
                attack_multiplier += 0.5f;
                if (attack_multiplier > 4.0f) {
                    attack_multiplier = 4.0f;
                }
            }
        }
    }

    enemy_count = enemies.size();

    if (registry.tutorialTexts.size() < 1){
        curr_state = TutorialState::WASD;
    } else {
        curr_state = registry.tutorialTexts.components[0].state;
        if (!registry.texts.has(registry.tutorialTexts.entities[0])) {
            Entity instructionText = createText("", vec2(-window_width_px / 2.0f + 20.0f, 100.0f),
                                                vec3(0.0f, 0.0f, 0.0f), true);
            IsText &eText = registry.texts.get(instructionText);
            eText.text = TUTORIAL_TEXT_MAP[(int) curr_state];
            eText.doRender = true;
        }
    }
}

void WorldSystem::create_world_one(float cell_x, float cell_y, int middle_x, int middle_y, std::vector<Entity>& obstacles, std::vector<Entity>& enemies) {
    // Creating the spawn cell in cell col 3, row 5
    obstacles.push_back(createFenceWall(renderer, {-7.5 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 5) * SPRITE_SIZE_STANDARD}, {15, 1}));
    obstacles.push_back(createFenceWall(renderer, {-7.5 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 6) * SPRITE_SIZE_STANDARD}, {15, 1}));

    obstacles.push_back(createFenceWall(renderer, {(WORLD_MIN_X + (cell_x * 3)) * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD}, {1,3}));
    obstacles.push_back(createFenceWall(renderer, {(WORLD_MIN_X + (cell_x * 3)) * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD}, {1,3}));
    obstacles.push_back(createFenceWall(renderer, {(WORLD_MIN_X + (cell_x * 4)) * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD}, {1,3}));
    obstacles.push_back(createFenceWall(renderer, {(WORLD_MIN_X + (cell_x * 4)) * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD}, {1,3}));

    obstacles.push_back(createBarrel(renderer, { -7 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -3 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -11 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -12 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-12 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD), vec2(2, 1), Effects::MUD);


    // Creating the double cell in cell col 4-5, row 5

    obstacles.push_back(createFenceWall(renderer, {3 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 5) * SPRITE_SIZE_STANDARD }, {7, 1}));
    obstacles.push_back(createFenceWall(renderer, {15 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 5) * SPRITE_SIZE_STANDARD }, {13, 1}));
    obstacles.push_back(createFenceWall(renderer, {27.5 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 5) * SPRITE_SIZE_STANDARD }, {6, 1}));

    obstacles.push_back(createFenceWall(renderer, { 10.5 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 6) * SPRITE_SIZE_STANDARD }, {22, 1}));
    obstacles.push_back(createFenceWall(renderer, { 27.5 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 6) * SPRITE_SIZE_STANDARD }, {6, 1}));

    obstacles.push_back(createFenceWall(renderer, { (WORLD_MIN_X + (cell_x * 6)) * SPRITE_SIZE_STANDARD, 5.5 * SPRITE_SIZE_STANDARD }, {1, 10}));

    for (float x = WORLD_MIN_X + (cell_x * 4) + 3; x <= (WORLD_MIN_X + (cell_x * 6)) - 1; x += 3) {
        obstacles.push_back(createBarrel(renderer, { x * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 5 + 3) * SPRITE_SIZE_STANDARD }));
        obstacles.push_back(createBarrel(renderer, { x * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 5 + 6) * SPRITE_SIZE_STANDARD }));
    }


    // Creating the quad cell in cell col 4-5, row 3-4

    obstacles.push_back(createFenceWall(renderer, { 15 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 3) * SPRITE_SIZE_STANDARD }, {31, 1}));

    obstacles.push_back(createFenceWall(renderer, { (WORLD_MIN_X + (cell_x * 4)) * SPRITE_SIZE_STANDARD, -9.5 * SPRITE_SIZE_STANDARD }, {1, 20}));
    obstacles.push_back(createFenceWall(renderer, { (WORLD_MIN_X + (cell_x * 6)) * SPRITE_SIZE_STANDARD, -9.5 * SPRITE_SIZE_STANDARD }, {1, 20}));

    obstacles.push_back(createCactus(renderer, { 6 * SPRITE_SIZE_STANDARD, -4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 13 * SPRITE_SIZE_STANDARD, -16 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { 27 * SPRITE_SIZE_STANDARD, -6 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 18 * SPRITE_SIZE_STANDARD, -9 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { 4 * SPRITE_SIZE_STANDARD, -17 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ 24 * SPRITE_SIZE_STANDARD, -5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { 15 * SPRITE_SIZE_STANDARD, -7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 8 * SPRITE_SIZE_STANDARD, -11 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { 26 * SPRITE_SIZE_STANDARD, -3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 11 * SPRITE_SIZE_STANDARD, -19 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { 7 * SPRITE_SIZE_STANDARD, -5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ 22 * SPRITE_SIZE_STANDARD, -14 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { 16 * SPRITE_SIZE_STANDARD, -13 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 9 * SPRITE_SIZE_STANDARD, -8 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { 21 * SPRITE_SIZE_STANDARD, -18 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ 17 * SPRITE_SIZE_STANDARD, -6 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(18 * SPRITE_SIZE_STANDARD, -7 * SPRITE_SIZE_STANDARD), vec2(2, 1), Effects::MUD);
    passiveItems.push_back(createPassiveItem(renderer, { 16 * SPRITE_SIZE_STANDARD, -6 * SPRITE_SIZE_STANDARD }, TEXTURE_ASSET_ID::SYRINGE, P_DMGUP));


    // Creating the cell in cell col 2, row 5

    obstacles.push_back(createFenceWall(renderer, { -27 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 6) * SPRITE_SIZE_STANDARD }, {7, 1}));
    obstacles.push_back(createFenceWall(renderer, {  -18 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 6) * SPRITE_SIZE_STANDARD }, {7, 1}));

    obstacles.push_back(createFenceWall(renderer, { (WORLD_MIN_X + (cell_x * 2)) * SPRITE_SIZE_STANDARD, 5.5 * SPRITE_SIZE_STANDARD }, {1, 10}));

    obstacles.push_back(createCactus(renderer, { -29 * SPRITE_SIZE_STANDARD, 9 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -25 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -20 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-19 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD), vec2(3, 2), Effects::MUD);
    obstacles.push_back(createCactus(renderer, { -18 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -27 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-27 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD), vec2(2, 1), Effects::MUD);
    obstacles.push_back(createCactus(renderer, { -16 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }));
    passiveItems.push_back(createPassiveItem(renderer, { -14 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }, TEXTURE_ASSET_ID::ELIXIR, P_HEALTHUP));



    // Creating the cell in cell col 1-2, row 4

    obstacles.push_back(createFenceWall(renderer, { -30 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 4) * SPRITE_SIZE_STANDARD }, {30, 1}));
    obstacles.push_back(createFenceWall(renderer, {  -33.5 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 5) * SPRITE_SIZE_STANDARD }, {22, 1}));
    obstacles.push_back(createFenceWall(renderer, {  -18 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 5) * SPRITE_SIZE_STANDARD }, {7, 1}));

    obstacles.push_back(createFenceWall(renderer, { (WORLD_MIN_X + (cell_x * 1)) * SPRITE_SIZE_STANDARD, -4.5 * SPRITE_SIZE_STANDARD }, {1, 10}));
    obstacles.push_back(createFenceWall(renderer, { (WORLD_MIN_X + (cell_x * 3)) * SPRITE_SIZE_STANDARD, -4.5 * SPRITE_SIZE_STANDARD }, {1, 10}));

    obstacles.push_back(createCactus(renderer, { -44 * SPRITE_SIZE_STANDARD, -9 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -40 * SPRITE_SIZE_STANDARD, -7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -38 * SPRITE_SIZE_STANDARD, -8 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -35 * SPRITE_SIZE_STANDARD, -3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -41 * SPRITE_SIZE_STANDARD, -6 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -42 * SPRITE_SIZE_STANDARD, -4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -32 * SPRITE_SIZE_STANDARD, -2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -31 * SPRITE_SIZE_STANDARD, -5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -36 * SPRITE_SIZE_STANDARD, -1 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -28 * SPRITE_SIZE_STANDARD, -9 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -24 * SPRITE_SIZE_STANDARD, -7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -30 * SPRITE_SIZE_STANDARD, -6 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -27 * SPRITE_SIZE_STANDARD, -3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -19 * SPRITE_SIZE_STANDARD, -8 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -17 * SPRITE_SIZE_STANDARD, -1 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-17 * SPRITE_SIZE_STANDARD, -4 * SPRITE_SIZE_STANDARD), vec2(3, 2), Effects::OASIS);
    passiveItems.push_back(createPassiveItem(renderer, { -25 * SPRITE_SIZE_STANDARD, -3 * SPRITE_SIZE_STANDARD }, TEXTURE_ASSET_ID::POWDER, P_STAMUP));



    // Creating the cell in cell col 2, row 5-6

    obstacles.push_back(createFenceWall(renderer, { -21.5 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 8) * SPRITE_SIZE_STANDARD }, {16, 1}));

    obstacles.push_back(createFenceWall(renderer, { (WORLD_MIN_X + (cell_x * 2)) * SPRITE_SIZE_STANDARD, 20.5 * SPRITE_SIZE_STANDARD }, {1, 20}));
    obstacles.push_back(createFenceWall(renderer, { (WORLD_MIN_X + (cell_x * 3)) * SPRITE_SIZE_STANDARD, 16.5 * SPRITE_SIZE_STANDARD }, {1, 14}));
    obstacles.push_back(createFenceWall(renderer, { (WORLD_MIN_X + (cell_x * 3)) * SPRITE_SIZE_STANDARD, 28 * SPRITE_SIZE_STANDARD }, {1, 3}));

    obstacles.push_back(createCactus(renderer, { -29 * SPRITE_SIZE_STANDARD, 12 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -25 * SPRITE_SIZE_STANDARD, 15 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -20 * SPRITE_SIZE_STANDARD, 18 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -18 * SPRITE_SIZE_STANDARD, 22 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-22 * SPRITE_SIZE_STANDARD, 17 * SPRITE_SIZE_STANDARD), vec2(2, 1), Effects::OASIS);
    obstacles.push_back(createCactus(renderer, { -27 * SPRITE_SIZE_STANDARD, 19 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -26 * SPRITE_SIZE_STANDARD, 13 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -22 * SPRITE_SIZE_STANDARD, 25 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -24 * SPRITE_SIZE_STANDARD, 28 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { -17 * SPRITE_SIZE_STANDARD, 29 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -19 * SPRITE_SIZE_STANDARD, 14 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-25 * SPRITE_SIZE_STANDARD, 17 * SPRITE_SIZE_STANDARD), vec2(2, 2), Effects::MUD);
    passiveItems.push_back(createPassiveItem(renderer, { -21 * SPRITE_SIZE_STANDARD, 14 * SPRITE_SIZE_STANDARD }, TEXTURE_ASSET_ID::GUNMOD, P_GUNMOD));



    // Creating the cell in cell in col 5, row 6

    obstacles.push_back(createFenceWall(renderer, { 23 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 7) * SPRITE_SIZE_STANDARD }, {15, 1}));

    obstacles.push_back(createFenceWall(renderer, { (WORLD_MIN_X + (cell_x * 5)) * SPRITE_SIZE_STANDARD, 15.5 * SPRITE_SIZE_STANDARD }, {1, 10}));
    obstacles.push_back(createFenceWall(renderer, { (WORLD_MIN_X + (cell_x * 6)) * SPRITE_SIZE_STANDARD, 15.5 * SPRITE_SIZE_STANDARD }, {1, 10}));

    obstacles.push_back(createCactus(renderer, { 25 * SPRITE_SIZE_STANDARD, 18 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { 28 * SPRITE_SIZE_STANDARD, 11 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { 17 * SPRITE_SIZE_STANDARD, 14 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { 22 * SPRITE_SIZE_STANDARD, 19 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { 26 * SPRITE_SIZE_STANDARD, 13 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createCactus(renderer, { 20 * SPRITE_SIZE_STANDARD, 17 * SPRITE_SIZE_STANDARD }));
    passiveItems.push_back(createPassiveItem(renderer, { 18 * SPRITE_SIZE_STANDARD, 17 * SPRITE_SIZE_STANDARD }, TEXTURE_ASSET_ID::PADDING, P_PADDING));


    // Creating the boss arena at cell col 3, row 8

    obstacles.push_back(createFenceWall(renderer, { -7.5 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 7) * SPRITE_SIZE_STANDARD }, {15, 1}));
    obstacles.push_back(createFenceWall(renderer, { -7.5 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 8) * SPRITE_SIZE_STANDARD }, {15, 1}));

    obstacles.push_back(createFenceWall(renderer, { (WORLD_MIN_X + (cell_x * 4)) * SPRITE_SIZE_STANDARD, 25.5 * SPRITE_SIZE_STANDARD }, {1, 10}));

}

void WorldSystem::create_world_two(float cell_x, float cell_y, int middle_x, int middle_y, std::vector<Entity>& obstacles, std::vector<Entity>& enemies) {
    obstacles.push_back(createStumpWall(renderer, { -7.5 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 5) * SPRITE_SIZE_STANDARD }, { 15, 1 }));
    obstacles.push_back(createStumpWall(renderer, { -7.5 * SPRITE_SIZE_STANDARD, (WORLD_MIN_Y + cell_y * 6) * SPRITE_SIZE_STANDARD }, { 15, 1 }));
    obstacles.push_back(createStumpWall(renderer, { (WORLD_MIN_X + (cell_x * 3)) * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }, { 1,3 }));
    obstacles.push_back(createStumpWall(renderer, { (WORLD_MIN_X + (cell_x * 3)) * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }, { 1,3 }));
    obstacles.push_back(createStumpWall(renderer, { (WORLD_MIN_X + (cell_x * 4)) * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }, { 1,3 }));
    obstacles.push_back(createStumpWall(renderer, { (WORLD_MIN_X + (cell_x * 4)) * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }, { 1,3 }));
    obstacles.push_back(createStumpWall(renderer, { 12 * SPRITE_SIZE_STANDARD, 0 * SPRITE_SIZE_STANDARD }, { 7, 1 }));
    obstacles.push_back(createStumpWall(renderer, { 3 * SPRITE_SIZE_STANDARD, 0 * SPRITE_SIZE_STANDARD }, { 6, 1 }));
    obstacles.push_back(createStumpWall(renderer, { 22.5 * SPRITE_SIZE_STANDARD, 0 * SPRITE_SIZE_STANDARD }, { 15, 1 }));
    obstacles.push_back(createStumpWall(renderer, { 14 * SPRITE_SIZE_STANDARD, 10 * SPRITE_SIZE_STANDARD }, { 30, 1 }));
    obstacles.push_back(createStumpWall(renderer, { 29 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }, { 1,10 }));
    obstacles.push_back(createStumpWall(renderer, { 12 * SPRITE_SIZE_STANDARD, -10 * SPRITE_SIZE_STANDARD }, { 7, 1 }));
    obstacles.push_back(createStumpWall(renderer, { 3 * SPRITE_SIZE_STANDARD, -10 * SPRITE_SIZE_STANDARD }, { 6, 1 }));
    obstacles.push_back(createStumpWall(renderer, { 15 * SPRITE_SIZE_STANDARD, -5 * SPRITE_SIZE_STANDARD }, { 1,11 }));
    obstacles.push_back(createStumpWall(renderer, { 0 * SPRITE_SIZE_STANDARD, -5 * SPRITE_SIZE_STANDARD }, { 1,11 }));
    obstacles.push_back(createStumpWall(renderer, { 22.5 * SPRITE_SIZE_STANDARD, -10 * SPRITE_SIZE_STANDARD }, { 15, 1 }));
    obstacles.push_back(createStumpWall(renderer, { 30 * SPRITE_SIZE_STANDARD, -20 * SPRITE_SIZE_STANDARD }, { 1,20 }));
    obstacles.push_back(createStumpWall(renderer, { 15 * SPRITE_SIZE_STANDARD, -30 * SPRITE_SIZE_STANDARD }, { 30,1 }));
	obstacles.push_back(createStumpWall(renderer, { 0 * SPRITE_SIZE_STANDARD, -25 * SPRITE_SIZE_STANDARD }, { 1,11 }));
    obstacles.push_back(createStumpWall(renderer, { 0 * SPRITE_SIZE_STANDARD, -18 * SPRITE_SIZE_STANDARD }, { 1,3 }));
    obstacles.push_back(createStumpWall(renderer, { 0 * SPRITE_SIZE_STANDARD, -12 * SPRITE_SIZE_STANDARD }, { 1,3 }));
    obstacles.push_back(createStumpWall(renderer, { -15 * SPRITE_SIZE_STANDARD, -20 * SPRITE_SIZE_STANDARD }, { 30,1 }));
    obstacles.push_back(createStumpWall(renderer, { -7.5 * SPRITE_SIZE_STANDARD, -10 * SPRITE_SIZE_STANDARD }, { 15,1 }));
	obstacles.push_back(createStumpWall(renderer, { -30 * SPRITE_SIZE_STANDARD, 0 * SPRITE_SIZE_STANDARD }, { 1,41 }));
    obstacles.push_back(createStumpWall(renderer, { -15 * SPRITE_SIZE_STANDARD, 15 * SPRITE_SIZE_STANDARD }, { 1,10 }));
    obstacles.push_back(createStumpWall(renderer, { -22.5 * SPRITE_SIZE_STANDARD, 20 * SPRITE_SIZE_STANDARD }, { 15, 1 }));
    obstacles.push_back(createStumpWall(renderer, { -18 * SPRITE_SIZE_STANDARD, 0 * SPRITE_SIZE_STANDARD }, { 7, 1 }));
    obstacles.push_back(createStumpWall(renderer, { -27 * SPRITE_SIZE_STANDARD, 0 * SPRITE_SIZE_STANDARD }, { 6, 1 }));
    obstacles.push_back(createStumpWall(renderer, { -18 * SPRITE_SIZE_STANDARD, 10 * SPRITE_SIZE_STANDARD }, { 7, 1 }));
    obstacles.push_back(createStumpWall(renderer, { -27 * SPRITE_SIZE_STANDARD, 10 * SPRITE_SIZE_STANDARD }, { 6, 1 }));
    obstacles.push_back(createStumpWall(renderer, { -18 * SPRITE_SIZE_STANDARD, -10 * SPRITE_SIZE_STANDARD }, { 7, 1 }));
    obstacles.push_back(createStumpWall(renderer, { -27 * SPRITE_SIZE_STANDARD, -10 * SPRITE_SIZE_STANDARD }, { 6, 1 }));
    obstacles.push_back(createStumpWall(renderer, { -15 * SPRITE_SIZE_STANDARD, -8 * SPRITE_SIZE_STANDARD }, { 1,3 }));
    obstacles.push_back(createStumpWall(renderer, { -15 * SPRITE_SIZE_STANDARD, -2 * SPRITE_SIZE_STANDARD }, { 1,3 }));
    obstacles.push_back(createStumpWall(renderer, { -15 * SPRITE_SIZE_STANDARD, -18 * SPRITE_SIZE_STANDARD }, { 1,3 }));
    obstacles.push_back(createStumpWall(renderer, { -15 * SPRITE_SIZE_STANDARD, -12 * SPRITE_SIZE_STANDARD }, { 1,3 }));


    obstacles.push_back(createBarrel(renderer, { -12 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 5 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 10 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createThorny(renderer, { 3 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 12 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 25 * SPRITE_SIZE_STANDARD, 9 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createThorny(renderer, { 18 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(19 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD), vec2(2, 3), Effects::MUD);
    obstacles.push_back(createBush(renderer, { 7 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 14 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createThorny(renderer, { 21 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 28 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 10 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createThorny(renderer, { 5 * SPRITE_SIZE_STANDARD, 9 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 19 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 21 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 23 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 25 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(23 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD), vec2(3, 2), Effects::MUD);

    obstacles.push_back(createBush(renderer, { 1 * SPRITE_SIZE_STANDARD, -1 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ 3 * SPRITE_SIZE_STANDARD, -2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ 4 * SPRITE_SIZE_STANDARD, -5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 5 * SPRITE_SIZE_STANDARD, -3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 6 * SPRITE_SIZE_STANDARD, -8 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ 7 * SPRITE_SIZE_STANDARD, -4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 12 * SPRITE_SIZE_STANDARD, -3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 13 * SPRITE_SIZE_STANDARD, -5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ 14 * SPRITE_SIZE_STANDARD, -2 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createBush(renderer, { 1 * SPRITE_SIZE_STANDARD, -8 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 2 * SPRITE_SIZE_STANDARD, -12 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 4 * SPRITE_SIZE_STANDARD, -15 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 5 * SPRITE_SIZE_STANDARD, -18 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 7 * SPRITE_SIZE_STANDARD, -14 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 8 * SPRITE_SIZE_STANDARD, -22 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 10 * SPRITE_SIZE_STANDARD, -25 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 11 * SPRITE_SIZE_STANDARD, -20 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-25 * SPRITE_SIZE_STANDARD, -14 * SPRITE_SIZE_STANDARD), vec2(2, 2), Effects::MUD);
    obstacles.push_back(createBush(renderer, { 12 * SPRITE_SIZE_STANDARD, -28 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { 14 * SPRITE_SIZE_STANDARD, -17 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createTNT({ 16 * SPRITE_SIZE_STANDARD, -10 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ 18 * SPRITE_SIZE_STANDARD, -13 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ 20 * SPRITE_SIZE_STANDARD, -22 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ 22 * SPRITE_SIZE_STANDARD, -16 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(23 * SPRITE_SIZE_STANDARD, -15 * SPRITE_SIZE_STANDARD), vec2(1, 2), Effects::MUD);
    obstacles.push_back(createTNT({ 24 * SPRITE_SIZE_STANDARD, -25 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createThorny(renderer, { 19 * SPRITE_SIZE_STANDARD, -12 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createThorny(renderer, { 21 * SPRITE_SIZE_STANDARD, -19 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createThorny(renderer, { 23 * SPRITE_SIZE_STANDARD, -27 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createThorny(renderer, { 25 * SPRITE_SIZE_STANDARD, -15 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(28 * SPRITE_SIZE_STANDARD, -26 * SPRITE_SIZE_STANDARD), vec2(2, 2), Effects::MUD);
    obstacles.push_back(createThorny(renderer, { 27 * SPRITE_SIZE_STANDARD, -24 * SPRITE_SIZE_STANDARD }));


    obstacles.push_back(createBarrel(renderer, { 26 * SPRITE_SIZE_STANDARD, -11 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 28 * SPRITE_SIZE_STANDARD, -19 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 22 * SPRITE_SIZE_STANDARD, -23 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 29 * SPRITE_SIZE_STANDARD, -26 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createTNT({ -1 * SPRITE_SIZE_STANDARD, -2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -3 * SPRITE_SIZE_STANDARD, -3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -4 * SPRITE_SIZE_STANDARD, -5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -5 * SPRITE_SIZE_STANDARD, -4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -6 * SPRITE_SIZE_STANDARD, -6 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -7 * SPRITE_SIZE_STANDARD, -2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -9 * SPRITE_SIZE_STANDARD, -7 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-5 * SPRITE_SIZE_STANDARD, -4 * SPRITE_SIZE_STANDARD), vec2(2, 2), Effects::MUD);
    obstacles.push_back(createTNT({ -10 * SPRITE_SIZE_STANDARD, -9 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -12 * SPRITE_SIZE_STANDARD, -8 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -13 * SPRITE_SIZE_STANDARD, -1 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createTNT({ -2 * SPRITE_SIZE_STANDARD, -7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -4 * SPRITE_SIZE_STANDARD, -9 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -6 * SPRITE_SIZE_STANDARD, -10 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -8 * SPRITE_SIZE_STANDARD, -5 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-5 * SPRITE_SIZE_STANDARD, -4 * SPRITE_SIZE_STANDARD), vec2(2, 2), Effects::MUD);
    obstacles.push_back(createTNT({ -11 * SPRITE_SIZE_STANDARD, -3 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createBush(renderer, { -1 * SPRITE_SIZE_STANDARD, -19 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -3 * SPRITE_SIZE_STANDARD, -14 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -5 * SPRITE_SIZE_STANDARD, -16 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createThorny(renderer, { -7 * SPRITE_SIZE_STANDARD, -12 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { -9 * SPRITE_SIZE_STANDARD, -18 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -10 * SPRITE_SIZE_STANDARD, -15 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -12 * SPRITE_SIZE_STANDARD, -17 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-8 * SPRITE_SIZE_STANDARD, -14 * SPRITE_SIZE_STANDARD), vec2(4, 3), Effects::OASIS);
    obstacles.push_back(createThorny(renderer, { -13 * SPRITE_SIZE_STANDARD, -11 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createBush(renderer, { -16 * SPRITE_SIZE_STANDARD, -19 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -18 * SPRITE_SIZE_STANDARD, -13 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -20 * SPRITE_SIZE_STANDARD, -15 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createThorny(renderer, { -22 * SPRITE_SIZE_STANDARD, -12 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { -25 * SPRITE_SIZE_STANDARD, -17 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -27 * SPRITE_SIZE_STANDARD, -14 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-24 * SPRITE_SIZE_STANDARD, -17 * SPRITE_SIZE_STANDARD), vec2(2, 2), Effects::MUD);
    obstacles.push_back(createBarrel(renderer, { -29 * SPRITE_SIZE_STANDARD, -16 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createTNT({ -16 * SPRITE_SIZE_STANDARD, -9 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -17 * SPRITE_SIZE_STANDARD, -4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { -19 * SPRITE_SIZE_STANDARD, -6 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createThorny(renderer, { -21 * SPRITE_SIZE_STANDARD, -2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -24 * SPRITE_SIZE_STANDARD, -8 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { -26 * SPRITE_SIZE_STANDARD, -3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -28 * SPRITE_SIZE_STANDARD, -1 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createBush(renderer, { -16 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -17 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -19 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createThorny(renderer, { -22 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -24 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { -26 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-24 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD), vec2(2, 1), Effects::OASIS);
    obstacles.push_back(createBarrel(renderer, { -28 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createTNT({ -16 * SPRITE_SIZE_STANDARD, 12 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -17 * SPRITE_SIZE_STANDARD, 14 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBush(renderer, { -19 * SPRITE_SIZE_STANDARD, 18 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createThorny(renderer, { -21 * SPRITE_SIZE_STANDARD, 15 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createTNT({ -23 * SPRITE_SIZE_STANDARD, 16 * SPRITE_SIZE_STANDARD }));
    createWorldEffect(vec2(-23 * SPRITE_SIZE_STANDARD, 17 * SPRITE_SIZE_STANDARD), vec2(2, 3), Effects::MUD);
    obstacles.push_back(createBush(renderer, { -25 * SPRITE_SIZE_STANDARD, 13 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -28 * SPRITE_SIZE_STANDARD, 17 * SPRITE_SIZE_STANDARD }));

    passiveItems.push_back(createPassiveItem(renderer, { -21 * SPRITE_SIZE_STANDARD, -12 * SPRITE_SIZE_STANDARD }, TEXTURE_ASSET_ID::SYRINGE, P_DMGUP));
    passiveItems.push_back(createPassiveItem(renderer, { 4 * SPRITE_SIZE_STANDARD, -24 * SPRITE_SIZE_STANDARD }, TEXTURE_ASSET_ID::ELIXIR, P_HEALTHUP));
    passiveItems.push_back(createPassiveItem(renderer, { 19 * SPRITE_SIZE_STANDARD, -13 * SPRITE_SIZE_STANDARD }, TEXTURE_ASSET_ID::POWDER, P_STAMUP));
    passiveItems.push_back(createPassiveItem(renderer, { -21 * SPRITE_SIZE_STANDARD, 14 * SPRITE_SIZE_STANDARD }, TEXTURE_ASSET_ID::GUNMOD, P_GUNMOD));
    passiveItems.push_back(createPassiveItem(renderer, { 22 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }, TEXTURE_ASSET_ID::PADDING, P_PADDING));
}

void WorldSystem::create_world_three(float cell_x, float cell_y, int middle_x, int middle_y, std::vector<Entity>& obstacles, std::vector<Entity>& enemies) {
    obstacles.push_back(createRockWall(renderer, { -7.5 * SPRITE_SIZE_STANDARD, 0 * SPRITE_SIZE_STANDARD }, { 45, 1 }));
    obstacles.push_back(createRockWall(renderer, { -7.5 * SPRITE_SIZE_STANDARD, 10 * SPRITE_SIZE_STANDARD }, { 45, 1 }));
    obstacles.push_back(createRockWall(renderer, { -30 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }, { 1, 10 }));
    obstacles.push_back(createRockWall(renderer, { 15 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }, { 1, 10 }));

    obstacles.push_back(createMinecart(renderer, { -28 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { -15 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -3 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -12 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { -5 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { -19 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -9 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -21 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { -2 * SPRITE_SIZE_STANDARD, 9 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { -7 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -14 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { -25 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { -20 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -27 * SPRITE_SIZE_STANDARD, 9 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { -16 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { -13 * SPRITE_SIZE_STANDARD, 9 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -4 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { -6 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { -29 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -11 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -17 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { -12 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { -8 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -23 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { -10 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { -16 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -9 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -20 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { -27 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { -11 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -22 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { -8 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { -14 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -6 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { -18 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { 1 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { 5 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 9 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 10 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { 3 * SPRITE_SIZE_STANDARD, 5 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { 6 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 13 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { 2 * SPRITE_SIZE_STANDARD, 6 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { 7 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 11 * SPRITE_SIZE_STANDARD, 9 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { 14 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { 4 * SPRITE_SIZE_STANDARD, 2 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 9 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 13 * SPRITE_SIZE_STANDARD, 3 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { 6 * SPRITE_SIZE_STANDARD, 9 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { 1 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 5 * SPRITE_SIZE_STANDARD, 8 * SPRITE_SIZE_STANDARD }));

    obstacles.push_back(createMinecart(renderer, { 8 * SPRITE_SIZE_STANDARD, 1 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createMinecartFull(renderer, { 2 * SPRITE_SIZE_STANDARD, 7 * SPRITE_SIZE_STANDARD }));
    obstacles.push_back(createBarrel(renderer, { 10 * SPRITE_SIZE_STANDARD, 4 * SPRITE_SIZE_STANDARD }));
}




// Compute collisions between entities
void WorldSystem::handle_collisions(float elapsed_ms) {
    // Loop over all collisions detected by the physics system
    auto &collisionsRegistry = registry.collisions;
    if (registry.players.get(player).worldEffect == Effects::MUD){
        registry.players.get(player).worldEffect = Effects::NONE;
    }

    for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
        // The entity and its collider
    // CITATION: method of collision detection inspired by: 
    // https://happycoding.io/tutorials/processing/collision-detection#rectangle-rectangle-collision-detection

        Entity entity = collisionsRegistry.entities[i];
        Entity collider = collisionsRegistry.components[i].other;

        // IMPORTANT! Please generally try to treat enemies and players as the 'collider', and other things as the 'entity'. 

        if (registry.motions.has(entity) && registry.motions.has(collider) && !registry.uiElements.has(entity) && !registry.uiElements.has(collider)) {

            Motion& collider_motion = registry.motions.get(collider);
            Motion& entity_motion = registry.motions.get(entity);

            float collider_radius_x = (abs(collider_motion.scale.x) / 2.0f);
            float collider_radius_y = (abs(collider_motion.scale.y) / 2.0f);

            float collider_left = collider_motion.position.x - collider_radius_x;
            float collider_right = collider_motion.position.x + collider_radius_x;
            float collider_top = collider_motion.position.y - collider_radius_y;
            float collider_bottom = collider_motion.position.y + collider_radius_y;

            float obj_left = entity_motion.position.x - (abs(entity_motion.scale.x) / 2.0f);
            float obj_right = entity_motion.position.x + (abs(entity_motion.scale.x) / 2.0f);
            float obj_top = entity_motion.position.y - (abs(entity_motion.scale.y) / 2.0f);
            float obj_bottom = entity_motion.position.y + (abs(entity_motion.scale.y) / 2.0f);

            float obj_radius_x = abs(entity_motion.scale.x) / 2.0f;
            float obj_radius_y = abs(entity_motion.scale.y) / 2.0f;

            // which direction are we colliding from?
            float delta_x = collider_motion.position.x - entity_motion.position.x;
            float delta_y = collider_motion.position.y - entity_motion.position.y;

            // get the x, y distance required to resolve the collision
            float overlap_x = collider_radius_x + obj_radius_x - abs(delta_x);
            float overlap_y = collider_radius_y + obj_radius_y - abs(delta_y);

            // if player collides with a damaging object, deal damage
            if (registry.players.has(collider) && registry.damaging.has(entity) && registry.healths.has(collider) && !registry.invincibleTimers.has(collider)) {
                if (registry.explosions.has(entity)) {
                    auto& explosion = registry.explosions.get(entity);
                    if (!HasBeenHitByExplosion(explosion, collider)) {
                        explosion.entitiesHit.push_back(&collider);
                        goto PlayerGetHit;
                    }
                    goto next;
                }
            PlayerGetHit:
                Health& health = registry.healths.get(collider);
                Damaging& damaging = registry.damaging.get(entity);

                Player &player_component = registry.players.get(collider);
                if (!(player_component.worldEffect == Effects::OASIS && registry.fires.has(entity))){
                    if (health.val > 0) {
                        if (!registry.hurtSoundTimers.has(collider)) {
                            registry.hurtSoundTimers.emplace(collider);
                            Mix_PlayChannel(-1, hurt_sound, 0);
                        }
                    }

                    health.val -= damaging.val * damage_multiplier;
                    printf("New health value for player: %i\n", health.val);
                    createColoredBurst(collider, vec4(1.0, 0.0, 0.0, 1.0));
                }

                editHealthBar(health.val, PLAYER_MAX_HEALTH);

                if (!registry.invincibleTimers.has(collider)) {
                    vec4& c = registry.colors.get(collider);
                    c = vec4(1.0f, 0.0f, 0.0f, 1.0f);
                    InvincibleTimer& it = registry.invincibleTimers.emplace(collider);
                    if (registry.fires.has(entity)) {
                        it.counter_ms = 500.0f;
                    }
                }

                // 'kill' player - probably refine this later
                if (health.val <= 0 && !registry.deathTimers.has(collider)) {

                    if (registry.motions.has(entity)) {
                        Motion& dm = registry.motions.get(entity);
                        dm.velocity = { 0.0f, 0.0f };
                    }

                    Mix_PlayChannel(-1, dead_sound, 0);
                    registry.deathTimers.emplace(collider);
                }
            }

            // if enemy collides with a EnemyDamaging object, deal damage
            if (registry.enemies.has(collider) && registry.enemyDamaging.has(entity) && registry.healths.has(collider) && !registry.invincibleTimers.has(collider)) {
                if (registry.explosions.has(entity)) {
                    auto& explosion = registry.explosions.get(entity);
                    if (!HasBeenHitByExplosion(explosion, collider)) {
                        explosion.entitiesHit.push_back(&collider);
                        goto EnemyGetHit;
                    }
                    goto next;
                }
            EnemyGetHit:

                Health& health = registry.healths.get(collider);
                EnemyDamaging& damaging = registry.enemyDamaging.get(entity);

                float this_multiplier = 1.0f;
                // only add player attack multiplier if it's their bullet
                // currently that's the only thing that can damage enemies, but that may change in the future.
                if (registry.projectiles.has(entity)) {
                    this_multiplier = attack_multiplier;
                }
                Enemy &enemy = registry.enemies.get(collider);
                if (!(enemy.worldEffect == Effects::OASIS && registry.fires.has(entity))){

                    health.val -= damaging.val * this_multiplier;
                    printf("%f, %i, %f", attack_multiplier, damaging.val, attack_multiplier * damaging.val);
                    printf("New health value for enemy: %i\n", health.val);
                    if (!registry.hurtSoundTimers.has(collider)) {
                        registry.hurtSoundTimers.emplace(collider);
                        Mix_PlayChannel(-1, bullet_hit, 0);
                    }
                }

                // edit this enemy's health bar, if it has one
                if (registry.hasHealthBars.has(collider)) {
                    HasHealthBar hhb = registry.hasHealthBars.get(collider);
                    Entity hb = hhb.healthBar;
                    if (registry.motions.has(hb)) {
                        Motion& hb_motion = registry.motions.get(hb);
                        hb_motion.scale.x = BAR_LENGTH_ENEMY - (BAR_LENGTH_ENEMY * (1 - ((float)health.val / ENEMY_MAX_HEALTH)));
                    }
                }
                createColoredBurst(collider, vec4(1.0, 0.0, 0.0, 1.0));

                /*if (!registry.invincibleTimers.has(collider)) {
                    vec4& c = registry.colors.get(collider);
                    c = vec4(1.0f, 0.0f, 0.0f, 1.0f);
                    InvincibleTimer& it = registry.invincibleTimers.emplace(collider);
                    it.counter_ms = 500.0f;
                }*/

                if (health.val <= 0) {
                    // remove the weapon associated to the enemy if it exists
                    // TODO: probably a cleaner way to do this
                    Mix_PlayChannel(-1, enemy_dead, 0);
                    if (registry.weaponHolders.has(collider)) {
                        Entity& weapon = registry.weaponHolders.get(collider).weapon;
                        registry.remove_all_components_of(weapon);
                    }

                    // remove the limb entities associated to the enemy
                    if (registry.skeletons.has(collider)) {
                        Entity& left_limb = registry.skeletons.get(collider).left_arm;
                        registry.remove_all_components_of(left_limb);                        
                        Entity& right_limb = registry.skeletons.get(collider).right_arm;
                        registry.remove_all_components_of(right_limb);
                    }

                    // remove the health bar associated to the enemy if it exists
                    if (registry.hasHealthBars.has(collider)) {
                        Entity& hb = registry.hasHealthBars.get(collider).healthBar;
                        registry.remove_all_components_of(hb);
                    }

                    // delete the particles associated to the enemy
                    for (Entity& e : registry.particles.entities) {
                        if (registry.enemies.has(registry.particles.get(e).parent))
                            registry.remove_all_components_of(e);
                    }

                    // have a 50% chance to drop a consumable
                    int will_drop_consumable = consumable_drop_dist(rng);
                    if (will_drop_consumable == 1) {
                        int which_consumable = consumable_dist(rng) - 1;
                        Entity this_consumable;
                        if (which_consumable == (int) Consumable::CHICKEN) {
                            this_consumable = createConsumableItem(renderer, collider_motion.position, TEXTURE_ASSET_ID::CHICKEN, Consumable::CHICKEN);
                        }
                        else if (which_consumable == (int)Consumable::HEARTY_STEW) {
                            this_consumable = createConsumableItem(renderer, collider_motion.position, TEXTURE_ASSET_ID::STEW, Consumable::HEARTY_STEW);
                        }
                        else if (which_consumable == (int)Consumable::LANDMINE) {
                            this_consumable = createConsumableItem(renderer, collider_motion.position, TEXTURE_ASSET_ID::LANDMINE, Consumable::LANDMINE);

                        }
                        else if (which_consumable == (int)Consumable::STAMINA_POTION) {
                            this_consumable = createConsumableItem(renderer, collider_motion.position, TEXTURE_ASSET_ID::POTION, Consumable::STAMINA_POTION);
                        }

                        consumableItems.push_back(this_consumable);
                    }
                    

                    registry.remove_all_components_of(collider);
                    enemy_count--;
                    points += 1;

                    // update enemy counter
//                    if(enemiesLeft > 0){
//                        enemiesLeft -= 1;
//                    }

                    printf("You killed an enemy!\n");
                    for (Entity enemyCounter : registry.enemyCounters.entities) {
                        if (registry.texts.has(enemyCounter)) {
                            IsText& it = registry.texts.get(enemyCounter);
                            it.text = std::string("Enemies left: ") + std::to_string(enemy_count);
                        }
                    }
                }

                // landmine explosion

                if (registry.consumableItems.has(entity) && !registry.consumableItems.get(entity).player_collidable) {
                    Entity explosion = Entity();
                    Motion& motion = registry.motions.emplace(explosion);
                    motion.position = registry.motions.get(entity).position;
                    motion.angle = 0.0f;
                    motion.velocity = { 0, 0 };
                    motion.scale = { 2*SPRITE_SIZE_STANDARD , 2*SPRITE_SIZE_STANDARD };
                    if (!registry.renderRequests.has(explosion)) {
                        registry.renderRequests.insert(
                            explosion, {
                                    TEXTURE_ASSET_ID::EXPLOSION,
                                    EFFECT_ASSET_ID::TEXTURED,
                                    GEOMETRY_BUFFER_ID::SPRITE
                            }
                        );
                        RenderGroup& rg = registry.renderGroups.emplace(explosion);
                        rg.entities.push_back(explosion);
                        rg.indices.push_back(0);
                    }
                    AnimationState& as = registry.animationStates.emplace(explosion);
                    as.num_ms = 50.0f;
                    as.counter_ms = as.num_ms;
                    as.num_rows = 1;
                    as.num_cols = 4;
                    as.col = 0;
                    as.row = 0;

                    registry.animationTimers.emplace(explosion);
                    Mix_PlayChannel(-1, explosion_sound, 0);

                    registry.remove_all_components_of(entity);
                }
            }

        next:

            //if explosion collides with TNT
            if (registry.explosions.has(entity) && registry.explodingTNTs.has(collider)) {
                auto& tnt = registry.explodingTNTs.get(collider);
                tnt.hp = 0;
            }

            //if projectiles or fire collides with TNT
            if ((registry.projectiles.has(entity) || registry.fires.has(entity)) && registry.explodingTNTs.has(collider)) {
                auto& tnt = registry.explodingTNTs.get(collider);
                --tnt.hp;
            }

            //if explosion collide with enemy/player (for displacement only)
            if (registry.explosions.has(entity) &&
                (registry.players.has(collider) || registry.enemies.has(collider))) {
                auto& explosion = registry.explosions.get(entity);
                vec2 direction = collider_motion.position - entity_motion.position;
                direction = glm::normalize(direction);
                collider_motion.position += direction * explosion.knockbackStrength;
                continue;
            }
            else if (registry.explosions.has(entity) || registry.explosions.has(collider)) {
                continue;
            }

            // exclude the following combinations of entities from colliding
// NOTE: it is generally more efficient to exclude them here, rather than in physics_system
            if (!(registry.worldObjects.has(entity) && registry.worldObjects.has(collider))
            &&  !(registry.projectiles.has(collider) && registry.damaging.has(collider) && registry.enemies.has(entity))
            &&  !(registry.projectiles.has(entity) && registry.damaging.has(entity) && registry.enemies.has(collider))
            &&  !(registry.projectiles.has(entity) && registry.projectiles.has(entity))
            &&  !(registry.meshPtrs.has(entity) || registry.meshPtrs.has(collider))
            &&  !(registry.weapons.has(entity) || registry.weapons.has(collider))
            &&  !(registry.passiveItems.has(entity) || registry.passiveItems.has(collider))
            && !(registry.consumableItems.has(entity) || registry.consumableItems.has(collider))
            && !registry.uiElements.has(entity) && !registry.uiElements.has(collider) 
            && !(registry.canMoveThroughs.has(entity) || registry.canMoveThroughs.has(collider))
            && !registry.worldEffects.has(entity) && !registry.worldEffects.has(collider)) {

                // resolve the collision by taking the minimum overlap between x and y
                if (overlap_x < overlap_y) {
                    if (delta_x > 0.0f) {
                        collider_motion.position.x += overlap_x;
                    }
                    else {
                        collider_motion.position.x -= overlap_x;
                    }
                }
                else {
                    if (delta_y > 0.0f) {
                        collider_motion.position.y += overlap_y;
                    }
                    else {
                        collider_motion.position.y -= overlap_y;
                    }
                }

                // add collisonTimers to each
                if (!registry.collisionTimers.has(entity)) {
                    registry.collisionTimers.emplace(entity);
                }

                if (!registry.collisionTimers.has(collider)) {
                    registry.collisionTimers.emplace(collider);
                }

            } else if (registry.meshPtrs.has(entity) && !registry.uiElements.has(entity) && !registry.uiElements.has(collider) && !registry.canMoveThroughs.has(collider)) { // resolve mesh collision
                Mesh* mesh = registry.meshPtrs.get(entity);
                std::vector<ColoredVertex> vertices = mesh->vertices;
                std::vector<uint16_t> indices = mesh->vertex_indices;

                Transform trans;
                trans.translate(entity_motion.position);
                trans.rotate(entity_motion.angle);
                trans.scale(entity_motion.scale);

                for (ColoredVertex& vertex : vertices) {
                    vec2 vertex_position = trans.mat * vec3(vertex.position.x, vertex.position.y, 1);

                    bool horizontal_collision = false;
                    bool vertical_collision = false;
                    if ((vertex_position.x >= collider_left)
                        || (vertex_position.x <= collider_right)) {
                        horizontal_collision = true;
                    }

                    if ((vertex_position.y > collider_top)
                        || (vertex_position.y < collider_bottom)) {
                        vertical_collision = true;
                    }

                    if (horizontal_collision && vertical_collision) {
                        // which direction are we colliding from?
                        float delta_x = collider_motion.position.x - vertex_position.x;
                        float delta_y = collider_motion.position.y - vertex_position.y;

                        // get the x, y distance required to resolve the collision
                        float overlap_x = collider_radius_x - abs(delta_x);
                        float overlap_y = collider_radius_x - abs(delta_y);

                        // resolve the collision by taking the minimum overlap between x and y
                        // NOTE: a small hack is used here, we move the player by some (small) fixed amount
                        //       this isn't the exact overlap distance, but it's good enough for the knockback
                        //       and avoids more complex calculations to get it working.
                        if (overlap_x < overlap_y) {
                            if (delta_x > 0.0f) {
                                collider_motion.position.x += 1.0f;
                            }
                            else {
                                collider_motion.position.x -= 1.0f;
                            }
                        }
                        else {
                            if (delta_y > 0.0f) {
                                collider_motion.position.y += 1.0f;
                            }
                            else {
                                collider_motion.position.y -= 1.0f;

                            }
                        }
                        break;
                    }
                }

                // add collisonTimers to each
                if (!registry.collisionTimers.has(entity)) {
                    registry.collisionTimers.emplace(entity);
                }

                if (!registry.collisionTimers.has(collider)) {
                    registry.collisionTimers.emplace(collider);
                }
            }



            // handle case if object is a knockbacker and entity is able to be knocked back (i.e., player or enemy)
            if (registry.knockbackers.has(entity) && registry.movables.has(collider) && !registry.uiElements.has(entity) && !registry.uiElements.has(collider)
                && ((registry.knockbackers.get(entity).is_enemy_affecting && registry.enemies.has(collider))
                    || (registry.knockbackers.get(entity).is_player_affecting && registry.players.has(collider)))) {
                if (entity_motion.velocity.x == 0 && entity_motion.velocity.y == 0) {
                    collider_motion.velocity *= -1; // stationary knockbacker
                }
                else {
                    collider_motion.velocity = entity_motion.velocity; // moving knockbacker
                }

                if (!registry.knockbackTimers.has(collider)) {
                    registry.knockbackTimers.emplace(collider);
                }
                if (registry.dashCooldownTimers.has(collider)) {
                    registry.dashCooldownTimers.remove(collider);
                }
            }    

            // if player collides with a damaging object, deal damage
            if (registry.players.has(collider) && registry.damaging.has(entity) && registry.healths.has(collider) && !registry.invincibleTimers.has(collider)) {
                Health& health = registry.healths.get(collider);
                Damaging& damaging = registry.damaging.get(entity);

                if (health.val > 0) {
                    Mix_PlayChannel(-1, hurt_sound, 0);
                }

                if (health.val > 40) {
                    isHealthy = true;
                }
                else {
                    isHealthy = false;
                    if (isPlaying == false) {
                        channel = Mix_PlayChannel(-1, heartbeat, -1);
						isPlaying = true;
                    }
                }


                health.val -= damaging.val * damage_multiplier;
                printf("New health value for player: %i\n", health.val);

                editHealthBar(health.val, PLAYER_MAX_HEALTH);

                if (!registry.invincibleTimers.has(collider)) {
                    vec4& c = registry.colors.get(collider);
                    c = vec4(1.0f, 0.0f, 0.0f, 1.0f);
                    InvincibleTimer& it = registry.invincibleTimers.emplace(collider);
                    if (registry.fires.has(entity)) {
                        it.counter_ms = 500.0f;
                    }
                }

                // 'kill' player - probably refine this later
                if (health.val <= 0 && !registry.deathTimers.has(collider)) {

                    if (registry.motions.has(entity)) {
                        Motion& dm = registry.motions.get(entity);
                        dm.velocity = { 0.0f, 0.0f };
                    }

                    Mix_PlayChannel(-1, dead_sound, 0);
                    registry.deathTimers.emplace(collider);
                }
            }

            // if player collides with a blinding object, blind the player
            if (registry.players.has(collider) && registry.blinders.has(entity)) {
                if (!registry.blindTimers.has(collider)) {
                    isBlinded = true;
                    registry.blindTimers.emplace(collider);
                }
                else {
                    isBlinded = true;
                    BlindTimer& bt = registry.blindTimers.get(collider);
                    bt.counter_ms = 5000.0f;
                }
            }

            // if player collides with a bleeding object, bleed the player

            if (registry.players.has(collider) && registry.bleedCausers.has(entity) && !registry.bleedTimers.has(collider)) {
                registry.bleedTimers.emplace(collider);
            }

            // if enemy collides with a EnemyDamaging object, deal damage
            if (registry.enemies.has(collider) && registry.enemyDamaging.has(entity) && registry.healths.has(collider) && !registry.invincibleTimers.has(collider)) {
                Health& health = registry.healths.get(collider);
                EnemyDamaging& damaging = registry.enemyDamaging.get(entity);
                if (!registry.attacking.has(collider)){
                    Attacking &attacking = registry.attacking.emplace(collider);
                } else {
                    Attacking &attacking = registry.attacking.get(collider);
                    attacking.counter_ms = 3000.f;
                }


                float this_multiplier = 1.0f;
                // only add player attack multiplier if it's their bullet
                // currently that's the only thing that can damage enemies, but that may change in the future.
                if (registry.projectiles.has(entity)) {
                    this_multiplier = attack_multiplier;
                }

                Enemy &enemy = registry.enemies.get(collider);
                if (!(enemy.worldEffect == Effects::OASIS && registry.fires.has(entity))){
                    health.val -= damaging.val * this_multiplier;
                    printf("%f, %i, %f", attack_multiplier, damaging.val, attack_multiplier * damaging.val);
                    printf("New health value for enemy: %i\n", health.val);
                    if (!registry.hurtSoundTimers.has(collider)) {
                        registry.hurtSoundTimers.emplace(collider);
                        Mix_PlayChannel(-1, bullet_hit, 0);
                    }
                }
                // edit this enemy's health bar, if it has one
                if (registry.hasHealthBars.has(collider)) {
                    HasHealthBar hhb = registry.hasHealthBars.get(collider);
                    Entity hb = hhb.healthBar;
                    if (registry.motions.has(hb)) {
                        Motion& hb_motion = registry.motions.get(hb);
                        hb_motion.scale.x = BAR_LENGTH_ENEMY - (BAR_LENGTH_ENEMY * (1 - ((float)health.val / ENEMY_MAX_HEALTH)));
                    }
                }
                
                // only give enemy immunity to fire instakill
                if (!registry.invincibleTimers.has(collider) && registry.fires.has(entity)) {
                    vec4& c = registry.colors.get(collider);
                    c = vec4(1.0f, 0.0f, 0.0f, 1.0f);
                    InvincibleTimer& it = registry.invincibleTimers.emplace(collider);
                    it.counter_ms = 500.0f;
                }


                if (health.val <= 0) {
                    // remove the weapon associated to the enemy if it exists
                    // TODO: probably a cleaner way to do this
                    Mix_PlayChannel(-1, enemy_dead, 0);
                    if (registry.weaponHolders.has(collider)) {
                        Entity& weapon = registry.weaponHolders.get(collider).weapon;
                        registry.remove_all_components_of(weapon);
                    }

                    // remove the limb entities associated to the enemy
                    if (registry.skeletons.has(collider)) {
                        Entity& left_limb = registry.skeletons.get(collider).left_arm;
                        registry.remove_all_components_of(left_limb);                        
                        Entity& right_limb = registry.skeletons.get(collider).right_arm;
                        registry.remove_all_components_of(right_limb);
                    }

                    // remove the health bar associated to the enemy if it exists
                    if (registry.hasHealthBars.has(collider)) {
                        Entity& hb = registry.hasHealthBars.get(collider).healthBar;
                        registry.remove_all_components_of(hb);
                    }

                    // have a 50% chance to drop a consumable

                    int will_drop_consumable = consumable_drop_dist(rng);
                    if (will_drop_consumable == 1) {
                        int which_consumable = consumable_dist(rng) - 1;
                        Entity this_consumable;
                        if (which_consumable == (int)Consumable::CHICKEN) {
                            this_consumable = createConsumableItem(renderer, collider_motion.position, TEXTURE_ASSET_ID::CHICKEN, Consumable::CHICKEN);
                        }
                        else if (which_consumable == (int)Consumable::HEARTY_STEW) {
                            this_consumable = createConsumableItem(renderer, collider_motion.position, TEXTURE_ASSET_ID::STEW, Consumable::HEARTY_STEW);
                        }
                        else if (which_consumable == (int)Consumable::LANDMINE) {
                            this_consumable = createConsumableItem(renderer, collider_motion.position, TEXTURE_ASSET_ID::LANDMINE, Consumable::LANDMINE);

                        }
                        else if (which_consumable == (int)Consumable::STAMINA_POTION) {
                            this_consumable = createConsumableItem(renderer, collider_motion.position, TEXTURE_ASSET_ID::POTION, Consumable::STAMINA_POTION);
                        }

                        consumableItems.push_back(this_consumable);
                    }

                    // update enemy counter
//                    if (enemiesLeft > 0){
//                        enemiesLeft -= 1;
//                    }
                    registry.remove_all_components_of(collider);
                    points += 1;
                    enemy_count--;

                    for (Entity enemyCounter : registry.enemyCounters.entities) {
                        if (registry.texts.has(enemyCounter)) {
                            IsText& it = registry.texts.get(enemyCounter);
                            it.text = std::string("Enemies left: ") + std::to_string(enemy_count);
                        }
                    }

                    printf("You killed an enemy!\n");
                }
            }

            // if enemy is colliding with an entity other than it's own bullets, try to move in opposite direction to not get stuck
            // !!! added a check here to ignore collisions with weapons (such that dropped weapons do not classify as stuck)
            if (registry.enemies.has(collider) && !registry.worldEffects.has(entity) && !registry.damaging.has(entity) && registry.following.has(collider) && registry.motions.has(collider) && !registry.weapons.has(entity)){
                Enemy &enemy = registry.enemies.get(collider);
                Motion &motion = registry.motions.get(collider);
                // if no path was specified, means that enemy is moving in general direction of player (calculated with angle)
                if (enemy.path.empty()){
                    if (motion.velocity != vec2(0,0)){
                        motion.velocity = -1.f*motion.velocity;

                        vec2 motion_dir = normalize(motion.velocity);
                        enemy.path.push_back(motion.position/(float) SPRITE_SIZE_STANDARD + motion_dir);
                    }
                }
            }

            // added check that ensures player bullets can't collide with the player
            if (registry.projectiles.has(entity)
            && !(registry.enemyDamaging.has(entity) && registry.players.has(collider))
            && !(registry.damaging.has(entity) && registry.enemies.has(collider))
            && !registry.projectiles.has(collider)
            && !(registry.fireBombs.has(entity) && registry.enemies.has(collider))
            && !(registry.blinders.has(entity) && registry.projectiles.has(entity) && registry.enemies.has(collider))
            && !(registry.fireBombs.has(entity) && registry.canMoveThroughs.has(collider))
            && !(registry.projectiles.has(entity) && registry.canMoveThroughs.has(collider))
            && !registry.worldEffects.has(collider)) {
                if (registry.fireBombs.has(entity)) {
                    Motion& bomb = registry.motions.get(entity);
                    createFireRing(renderer, bomb.position, 3);
                    Mix_PlayChannel(-1, explosion_sound, 0);

                }
                registry.remove_all_components_of(entity);
            }

            if (registry.players.has(collider) && registry.worldEffects.has(entity)) {
                Player &player_component = registry.players.get(player);
                WorldEffect &worldEffect = registry.worldEffects.get(entity);

                player_component.worldEffect = worldEffect.worldEffect;
                if (worldEffect.worldEffect == Effects::OASIS){
                    isWet = true;
                    if (isBasic && curr_state == TutorialState::DONE){
                        curr_state = TutorialState::WET;
                        handle_tutorial_state(true);
                    }
                } else {
                    isWet = false;
                    if (isBasic && curr_state == TutorialState::WET){
                        handle_tutorial_state(false);
                    }
                    if (registry.puddleTimers.has(player)){
                        registry.puddleTimers.remove(player);
                    }
                    if (registry.effectTimers.has(player)){
                        registry.effectTimers.remove(player);
                    }
                }
                if (worldEffect.worldEffect == Effects::OASIS && !registry.effectTimers.has(player)){
                    EffectTimer &et = registry.effectTimers.emplace(player);
                    if (!registry.puddleTimers.has(player)){
                        registry.puddleTimers.emplace(player);
                    } else {
                        PuddleTimer &pt = registry.puddleTimers.get(player);
                        pt.counter_ms = et.counter_ms;
                    }
                } else if (registry.effectTimers.has(player)){
                    EffectTimer &et = registry.effectTimers.get(player);
                    et.counter_ms = 3000;
                    if (!registry.puddleTimers.has(player)){
                        registry.puddleTimers.emplace(player);
                    } else {
                        PuddleTimer &pt = registry.puddleTimers.get(player);
                        pt.counter_ms = et.counter_ms;
                    }
                }
            }

            if (registry.enemies.has(collider) && registry.worldEffects.has(entity)) {
                Enemy &enemy = registry.enemies.get(collider);
                WorldEffect &worldEffect = registry.worldEffects.get(entity);

                enemy.worldEffect = worldEffect.worldEffect;

                if (worldEffect.worldEffect != Effects::OASIS){
                    if (registry.puddleTimers.has(player)){
                        registry.puddleTimers.remove(player);
                    }
                    if (registry.effectTimers.has(player)){
                        registry.effectTimers.remove(player);
                    }
                }
                if (worldEffect.worldEffect == Effects::OASIS && !registry.effectTimers.has(collider)){
                    EffectTimer &et = registry.effectTimers.emplace(collider);
                    if (!registry.puddleTimers.has(player)){
                        registry.puddleTimers.emplace(player);
                    } else {
                        PuddleTimer &pt = registry.puddleTimers.get(player);
                        pt.counter_ms = et.counter_ms;
                    }
                } else if (registry.effectTimers.has(collider)){
                    EffectTimer &et = registry.effectTimers.get(collider);
                    et.counter_ms = 3000;
                    if (!registry.puddleTimers.has(collider)){
                        registry.puddleTimers.emplace(collider);
                    } else {
                        PuddleTimer &pt = registry.puddleTimers.get(collider);
                        pt.counter_ms = et.counter_ms;
                    }
                }
            }

            // pickup weapon on the ground
            // checks if entity is a weapon, and isn't already equipped, the collider is the player and equipped weapons is not full
            if (registry.weapons.has(entity) && registry.players.has(collider) && !registry.equippedWeapons.has(entity) && (registry.equippedWeapons.size() < MAX_WEAPONS)) {
                // hides the rendering of the weapon
                // if (registry.renderRequests.has(entity)) {
                //     registry.renderRequests.remove(entity);
                // }
                // equips the weapon
                Mix_PlayChannel(-1, weapon_pick, 0);
                registry.equippedWeapons.emplace(entity);
                registry.renderGroups.remove(entity);
                registry.motions.get(entity).can_collide = false;
                if (isBasic && curr_state == TutorialState::PICK_UP_WEAPON){
                    handle_tutorial_state(false);
                }
            }

            // pick up passive item on the ground. create text that shows up on screen for 3 seconds
            // don't pick up passive items generated for display in the inventory screen
            if (registry.players.has(collider) && registry.passiveItems.has(entity) && registry.hasPassiveItems.has(collider) && !registry.inventoryImages.has(entity)) {

                if (isBasic && curr_state == TutorialState::DONE){
                    curr_state = TutorialState::PASSIVE_ITEM;
                    handle_tutorial_state(true);
                }
                HasPassiveItem& hpi = registry.hasPassiveItems.get(collider);
                PassiveItem pi = registry.passiveItems.get(entity);

                hpi.items_list.push_back(pi.id);

                std::string item_string = ITEM_NAME_MAP[pi.id] + ": " + ITEM_EFFECT_MAP[pi.id];
                Entity item_pickup_text = registry.screenStates.components[0].item_description_text;

                if (!registry.texts.has(item_pickup_text)){
                    item_pickup_text = createText(item_string, vec2(-window_width_px / 2.0f + 20.0f, 150.0f), vec3(0.0f, 0.0f, 0.0f), true);
                    ScreenState &screenState = registry.screenStates.components[0];
                    screenState.item_description_text = item_pickup_text;
                }
                IsText& itxt = registry.texts.get(item_pickup_text);
                itxt.text = item_string;

                if (!inventory_visible && !help_popup_visible) {
                    itxt.doRender = true;
                    if (!registry.passiveTextTimers.has(item_pickup_text)) {
                        registry.passiveTextTimers.emplace(item_pickup_text);
                    }
                    if (!registry.pickedUpItemTimers.has(collider)) {
                        registry.pickedUpItemTimers.emplace(collider);
                    }
                }

                // create inventory text and image at appropriate position
                Entity test_inventory_text = createText(item_string, vec2(INVENTORY_TEXT_XPOS, INVENTORY_TEXT_YPOS), vec3(0.0f, 0.0f, 0.0f), true);
                InventoryText& it = registry.inventoryTexts.emplace(test_inventory_text);
                it.index = hpi.items_list.size() - 1;
                it.is_title = false;

                Entity inventory_item = createInventoryImage(vec2(CURR_INVENTORY_XPOS + collider_motion.position.x, CURR_INVENTORY_YPOS + collider_motion.position.y), pi.tex);
                if (inventory_visible) {
                    InventoryImage& ii = registry.inventoryImages.get(inventory_item);
                    Motion& iim = registry.motions.get(inventory_item);
                    iim.scale = vec2(ii.scale_x, ii.scale_y);
                }


                CURR_INVENTORY_XPOS += SPRITE_SIZE_STANDARD;
                if (CURR_INVENTORY_XPOS > item_xpos_end) {
                    CURR_INVENTORY_XPOS = item_xpos_start;
                    CURR_INVENTORY_YPOS += SPRITE_SIZE_STANDARD;
                }

                // apply 'immediate' passive item effects

                if (pi.id == P_HEALTHUP) {
                    PLAYER_MAX_HEALTH += 30;
                    if (registry.healths.has(player)) {
                        Health& h = registry.healths.get(player);
                        h.val += 30;
                        editHealthBar(h.val, PLAYER_MAX_HEALTH);
                    }

                }
                else if (pi.id == P_STAMUP) {
                    PLAYER_MAX_STAMINA += 30;
                    if (registry.staminas.has(player)) {
                        Stamina& h = registry.staminas.get(player);
                        h.val += 30;
                        editStaminaBar(h.val, PLAYER_MAX_STAMINA);
                    }
                }
                else if (pi.id == P_GUNMOD) {
                    fire_delay = 250.0f;
                }
                else if (pi.id == P_PADDING) {
                    damage_multiplier -= 0.25f;
                    if (damage_multiplier < 0.25f) {
                        damage_multiplier = 0.25f;
                    }
                }
                else if (pi.id == P_DMGUP) {
                    attack_multiplier += 0.5f;
                    if (attack_multiplier > 4.0f) {
                        attack_multiplier = 4.0f;
                    }
                }

                // erase the rendered passive item
                registry.remove_all_components_of(entity);
            }

            // pick up consumable item on the ground. create text that shows up on screen for 3 seconds
            if (registry.players.has(collider) && registry.consumableItems.has(entity) && registry.consumableItems.get(entity).player_collidable ) {

                if (isBasic && curr_state == TutorialState::DONE){
                    curr_state = TutorialState::CONSUMABLE_ITEM;
                    handle_tutorial_state(true);
                }
                Player &p = registry.players.components[0];
                for (int i = 0; i < 4 ; i++){
                    Entity consumable = p.consumables[i];
                    ConsumableItem &ci = registry.consumableItems.get(entity);
                    if (!registry.consumableItems.has(consumable) && ci.player_collidable){

                        ConsumableItem &consumable = registry.consumableItems.get(entity);
                        std::string item_string = CONSUMABLE_NAME_MAP[(int) consumable.id] + ": " + CONSUMABLE_EFFECT_MAP[(int) consumable.id];
                        Entity item_pickup_text = registry.screenStates.components[0].item_description_text;
                        if (!registry.texts.has(item_pickup_text)){
                            item_pickup_text = createText(item_string, vec2(-window_width_px / 2.0f + 20.0f, 150.0f), vec3(0.0f, 0.0f, 0.0f), true);
                            ScreenState &screenState = registry.screenStates.components[0];
                            screenState.item_description_text = item_pickup_text;
                        }
                        IsText& itxt = registry.texts.get(item_pickup_text);
                        itxt.text = item_string;

                        if (!inventory_visible && !help_popup_visible) {
                            itxt.doRender = true;
                            if (!registry.passiveTextTimers.has(item_pickup_text)) {
                                registry.passiveTextTimers.emplace(item_pickup_text);
                            }
                            if (!registry.pickedUpItemTimers.has(collider)) {
                                registry.pickedUpItemTimers.emplace(collider);
                            }
                        }

                        p.consumables[i] = entity;
                        ConsumableItem &c = registry.consumableItems.get(entity);
                        vec2 loc = vec2(-window_width_px/2 + 40 + 75*i, -window_height_px/2 + 125) + collider_motion.position;
                        createConsumableIndicator(loc, entity, i+1);
                        break;
                    }
                }
            }



            // pick up passive item on the ground. create text that shows up on screen for 3 seconds
            //if (registry.enemies.has(collider) && registry.consumableItems.has(entity) && !registry.consumableItems.get(entity).player_collidable) {

            //    Entity explosion = Entity();
            //    Motion& motion = registry.motions.emplace(explosion);
            //    motion.position = registry.motions.get(entity).position;
            //    motion.angle = 0.0f;
            //    motion.velocity = { 0, 0 };
            //    motion.scale = { SPRITE_SIZE_STANDARD , SPRITE_SIZE_STANDARD };
            //    if (!registry.renderRequests.has(explosion)) {
            //        registry.renderRequests.insert(
            //                explosion, {
            //                        TEXTURE_ASSET_ID::EXPLOSION,
            //                        EFFECT_ASSET_ID::TEXTURED,
            //                        GEOMETRY_BUFFER_ID::SPRITE
            //                }
            //        );
            //        RenderGroup& rg = registry.renderGroups.emplace(explosion);
            //        rg.entities.push_back(explosion);
            //        rg.indices.push_back(0);
            //    }
            //    AnimationState& as = registry.animationStates.emplace(explosion);
            //    as.num_ms = 50.0f;
            //    as.counter_ms = as.num_ms;
            //    as.num_rows = 1;
            //    as.num_cols = 4;
            //    as.col = 0;
            //    as.row = 0;

            //    registry.animationTimers.emplace(explosion);
            //    Mix_PlayChannel(-1, explosion_sound, 0);

            //    registry.remove_all_components_of(entity);
            //    // maybe extend to deal aoe damage? check all enemies within range
            //}
        }


    }

    // Remove all collisions from this simulation step
    registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
    return bool(glfwWindowShouldClose(window));
}

void WorldSystem::reloadPlayerAmmo() {
    if (isBasic && curr_state == TutorialState::RELOAD){
        handle_tutorial_state(false);
    }
    Weapon& weapon_data = registry.weapons.get(registry.weaponHolders.get(player).weapon);
    if (weapon_data.cur_ammo == weapon_data.max_ammo || registry.reloadTimers.has(player)) return;
    Mix_PlayChannel(-1, reload_begin_sound, 0);

    // give player reload cooldown
    if (!registry.reloadTimers.has(player)) {
        ReloadTimer& rt = registry.reloadTimers.emplace(player);
        rt.counter_ms = WEAPON_RELOAD_TIME[weapon_data.type];
    }
    printf("Reloading...\n");

}

void WorldSystem::handle_consumable(Entity &entity){
    if (isBasic && curr_state == TutorialState::CONSUMABLE_ITEM){
        handle_tutorial_state(false);
    }

    if (!registry.consumableItems.has(entity)){
        std::cout << "Error, no consumable found for id " << entity << std::endl;
    }
    ConsumableItem &consumable = registry.consumableItems.get(entity);

    if (consumable.id == Consumable::CHICKEN) {
        Health &p_health = registry.healths.get(player);
        createParticleBurst(player, 15, vec2(30, 50), vec2(5, 10), {0, 1, 0, 1});        

        if (p_health.val + 10 > PLAYER_MAX_HEALTH) {
            p_health.val = PLAYER_MAX_HEALTH;
        } else {
            p_health.val += 10;
        }
        editHealthBar(p_health.val, PLAYER_MAX_HEALTH);
        registry.remove_all_components_of(entity);
    } else if (consumable.id == Consumable::LANDMINE && !consumable.placed){
        consumable.placed = true;
        std::cout <<"landmine" << std::endl;

        Entity landmine_copy = createConsumableItem(renderer, vec2(0.0f, 0.0f), TEXTURE_ASSET_ID::LANDMINE, Consumable::LANDMINE);
        ConsumableItem& consumable2 = registry.consumableItems.get(landmine_copy);

        consumable2.player_collidable = false;
        Motion &motion = registry.motions.get(landmine_copy);
        Motion player_motion = registry.motions.get(player);

        motion.position = player_motion.position;
        motion.angle = 0;
        motion.velocity = vec2(0,0);

        EnemyDamaging& ed = registry.enemyDamaging.emplace(landmine_copy);
        ed.val = 50;
        HasKnockback& kb = registry.knockbackers.emplace(landmine_copy);
        kb.is_enemy_affecting = true;
        kb.is_player_affecting = false;


        registry.remove_all_components_of(entity);

    } else if (consumable.id == Consumable::HEARTY_STEW){
        Health &p_health = registry.healths.get(player);
        p_health.val = PLAYER_MAX_HEALTH;
        editHealthBar(p_health.val, PLAYER_MAX_HEALTH);
        createParticleBurst(player, 30, vec2(30, 50), vec2(5, 10), {0, 1, 0, 1});        
        registry.remove_all_components_of(entity);
    } else if (consumable.id == Consumable::STAMINA_POTION){
        Stamina &p_stamina = registry.staminas.get(player);
        p_stamina.val = PLAYER_MAX_STAMINA;
        createParticleBurst(player, 30, vec2(30, 50), vec2(5, 10), {0, 1, 0, 1});
        registry.remove_all_components_of(entity);
    }
}


// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
    
    //// Exit Game
    //if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
    //    // TODO: Implement "Cancel"
    //    glfwSetWindowShouldClose(window, true);
    //    std::cout << "Exiting game" << std::endl;
    //}

    // ABOVE are key presses that are handled when game is in any state
    // BELOW are key presses that are handled when game is not in title screen
    if (!game_in_title_screen) {

            // Resetting game
        if (action == GLFW_RELEASE && key == GLFW_KEY_F5) {
            int w, h;
            glfwGetWindowSize(window, &w, &h);
            isSecond = false;
            isThird = false;
            isBasic = true;

            if (registry.levelStates.components.size() > 0) {
                LevelState& ls = registry.levelStates.components[0];
                ls.whichLevel = 0;
            }

            restart_game(false);
        }


	// press 8 to skip to level 2 
    if (action == GLFW_PRESS && key == GLFW_KEY_8 && !inventory_visible) {
        isSecond = true;
        isThird = false;
        isBasic = false;

        if (registry.levelStates.components.size() > 0) {
            LevelState& ls = registry.levelStates.components[0];
            ls.whichLevel = 2;
        }

        restart_game(false);
    }

    if (action == GLFW_PRESS && key == GLFW_KEY_9 && !inventory_visible) {
        isThird = true;
        isSecond = false;
        isBasic = false;

        if (registry.levelStates.components.size() > 0) {
            LevelState& ls = registry.levelStates.components[0];
            ls.whichLevel = 3;
        }

        restart_game(false);
    }

    //if press 0
    if (action == GLFW_PRESS && key == GLFW_KEY_0) {
        enemy_count--;
    }

        // Save game
        if (action == GLFW_RELEASE && key == GLFW_KEY_F3) {
            json saveData = registry.toJSON();
            std::ofstream file(save_file_path());
            file << saveData.dump();
            // if you want a human readable dump, at the cost of extra space
            // file << saveData.dump(4);
            file.close();
            std::cout << "Game Saved !" << std::endl;
        }

        // Load game
        if (action == GLFW_RELEASE && key == GLFW_KEY_F4) {
            load_game();
        }

        if (!registry.deathTimers.has(player)) {

            // Basic movements
            if ((action == GLFW_PRESS) && isMovementKey(key)) {
                if (isBasic && curr_state == TutorialState::WASD) {
                    handle_tutorial_state(false);
                }
                handle_movement_key_press(key, registry.motions.get(player).velocity,
                                          registry.motions.get(player).max_velocity);
            } else if (action == GLFW_RELEASE && isMovementKey(key)) {
                handle_movement_key_release(key, registry.motions.get(player).velocity,
                                            registry.motions.get(player).max_velocity);
            }


            Player &player_component = registry.players.components[0];
            if (action == GLFW_PRESS && key == GLFW_KEY_1) {
                if (registry.consumableItems.has(player_component.consumables[0])) {
                    // handle
                    handle_consumable(player_component.consumables[0]);
                }
            }
            if (action == GLFW_PRESS && key == GLFW_KEY_2) {
                if (registry.consumableItems.has(player_component.consumables[1])) {
                    // handle
                    handle_consumable(player_component.consumables[1]);
                }
            }
            if (action == GLFW_PRESS && key == GLFW_KEY_3) {
                if (registry.consumableItems.has(player_component.consumables[2])) {
                    // handle
                    handle_consumable(player_component.consumables[2]);
                }
            }
            if (action == GLFW_PRESS && key == GLFW_KEY_4) {
                if (registry.consumableItems.has(player_component.consumables[3])) {
                    // handle
                    handle_consumable(player_component.consumables[3]);
                }
            }
            if (action == GLFW_PRESS && key == GLFW_KEY_R) {
                reloadPlayerAmmo();
            }

            if (action == GLFW_PRESS && key == GLFW_KEY_L && !inventory_visible) {
                if (isBasic) {
                    isBasic = false;
                    isSecond = false;
                    isThird = false;

                    if (registry.levelStates.components.size() > 0) {
                        LevelState& ls = registry.levelStates.components[0];
                        ls.whichLevel = 1;
                    }

                } else {
                    isBasic = true;
                    isSecond = false;
                    isThird = false;

                    if (registry.levelStates.components.size() > 0) {
                        LevelState& ls = registry.levelStates.components[0];
                        ls.whichLevel = 0;
                    }

                }
                restart_game(false);
            }

        }


            if (action == GLFW_PRESS && key == GLFW_KEY_M) {
                // TODO: Implement open map
            }

            if (action == GLFW_PRESS && (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)) {
                if (!registry.deathTimers.has(player) && !registry.dashCooldownTimers.has(player) &&
                    (registry.motions.get(player).max_velocity != vec2(0, 0))) {
                    if (isBasic && curr_state == TutorialState::DASH){
                        handle_tutorial_state(false);
                    }
                    // edit length of stamina bar
                    Mix_PlayChannel(-1, dash_sound, 0);
                    if (registry.staminas.has(player)) {
                        Stamina &s = registry.staminas.get(player);
                        if (s.val <= 0.0) {
                            s.val = 0.0;
                        } else {
                            registry.dashCooldownTimers.emplace(player);
                            s.val -= 10.0;
                        }
                        editStaminaBar(s.val, PLAYER_MAX_STAMINA);
                    }
                }
            }


            if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE && !help_popup_visible) {
                if (isBasic && curr_state == TutorialState::PASSIVE_ITEM) {
                    handle_tutorial_state(false);
                }
                inventory_visible = !inventory_visible;
                if (!inventory_visible) {
                    if (isBasic) {
                        IsText &isText = registry.texts.get(registry.tutorialTexts.entities[0]);
                        isText.text = TUTORIAL_TEXT_MAP[(int) curr_state];
                    }
                    for (Entity e: registry.inventoryBGs.entities) {
                        if (registry.motions.has(e)) {
                            Motion &m = registry.motions.get(e);
                            m.scale.x = 0;
                            m.scale.y = 0;

                        }

                        for (Entity e: registry.inventoryImages.entities) {
                            if (registry.motions.has(e)) {
                                Motion &m = registry.motions.get(e);
                                m.scale = vec2(0.0f, 0.0f);
                            }
                        }
                    }
                } else {
                    if (isBasic) {
//                        ScreenState &screenState = registry.screenStates.components[0];
                        IsText &isText = registry.texts.get(registry.tutorialTexts.entities[0]);
                        isText.text = "";
                    }
                    for (Entity e: registry.inventoryBGs.entities) {
                        InventoryBG hpb = registry.inventoryBGs.get(e);
                        if (registry.motions.has(e)) {
                            Motion &m = registry.motions.get(e);
                            m.scale.x = hpb.scale_x;
                            m.scale.y = hpb.scale_y;
                        }

                        for (Entity e: registry.inventoryImages.entities) {
                            if (registry.motions.has(e)) {
                                Motion &m = registry.motions.get(e);
                                InventoryImage &ii = registry.inventoryImages.get(e);
                                m.scale = vec2(ii.scale_x, ii.scale_y);
                            }
                        }

                        // remove any existing pickup text

                        for (Entity e: registry.passiveTextTimers.entities) {
                            registry.remove_all_components_of(e);
                        }

                        for (Entity e: registry.pickedUpItemTimers.entities) {
                            registry.pickedUpItemTimers.remove(e);
                        }
                    }
                }
                game_paused = !game_paused;
            }


                if (action == GLFW_PRESS && key == GLFW_KEY_M) {
                    // TODO: Implement interact with npc, confirm, advance dialog
                }

                if (action == GLFW_PRESS && key == GLFW_KEY_TAB) {
                    // TODO: Implement open inventory
                }

                // open or close help popup
                if (action == GLFW_PRESS && key == GLFW_KEY_H && !inventory_visible) {
                    help_popup_visible = !help_popup_visible;

                    if (!help_popup_visible) {
                        if (isBasic) {
                            IsText &isText = registry.texts.get(registry.tutorialTexts.entities[0]);
                            isText.text = TUTORIAL_TEXT_MAP[(int) curr_state];
                            isText.doRender = true;
                        }
                        for (Entity e: registry.helpPopupBGs.entities) {
                            if (registry.motions.has(e)) {
                                Motion &m = registry.motions.get(e);
                                m.scale.x = 0;
                                m.scale.y = 0;
                            }
                        }
                    } else {
                        if (isBasic) {
                            ScreenState &screenState = registry.screenStates.components[0];
                            IsText &isText = registry.texts.get(registry.tutorialTexts.entities[0]);
                            isText.text = "";
                            isText.doRender = false;
                        }
                        help_popup_page = 0;
                        for (Entity e: registry.helpPopupBGs.entities) {
                            HelpPopupBG hpb = registry.helpPopupBGs.get(e);
                            if (registry.motions.has(e)) {
                                Motion &m = registry.motions.get(e);
                                m.scale.x = hpb.scale_x;
                                m.scale.y = hpb.scale_y;
                            }
                        }
                    }
                }

            // change to prev weapon
            if (action == GLFW_PRESS && key == GLFW_KEY_Q) {
                handle_change_weapon(false);
            }

            // change to next weapon
            if (action == GLFW_PRESS && key == GLFW_KEY_E) {
                handle_change_weapon(true);
            }

                    // pause and unpause game, show / hide relevant text
                    /*if (action == GLFW_PRESS && key == GLFW_KEY_P) {
                        if (!game_paused) {
                            for (Entity& p : registry.pauseTexts.entities) {
                                if (registry.texts.has(p)) {
                                    IsText& pt = registry.texts.get(p);
                                    pt.doRender = true;
                                }
                            }
                        }
                        else {
                            for (Entity& p : registry.pauseTexts.entities) {
                                if (registry.texts.has(p)) {
                                    IsText& pt = registry.texts.get(p);
                                    pt.doRender = false;
                                }
                            }
                        }
                        game_paused = !game_paused;
                    }*/

                    if (action == GLFW_PRESS && key == GLFW_KEY_RIGHT && help_popup_visible) {
                        help_popup_page += 1;
                        if (help_popup_page >= help_popup_num_pages) {
                            help_popup_page = help_popup_num_pages - 1;
                        }
                    }

                    if (action == GLFW_PRESS && key == GLFW_KEY_LEFT && help_popup_visible) {
                        help_popup_page -= 1;
                        if (help_popup_page < 0) {
                            help_popup_page = 0;
                        }
                    }
                }


                if (action == GLFW_PRESS && key == GLFW_KEY_F) {
                    fps_visible = true;
                }
                if (action == GLFW_RELEASE && key == GLFW_KEY_F) {
                    fps_visible = false;
                }


                // Zoom in and out with the up/down keys
                if (action == GLFW_PRESS && key == GLFW_KEY_UP) {
                    Player &player_data = registry.players.get(player);
                    if (player_data.camera_scale.x < MAX_ZOOM) {
                        player_data.camera_scale *= 2.0f;
                        printf("Zoomed in to %.2fx\n", player_data.camera_scale.x);
                    }
                }

                if (action == GLFW_PRESS && key == GLFW_KEY_DOWN) {
                    Player &player_data = registry.players.get(player);
                    if (player_data.camera_scale.x > MIN_ZOOM) {
                        player_data.camera_scale /= 2.0f;
                        printf("Zoomed out to %.2fx\n", player_data.camera_scale.x);
                    }
                }

                if (action == GLFW_PRESS && key == GLFW_KEY_RIGHT) {
                    if (inventory_visible) {
                        HasPassiveItem hpi = registry.hasPassiveItems.get(player);
                        if (hpi.items_list.size() != 0) {
                            inventory_index += 1;

                            if (inventory_index >= hpi.items_list.size()) {
                                inventory_index = hpi.items_list.size() - 1;
                            }

                            int which_row = inventory_index / items_per_row;
                            int which_column = inventory_index - (items_per_row * which_row);

                            for (Entity e: registry.inventoryBorders.entities) {
                                if (registry.motions.has(e)) {
                                    Motion &m = registry.motions.get(e);
                                    Motion pm = registry.motions.get(player);
                                    m.position = vec2(
                                            item_xpos_start + (SPRITE_SIZE_STANDARD * which_column) + pm.position.x,
                                            item_ypos_start + (SPRITE_SIZE_STANDARD * which_row) + pm.position.y);
                                }
                            }
                        }
                    }
                }

                if (action == GLFW_PRESS && key == GLFW_KEY_LEFT) {
                    if (inventory_visible) {
                        inventory_index -= 1;
                        if (inventory_index < 0) {
                            inventory_index = 0;
                        }

                        int which_row = inventory_index / items_per_row;
                        int which_column = inventory_index - (items_per_row * which_row);

                        for (Entity e: registry.inventoryBorders.entities) {
                            if (registry.motions.has(e)) {
                                Motion &m = registry.motions.get(e);
                                Motion pm = registry.motions.get(player);
                                m.position = vec2(
                                        item_xpos_start + (SPRITE_SIZE_STANDARD * which_column) + pm.position.x,
                                        item_ypos_start + (SPRITE_SIZE_STANDARD * which_row) + pm.position.y);
                            }
                        }
                    }
                }


                // Debugging
                if (key == GLFW_KEY_SLASH) {
                    if (action == GLFW_RELEASE)
                        debugging.in_debug_mode = false;
                    else
                        debugging.in_debug_mode = true;
                }

                // Control the current speed with `<` `>`
                if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
                    current_speed -= 0.1f;
                    printf("Current speed = %f\n", current_speed);
                }
                if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
                    current_speed += 0.1f;
                    printf("Current speed = %f\n", current_speed);
                }
                current_speed = fmax(0.f, current_speed);
}



bool in_box(vec2 AA, vec2 BB, vec2 mouse_pos) {
    return mouse_pos.x >= AA.x &&
        mouse_pos.y >= AA.y &&
        mouse_pos.x <= BB.x &&
        mouse_pos.y <= BB.y;
}

void WorldSystem::on_mouse_click(int button, int action, int mod) {        

    // BELOW are mouse click events handled while in normal game state (not in title screen or paused)
    if (!game_in_title_screen && !game_paused) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
          if (isBasic && curr_state == TutorialState::SHOOT){
            handle_tutorial_state(false);
         }
            if (registry.weaponHolders.has(player) && registry.motions.has(player) && !registry.attackTimers.has(player) && !registry.reloadTimers.has(player)) {
                Entity& weapon = registry.weaponHolders.get(player).weapon;
                Weapon& weapon_data = registry.weapons.get(weapon);

                if (weapon_data.cur_ammo > 0) {

                    // TODO (since I don't know how this will interfere with mouse aiming PR): 
                    // spawn the player's bullet just outside of the hitbox to avoid collision issues!
                    // added a temp fix for this in the step function where collision resolution is done

                    Player& p = registry.players.get(player);
                    Motion& weapon_motion = registry.motions.get(weapon);

                    vec2 bullet_position = weapon_motion.position;

                    handle_fire(bullet_position, p.angle);
                    Mix_PlayChannel(-1, pistol_shot_sound, 0);
                    weapon_data.cur_ammo--;

                    // put attack cooldown timer on player
                    if (!registry.attackTimers.has(player)) {
                        AttackTimer& att = registry.attackTimers.emplace(player);
                        att.counter_ms = fire_delay;
                    }

                    // set ith ammo counter to be black
                    std::vector<Entity> ammoCounters = registry.ammoCounters.entities;
                    if (ammoCounters.size() >= weapon_data.max_ammo) {
                        Entity thisCounter = ammoCounters[weapon_data.cur_ammo];
                        if (registry.colors.has(thisCounter)) {
                            vec4& color = registry.colors.get(thisCounter);
                            color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
                        }
                    }

                    printf("Ammo: %d / %d\n", weapon_data.cur_ammo, weapon_data.max_ammo);
                }
                else {
                    Mix_PlayChannel(-1, error_sound, 0);
                    printf("Oh no! You're out of ammo! Right-click or press R to reload.\n");
                }
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            reloadPlayerAmmo();
        }
    }

    // BELOW for mouse clicks in title screen
    else if (game_in_title_screen) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            vec2 mouse_pos = { xpos, ypos };
            Entity button_new_game = registry.titleScreenButtons.entities[0];
            Motion& motion = registry.motions.get(button_new_game);
            Entity button_resume_game = registry.titleScreenButtons.entities[1];
            Motion& motion2 = registry.motions.get(button_resume_game);
            Player& p = registry.players.get(player);

            // this calculates the top left corner of the button
            vec2 origin = vec2(p.camera_pos.x - window_width_px / 2.0f,
                p.camera_pos.y - window_height_px / 2.0f);
            int x0 = motion.position.x - origin.x;
            int y0 = motion.position.y - origin.y;
            int x1 = motion2.position.x - origin.x;
            int y1 = motion2.position.y - origin.y;

            // in new game button
            if (in_box(vec2(x0, y0), vec2(x0 + BUTTON_WIDTH, y0 + BUTTON_HEIGHT), mouse_pos)) {
                for (auto& c : registry.titleScreens.components) {
                    c.doRender = false;
                }
                for (Entity e : registry.titleScreenButtons.entities) {
                    if (registry.motions.has(e)) {
                        Motion& m = registry.motions.get(e);
                        m.scale.x = 0;
                        m.scale.y = 0;
                    }
                }
                for (Entity& e : registry.texts.entities) {
                    if (registry.titleScreenButtonTexts.has(e)) {
                        IsText& it = registry.texts.get(e);
                        it.doRender = false;
                    }
                }
                game_in_title_screen = false;
                restart_game(false);
            }
            // in resume game button
            if (in_box(vec2(x1, y1), vec2(x1 + BUTTON_WIDTH, y1 + BUTTON_HEIGHT), mouse_pos)) {
                // try loading
                std::ifstream file(save_file_path());
                if (!file.is_open()) {
                    printf("ERROR: Failed to read save file: %s\n", save_file_path().c_str());
                }
                else {
                    for (auto& c : registry.titleScreens.components) {
                        c.doRender = false;
                    }
                    for (Entity e : registry.titleScreenButtons.entities) {
                        if (registry.motions.has(e)) {
                            Motion& m = registry.motions.get(e);
                            m.scale.x = 0;
                            m.scale.y = 0;
                        }
                    }
                    for (Entity& e : registry.texts.entities) {
                        if (registry.titleScreenButtonTexts.has(e)) {
                            IsText& it = registry.texts.get(e);
                            it.doRender = false;
                        }
                    }
                    game_in_title_screen = false;
                    load_game();
                }
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
            // 
        }
    }
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
    // BELOW handles mouse moves while in normal running game state
    if (!game_in_title_screen && !game_paused) {

        Motion& motion = registry.motions.get(player);
        Player& p = registry.players.get(player);

        // convert the player position from world coordinates to screen coordinates,
        // using the camera position in world coordinates as our reference point
        vec2 origin = vec2(p.camera_pos.x - window_width_px / 2.0f,
            p.camera_pos.y - window_height_px / 2.0f);
        int x0 = motion.position.x - origin.x;
        int y0 = motion.position.y - origin.y;

        // update player angle for aiming
        p.angle = atan2((mouse_position.y - y0), ((mouse_position.x - x0)));

    }
    // BELOW handles mouse moves while in title screen state
    else if (game_in_title_screen) {
        Player& p = registry.players.get(player);
        for (Entity button : registry.titleScreenButtons.entities) {
            //Entity button_new_game = registry.titleScreenButtons.entities[0];
            Motion& motion = registry.motions.get(button);

            // this calculates the top left corner of the button
            vec2 origin = vec2(p.camera_pos.x - window_width_px / 2.0f,
                p.camera_pos.y - window_height_px / 2.0f);
            int x0 = motion.position.x - origin.x;
            int y0 = motion.position.y - origin.y;
            // highlight button
            if (in_box(vec2(x0, y0), vec2(x0 + BUTTON_WIDTH, y0 + BUTTON_HEIGHT), mouse_position)) {

                if (registry.colors.has(button)) {
                    vec4& col = registry.colors.get(button);
                    col = vec4(0.4f, 0.4f, 0.4f, 1.0f);
                }
                //std::cout << mouse_position.x << ", " << mouse_position.y << std::endl;
            }
            else {
                if (registry.colors.has(button)) {
                    vec4& col = registry.colors.get(button);
                    col = vec4(0.6, 0.6, 0.6, 1.0);
                }
            }
            //std::cout << mouse_position.x << ", " << mouse_position.y << std::endl;
            //std::cout << x0 << ", " << y0 << std::endl;
        }
    }
}

// input is x and y offset: provides offsets along Y-axis when scrolled (1 if scrolled up, -1 if scrolled down)
void WorldSystem::on_scroll_wheel(vec2 offset) {
    // BELOW handles scroll wheel while in normal running game state
    if (!game_in_title_screen && !game_paused) {
        if (offset.y > 0) {
            handle_change_weapon(false);
        }
        else if (offset.y < 0) {
            handle_change_weapon(true);
        }
    }
}

// input bool is true if changing to next weapon, false if changing to prev weapon
void WorldSystem::handle_change_weapon(bool is_next) {
    if (isBasic && curr_state == TutorialState::SWITCH_WEAPON){
        handle_tutorial_state(false);
    }

    auto &equippedWeaponsRegistry = registry.equippedWeapons;
    RenderGroup& rg =  registry.renderGroups.get(player);
    if (equippedWeaponsRegistry.size() == 1) {
        return;
    }

    Mix_PlayChannel(-1, weapon_switch, 0);

    for (uint i = 0; i < equippedWeaponsRegistry.entities.size(); i++) {
        uint num_weapons = equippedWeaponsRegistry.entities.size();
        Entity entity = equippedWeaponsRegistry.entities[i];
        Weapon& weaponComponent = registry.weapons.get(entity);
        if (weaponComponent.being_held) {
            // this weapon is currently being held
            // make this weapon not active, remove rendering component
            weaponComponent.being_held = false;

            // initiate prev and next weapons
            Entity prev;
            Entity next;

            // check if i = 0 or i = size - 1 and set according prev and next weapons
            if (i == 0) {
                prev = equippedWeaponsRegistry.entities[num_weapons - 1];
                next = equippedWeaponsRegistry.entities[i + 1];
            }
            else if (i == num_weapons - 1) {
                prev = equippedWeaponsRegistry.entities[i - 1];
                next = equippedWeaponsRegistry.entities[0];
            }
            else {
                prev = equippedWeaponsRegistry.entities[i - 1];
                next = equippedWeaponsRegistry.entities[i + 1];
            }

            // set next/prev weapon as active being held and render
            if (is_next) {
                registry.weapons.get(next).being_held = true;
                registry.weaponHolders.get(player).weapon = next;
                registry.motions.get(next).position = registry.motions.get(entity).position;
                registry.motions.get(next).scale.x = registry.motions.get(entity).scale.x;
                rg.entities.back() = next;
            }
            else {
                registry.weapons.get(prev).being_held = true;
                registry.weaponHolders.get(player).weapon = prev;
                registry.motions.get(prev).position = registry.motions.get(entity).position;
                registry.motions.get(prev).scale.x = registry.motions.get(entity).scale.x;
                rg.entities.back() = prev;
            }

            // update max ammo and update ammo ui
            // updateMaxAmmo();
            updateAmmoUI();

            // remove reload timer for the player if it has one
            if (registry.reloadTimers.has(player)) {
                registry.reloadTimers.remove(player);
            }

            // print weapon id
            std::cout << "current active weapon id = " << registry.weaponHolders.get(player).weapon << std::endl;

            for (Entity wi : registry.weaponIndicators.entities) {
                // change sprite of weapon indicator to player's currently held weapon
                WeaponIndicator& weap = registry.weaponIndicators.get(wi);
                if (registry.weaponHolders.has(player)) {
                    Entity p_weapon = registry.weaponHolders.get(player).weapon;
                    Weapon& w = registry.weapons.get(p_weapon);
                    weap.tex = w.tex;
                    if (registry.renderRequests.has(wi)) {
                        registry.renderRequests.remove(wi);
                    }
                    if (registry.renderGroups.has(wi)) {
                        registry.renderGroups.remove(wi);
                    }
                }
            }

            return;
        }
    }
}

bool WorldSystem::HasBeenHitByExplosion(Explosion& explosion, Entity& e)
{
    for (auto& it : explosion.entitiesHit) {
        if (*it == e) {
            return true;
        }
    }
    return false;
}


// Updates the ammoUI based on the max ammo of weapon held
void WorldSystem::updateAmmoUI() {
    // delete components from ammo counter registry
    for (int i = (int)registry.ammoCounters.components.size() - 1; i >= 0; --i) {
        registry.remove_all_components_of(registry.ammoCounters.entities[i]);
    }

    registry.ammoCounters.clear();
    int max_ammo = 0;
    int cur_ammo = 0;
    float ammo_counter_y_pos = -window_height_px / 2.0f + 10.0f + (BAR_HEIGHT_STANDARD * 2) + 20.0f;
    float ammo_counter_initial_x_pos = -window_width_px / 2.0f + 100.0f;
    vec2 player_pos = registry.motions.get(player).position;

    if (registry.weaponHolders.has(player)) {
        Weapon& weapon_data = registry.weapons.get(registry.weaponHolders.get(player).weapon);
        max_ammo = weapon_data.max_ammo;
        cur_ammo = weapon_data.cur_ammo;
    }

    for (int i = 0; i < cur_ammo; i++) {
        // NOTE: position of ammo counter translated by player position
        Entity counter = createAmmoCounter(vec2(ammo_counter_initial_x_pos + player_pos.x, ammo_counter_y_pos + player_pos.y), vec4(1.0f, 1.0f, 1.0f, 1.0f));
        ammo_counter_initial_x_pos += SPRITE_SIZE_AMMO_COUNTER + 5.0f;
    }

    for (int i = 0; i < max_ammo-cur_ammo; i++) {
        Entity counter = createAmmoCounter(vec2(ammo_counter_initial_x_pos + player_pos.x, ammo_counter_y_pos + player_pos.y), vec4(1.0f, 1.0f, 1.0f, 1.0f));
        registry.colors.get(counter) = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        ammo_counter_initial_x_pos += SPRITE_SIZE_AMMO_COUNTER + 5.0f;
    }

    // delete components from reloadindicators registry
    for (int i = (int)registry.reloadIndicators.components.size() - 1; i >= 0; --i) {
        registry.remove_all_components_of(registry.reloadIndicators.entities[i]);
    }

    // create reload indicator
    // NOTE: position of reload indicator translated by player position
    registry.reloadIndicators.clear();
    createReloadIndicator(vec2(-window_width_px / 2.0f + 100.0f + player_pos.x + (SPRITE_SIZE_AMMO_COUNTER + 5.0f) * max_ammo, -window_height_px / 2.0f + 10.0f + (BAR_HEIGHT_STANDARD * 2) + 20.0f + player_pos.y));
}

float normalizeAngle(float angle) {
    float two_pi = 2.0f * M_PI;
    float new_angle = fmod(angle, two_pi);
    if (new_angle >= M_PI) {
        new_angle -= two_pi;
    }
    else if (new_angle < -M_PI) {
        new_angle += two_pi;
    }
    return new_angle;
}

// creates bullets based on weapon type
void WorldSystem::handle_fire(vec2 bullet_position, float angle) {
    Entity weapon = registry.weaponHolders.get(player).weapon;
    WEAPON_TYPE type = registry.weapons.get(weapon).type;

    switch (type) {
    case WEAPON_TYPE::REVOLVER:
        createBullet(renderer, bullet_position, 500, angle, true, 10);
        break;
    case WEAPON_TYPE::RIFLE:
        createBullet(renderer, bullet_position, 1000, angle, true, 50);
        break;
    case WEAPON_TYPE::SHOTGUN:
        float offset1 = 0.50f;
        float offset2 = 1.0f;
        float angle1 = normalizeAngle(angle - offset1);
        float angle2 = normalizeAngle(angle - (offset1 / 2));
        float angle3 = normalizeAngle(angle + (offset1 / 2));
        float angle4 = normalizeAngle(angle + offset1);

        std::vector<Entity> shotgunBullets;
        shotgunBullets.push_back(createBullet(renderer, bullet_position, 500, angle1, true, 8));
        shotgunBullets.push_back(createBullet(renderer, bullet_position, 500, angle2, true, 8));
        shotgunBullets.push_back(createBullet(renderer, bullet_position, 500, angle, true, 8));
        shotgunBullets.push_back(createBullet(renderer, bullet_position, 500, angle3, true, 8));
        shotgunBullets.push_back(createBullet(renderer, bullet_position, 500, angle4, true, 8));

        for (Entity e : shotgunBullets) {
            LifeTimer& lt = registry.lifeTimers.emplace(e);
            lt.counter_ms = 400.0f;
        }

        break;
    }
}

bool WorldSystem::isMovementKey(int key) {
    return key == GLFW_KEY_W || key == GLFW_KEY_S || key == GLFW_KEY_A || key == GLFW_KEY_D;
}

void WorldSystem::handle_tutorial_state(bool keep_curr){
    if (isBasic){
//        ScreenState &screenState = registry.screenStates.components[0];
        Entity instruction_text = registry.tutorialTexts.entities[0];
        if (!registry.texts.has(instruction_text) || registry.tutorialTexts.entities.size() < 1){
            registry.tutorialTexts.clear();
            Entity instructionText = createText("", vec2(-window_width_px / 2.0f + 20.0f, 100.0f), vec3(0.0f, 0.0f, 0.0f), true);
            registry.tutorialTexts.emplace(instructionText);
        }
        if (!registry.texts.has(instruction_text)){
            IsText& eText = registry.texts.emplace(instruction_text);
        }
        IsText& eText = registry.texts.get(instruction_text);
//        eText.doRender = true;

        if (!keep_curr){
            if ((int) curr_state < 6){
                curr_state = static_cast<TutorialState>((int)curr_state+1);
            } else {
                curr_state = TutorialState::DONE;
            }
        }

        registry.tutorialTexts.components[0].state = curr_state;

        std::cout << "curr_state" << std::to_string((int) curr_state) << std::endl;
        eText.text = TUTORIAL_TEXT_MAP[(int)curr_state];
    }
}


void WorldSystem::handle_movement_key_press(int key, vec2 &velocity, vec2 &max_velocity) {
    if (!registry.knockbackTimers.has(player) && !registry.deathTimers.has(player) && !registry.dashCooldownTimers.has(player)) {

        velocity /= DEFAULT_SPEED;

        if (velocity.x != 0) {
            velocity.x /= abs(velocity.x);
        }
        if (velocity.y != 0) {
            velocity.y /= abs(velocity.y);
        }


        switch (key) {
        case GLFW_KEY_W:
            velocity.y = -1;
            break;
        case GLFW_KEY_S:
            velocity.y = 1;
            break;
        case GLFW_KEY_A:
            velocity.x = -1;
            break;
        case GLFW_KEY_D:
            velocity.x = 1;
            break;
        }

        if (velocity != vec2(0, 0)) {
            // normalize so that it is still moving at the correct speed
            velocity = normalize(velocity);
            max_velocity = velocity * DASH_SPEED;
            velocity *= DEFAULT_SPEED;
        }
        else {
            max_velocity = vec2(0, 0);
        }

        Player &player_component = registry.players.get(player);

        if (player_component.worldEffect == Effects::MUD){
            velocity *= 0.5;
        }

    }
    
}


void WorldSystem::handle_movement_key_release(int key, vec2 &velocity, vec2 &max_velocity) {
    if (!registry.knockbackTimers.has(player) && !registry.dashCooldownTimers.has(player)) {

        // remove effects of normalization
        velocity /= DEFAULT_SPEED;

        if (velocity.x!= 0) {
            velocity.x /= abs(velocity.x);
        }
        if (velocity.y != 0) {
            velocity.y /= abs(velocity.y);
        }

        switch (key) {
        case GLFW_KEY_W:
            velocity.y = 0;
            break;
        case GLFW_KEY_S:
            velocity.y = 0;
            break;
        case GLFW_KEY_A:
            velocity.x = 0;
            break;
        case GLFW_KEY_D:
            velocity.x = 0;
            break;
        }

        if (velocity != vec2(0, 0)) {
            velocity = normalize(velocity);
            max_velocity = velocity * DASH_SPEED;
            velocity *= DEFAULT_SPEED;
        }
        else {
            max_velocity = vec2(0, 0);
        }

    }

}

void WorldSystem::step_animation(Entity& entity, float elapsed_ms_since_last_update)
{
    float angle;
    if (registry.players.has(entity)) {
        angle = registry.players.get(entity).angle;
    } else if (registry.enemies.has(entity)) {
        angle = registry.enemies.get(entity).angle;
    } else {
        AnimationState& as = registry.animationStates.get(entity);
        if (as.counter_ms <= 0) {
            as.counter_ms = as.num_ms;
            // advance to next keyframe
            as.col = (as.col + 1) % as.num_cols;
        } else {
            as.counter_ms -= elapsed_ms_since_last_update;
        }
        return;
    }

    AnimationState& as = registry.animationStates.get(entity);
    Motion& motion = registry.motions.get(entity);

    // do not animate player/enemy if they are standing still
    if (motion.velocity.x == 0 && motion.velocity.y == 0) {
        as.counter_ms = 0;
        as.col = 0;
    } else if (as.counter_ms <= 0) {
        as.counter_ms = as.num_ms;
        // advance to next keyframe
        as.col = (as.col + 1) % as.num_cols;
    } else {
        as.counter_ms -= elapsed_ms_since_last_update;
    }

    // change the direction that the sprite should face
    if (angle < radians(-90.0f) || angle > radians(90.0f)) {
        motion.scale.x = -abs(motion.scale.x);
    } else {
        motion.scale.x = abs(motion.scale.x);
    }
}