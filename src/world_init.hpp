#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include "rng.hpp"

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
// enemies
Entity createEnemy(RenderSystem* renderer, vec2 pos, EnemyType type = EnemyType::DEFAULT);
// obstacles
Entity createObstacle(RenderSystem* renderer, vec2 pos, bool is_obstacle, bool damaging, bool knocks_back_player, bool knocks_back_enemy, TEXTURE_ASSET_ID tex);
Entity createCactus(RenderSystem* renderer, vec2 pos);
Entity createMeshBasedCactus(RenderSystem* renderer, vec2 pos);
Entity createBarrel(RenderSystem* renderer, vec2 pos);
Entity createFence(RenderSystem* renderer, vec2 pos);
Entity createFenceWall(RenderSystem* renderer, vec2 pos, vec2 tiling_factor);
Entity createFire(RenderSystem* renderer, vec2 pos);
Entity createFireRing(RenderSystem* renderer, vec2 pos, int size);
Entity createThorny(RenderSystem* renderer, vec2 pos);
Entity createBush(RenderSystem* renderer, vec2 pos);
Entity createStump(RenderSystem* renderer, vec2 pos);

Entity createBullet(RenderSystem* renderer, vec2 pos, float speed, float angle, bool enemyDamaging, int damage);
Entity createFireBomb(RenderSystem* renderer, vec2 pos, float speed, float angle);
Entity createDirtBall(RenderSystem* renderer, vec2 pos, float speed, float angle);

Entity createWeapon(RenderSystem* renderer, vec2 pos, bool dropped, TEXTURE_ASSET_ID tex, WEAPON_TYPE type);
Entity createBackground(RenderSystem* renderer);
Entity createTitleScreen(RenderSystem* renderer);
Entity createBloodSplotch(RenderSystem* renderer, vec2 pos);
Entity createPuddle(RenderSystem* renderer, vec2 pos);


Entity createParticle(Entity parent, vec2 start, vec2 dist, vec2 scale, float angle, vec4 color);
void createParticleBurst(Entity parent, unsigned int num_particles, vec2 dist_range, vec2 scale_range, vec4 color);
void createColoredBurst(Entity parent, vec4 color);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);
Entity createHealthbar(vec2 position);
Entity createStaminabar(vec2 position);
Entity createEnemyHealthBar(vec2 position);
Entity createAmmoCounter(vec2 position, vec4 color);
Entity createReloadIndicator(vec2 position);
Entity createText(std::string text, vec2 position, vec3 color, bool followsPlayer);
Entity createWorldEffect(vec2 position, vec2 size_scaling, Effects world_effect);
Entity createLimb(RenderSystem* renderer, vec2 pos);
Entity createTitleScreenButton(vec2 position, TITLE_BUTTON_TYPE button_type);
Entity createHelpPopupBG(vec2 position);
Entity createInventoryBG(vec2 position);
Entity createBorder(vec2 position);
Entity createWeaponIndicator(vec2 position);
Entity createConsumableIndicator(vec2 position, Entity consumable, int keybind);
Entity createPassiveItem(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID tex, int id);
Entity createConsumableItem(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID tex, Consumable id);
Entity createInventoryImage(vec2 pos, TEXTURE_ASSET_ID tex);
Entity createTNT(vec2 position);
Entity createExplosion(vec2 position);
Entity createStumpWall(RenderSystem* renderer, vec2 pos, vec2 tiling_factor);
Entity createMinecart(RenderSystem* renderer, vec2 pos);
Entity createMinecartFull(RenderSystem* renderer, vec2 pos);
Entity createRock(RenderSystem* renderer, vec2 pos);
Entity createRockWall(RenderSystem* renderer, vec2 pos, vec2 tiling_factor);
Entity createBackgroundGrass(RenderSystem* renderer);
Entity createBackgroundRock(RenderSystem* renderer);
Entity createInstructionImage(RenderSystem* renderer, vec2 pos, TEXTURE_ASSET_ID tex);
