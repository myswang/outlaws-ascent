#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"
#include "common.hpp"
#include <sstream>
#include <cassert>
#include <iostream>

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE


class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<LifeTimer> lifeTimers;
	ComponentContainer<BulletCooldownTimer> bulletCooldownTimers;
	ComponentContainer<BulletLifeTime> bulletLifeTimes;
	ComponentContainer<Health> healths;
	ComponentContainer<Stamina> staminas;
	ComponentContainer<IsText> texts;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<Obstacle> obstacles;
	ComponentContainer<Slower> slowers;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Damaging> damaging;
	ComponentContainer<EnemyDamaging> enemyDamaging;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec4> colors;
	ComponentContainer<HasKnockback> knockbackers;
	ComponentContainer<KnockbackTimer> knockbackTimers;
	ComponentContainer<InvincibleTimer> invincibleTimers;
	ComponentContainer<DashCooldownTimer> dashCooldownTimers;
	ComponentContainer<Weapon> weapons;
	ComponentContainer<EquippedWeapon> equippedWeapons;
	ComponentContainer<AmmoCounter> ammoCounters;
	ComponentContainer<HoldingWeapon> weaponHolders;
	ComponentContainer<Movable> movables;
  	ComponentContainer<Following> following;
  	ComponentContainer<AnimationState> animationStates;
	ComponentContainer<HealthBar> healthbars;
	ComponentContainer<StaminaBar> staminabars;
	ComponentContainer<EnemyHealthBar> enemyHealthBars;
	ComponentContainer<HasHealthBar> hasHealthBars;
	ComponentContainer<UIElement> uiElements;
	ComponentContainer<ReloadIndicator> reloadIndicators;
  	ComponentContainer<Background> backgrounds;
  	ComponentContainer<WorldObject> worldObjects;
    ComponentContainer<CollisionTimer> collisionTimers;
 	ComponentContainer<AttackTimer> attackTimers;
  	ComponentContainer<ReloadTimer> reloadTimers;
  	ComponentContainer<FPSCounter> fpsCounters;
	ComponentContainer<Limb> limbs;
	ComponentContainer<Skeleton> skeletons;
	ComponentContainer<RenderGroup> renderGroups;
	ComponentContainer<HelpPopupBG> helpPopupBGs;
	ComponentContainer<HelpText> helpTexts;
	ComponentContainer<WeaponIndicator> weaponIndicators;
	ComponentContainer<PassiveItem> passiveItems;
	ComponentContainer<HasPassiveItem> hasPassiveItems;
	ComponentContainer<PassiveTextTimer> passiveTextTimers;
	ComponentContainer<PickedUpItemTimer> pickedUpItemTimers;
	ComponentContainer<Particle> particles;
    ComponentContainer<ConsumableItem> consumableItems;
    ComponentContainer<ConsumableIndicator> consumableIndicators;
    ComponentContainer<AnimationTimer> animationTimers;
	ComponentContainer<InventoryBG> inventoryBGs;
	ComponentContainer<InventoryText> inventoryTexts;
	ComponentContainer<InventoryImage> inventoryImages;
	ComponentContainer<InventoryBorder> inventoryBorders;
	ComponentContainer<PauseText> pauseTexts;
	ComponentContainer<CanMoveThrough> canMoveThroughs;
	ComponentContainer<FireBomb> fireBombs;
    ComponentContainer<Fleeing> fleeing;
    ComponentContainer<Attacking> attacking;
	ComponentContainer<TNT> explodingTNTs;
	ComponentContainer<Explosion> explosions;
	ComponentContainer<Fire> fires;
	ComponentContainer<HurtSoundTimer> hurtSoundTimers;
	ComponentContainer<Blinder> blinders;
	ComponentContainer<BlindTimer> blindTimers;
	ComponentContainer<BleedTimer> bleedTimers;
	ComponentContainer<CausesBleeding> bleedCausers;
	ComponentContainer<BackLayer> backLayers;
	ComponentContainer<EnemyCounterText> enemyCounters;
  ComponentContainer<WorldEffect> worldEffects;
  ComponentContainer<EffectTimer> effectTimers;
  ComponentContainer<PuddleTimer> puddleTimers;
  ComponentContainer<PrescribedMotion> prescribedMotions;
  ComponentContainer<TutorialText> tutorialTexts;
	ComponentContainer<TitleScreen> titleScreens;
	ComponentContainer<TitleScreenButton> titleScreenButtons;
	ComponentContainer<TitleScreenButtonText> titleScreenButtonTexts;
	ComponentContainer<LevelState> levelStates;

    // constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		registry_list.push_back(&deathTimers);
        registry_list.push_back(&collisionTimers);
        registry_list.push_back(&lifeTimers);
		registry_list.push_back(&bulletCooldownTimers);
		registry_list.push_back(&bulletLifeTimes);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&enemies);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&obstacles);
		registry_list.push_back(&slowers);
		registry_list.push_back(&healths);
		registry_list.push_back(&staminas);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&damaging);
		registry_list.push_back(&enemyDamaging);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&knockbackers);
		registry_list.push_back(&knockbackTimers);
		registry_list.push_back(&weapons);
		registry_list.push_back(&equippedWeapons);
		registry_list.push_back(&weaponHolders);
		registry_list.push_back(&invincibleTimers);
		registry_list.push_back(&dashCooldownTimers);
		registry_list.push_back(&movables);
    	registry_list.push_back(&following);
    	registry_list.push_back(&animationStates);
		registry_list.push_back(&ammoCounters);
		registry_list.push_back(&uiElements);
		registry_list.push_back(&reloadIndicators);
    	registry_list.push_back(&backgrounds);
		registry_list.push_back(&worldObjects);
		registry_list.push_back(&texts);
		registry_list.push_back(&attackTimers);
		registry_list.push_back(&reloadTimers);
		registry_list.push_back(&fpsCounters);
		registry_list.push_back(&limbs);
		registry_list.push_back(&skeletons);
		registry_list.push_back(&renderGroups);
		registry_list.push_back(&helpPopupBGs);
		registry_list.push_back(&helpTexts);
		registry_list.push_back(&weaponIndicators);
		registry_list.push_back(&passiveItems);
		registry_list.push_back(&hasPassiveItems);
		registry_list.push_back(&passiveTextTimers);
		registry_list.push_back(&pickedUpItemTimers);
		registry_list.push_back(&particles);
        registry_list.push_back(&consumableItems);
        registry_list.push_back(&consumableIndicators);
        registry_list.push_back(&animationTimers);
		registry_list.push_back(&inventoryBGs);
		registry_list.push_back(&inventoryTexts);
		registry_list.push_back(&inventoryImages);
		registry_list.push_back(&inventoryBorders);
		registry_list.push_back(&pauseTexts);
		registry_list.push_back(&healthbars);
		registry_list.push_back(&staminabars);
		registry_list.push_back(&enemyHealthBars);
		registry_list.push_back(&hasHealthBars);
		registry_list.push_back(&explodingTNTs);
		registry_list.push_back(&explosions);
		registry_list.push_back(&fleeing);
		registry_list.push_back(&attacking);
		registry_list.push_back(&canMoveThroughs);
		registry_list.push_back(&fireBombs);
	registry_list.push_back(&fires);
	registry_list.push_back(&hurtSoundTimers);
	registry_list.push_back(&blinders);
	registry_list.push_back(&blindTimers);
	registry_list.push_back(&bleedTimers);
	registry_list.push_back(&bleedCausers);
	registry_list.push_back(&backLayers);
	registry_list.push_back(&enemyCounters);
  registry_list.push_back(&worldEffects);
  registry_list.push_back(&effectTimers);
  registry_list.push_back(&puddleTimers);
  registry_list.push_back(&prescribedMotions);
        registry_list.push_back(&tutorialTexts);
		registry_list.push_back(&titleScreens);
		registry_list.push_back(&titleScreenButtons);
		registry_list.push_back(&titleScreenButtonTexts);
		registry_list.push_back(&levelStates);
  

     deathTimers.name = "deathTimers";
		lifeTimers.name = "lifeTimers";
		bulletCooldownTimers.name = "bulletCooldownTimers";
		bulletLifeTimes.name = "bulletLifeTimes";
		healths.name = "healths";
		staminas.name = "staminas";
		texts.name = "texts";
		motions.name = "motions";
		collisions.name = "Not Saved";
		players.name = "players";
		enemies.name = "enemies";
		projectiles.name = "projectiles";
		obstacles.name = "obstacles";
		slowers.name = "slowers";
		meshPtrs.name = "meshPtrs";
		renderRequests.name = "renderRequests";
		screenStates.name = "Not Saved";
		damaging.name = "damaging";
		enemyDamaging.name = "enemyDamaging";
		debugComponents.name = "debugComponents";
		colors.name = "colors";
		knockbackers.name = "knockbackers";
		knockbackTimers.name = "knockbackTimers";
		invincibleTimers.name = "invincibleTimers";
		dashCooldownTimers.name = "dashCooldownTimers";
		weapons.name = "weapons";
		equippedWeapons.name = "equippedWeapons";
		ammoCounters.name = "ammoCounters";
		weaponHolders.name = "weaponHolders";
		movables.name = "movables";
		following.name = "following";
		animationStates.name = "animationStates";
		healthbars.name = "healthbars";
		staminabars.name = "staminabars";
		enemyHealthBars.name = "enemyHealthBars";
		hasHealthBars.name = "hasHealthBars";
		uiElements.name = "uiElements";
		reloadIndicators.name = "reloadIndicators";
		backgrounds.name = "backgrounds";
		worldObjects.name = "worldObjects";
		collisionTimers.name = "collisionTimers";
		attackTimers.name = "attackTimers";
		reloadTimers.name = "reloadTimers";
		fpsCounters.name = "fpsCounters";
		limbs.name = "limbs";
		skeletons.name = "skeletons";
		renderGroups.name = "renderGroups";
		helpPopupBGs.name = "helpPopupBGs";
		helpTexts.name = "helpTexts";
		weaponIndicators.name = "weaponIndicators";
		passiveItems.name = "passiveItems";
		hasPassiveItems.name = "hasPassiveItems";
		passiveTextTimers.name = "passiveTextTimers";
		pickedUpItemTimers.name = "pickedUpItemTimers";
		consumableItems.name = "consumableItems";
		consumableIndicators.name = "consumableIndicators";
		animationTimers.name = "animationTimers";
		inventoryBGs.name = "inventoryBGs";
		inventoryTexts.name = "inventoryTexts";
		inventoryImages.name = "inventoryImages";
		inventoryBorders.name = "inventoryBorders";
		pauseTexts.name = "pauseTexts";
		particles.name = "particles";
		explodingTNTs.name = "explodingTNTs";
		explosions.name = "explosions";
		fleeing.name = "fleeing";
		attacking.name = "attacking";
		canMoveThroughs.name = "canMoveThroughs";
		fireBombs.name = "fireBombs";
		fires.name = "fires";
		hurtSoundTimers.name = "hurtSoundTimers";
		blindTimers.name = "blindTimers";
		blinders.name = "blinders";
		bleedTimers.name = "bleedTimers";
		bleedCausers.name = "bleedCausers";
		backLayers.name = "backLayers";
		enemyCounters.name = "enemyCounters";
    effectTimers.name = "effectTimers";
    worldEffects.name = "worldEffects";
    puddleTimers.name = "puddleTimers";
	  prescribedMotions.name = "prescribedMotions";
    tutorialTexts.name = "tutorialText";
		titleScreens.name = "Not Saved";
		titleScreenButtons.name = "titleScreenButtons";
		titleScreenButtonTexts.name = "titleScreenButtonTexts";
		levelStates.name = "levelStates";
    }
		


	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void clear_most_components(bool clear_level_states) {
		if (!clear_level_states) {
			for (ContainerInterface* reg : registry_list) {
				if (reg->getName() != "Not Saved" && reg->getName() != "levelStates") {
					reg->clear();
				}
			}
		}
		else {
			for (ContainerInterface* reg : registry_list) {
				if (reg->getName() != "Not Saved") {
					reg->clear();
				}
			}
		}
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}

	// TODO make this available to all component containers
	json toJSON() const {
		/*json j;
		for (auto* reg : registry_list) {
			if (reg->getName() == "players") {
				json component_json = json::array();
				auto& container = static_cast<ComponentContainer<Player>&>(*reg);
				for (Entity entity : container.entities) {
					json entity_json;
					container.serialize(entity, entity_json);
					component_json.push_back({ {"entity", static_cast<unsigned int>(entity)}, {"component", entity_json} });
				}
				j[reg->getName()] = component_json;
			}


		}
		return j;*/

		nlohmann::json j;
		for (const auto* reg : registry_list) {
			nlohmann::json component_json = reg->toJSON();
			if (!component_json.empty()) {
				j.update(component_json);
			}
		}
		return j;

		/*json j;
		auto& container = players;
		json component_json = json::array();
		for (Entity entity : container.entities) {
			json entity_json;
			container.serialize(entity, entity_json);
			try {
				component_json.push_back({ {"entity", static_cast<unsigned int>(entity)}, {"component", entity_json} });
			}
			catch (const nlohmann::json::exception& e) {
				std::cout << "Error serializing Player: " << e.what() << std::endl;
			}
		}
		j["players"] = component_json;
		return j;*/
	}

	// TODO
	void fromJSON(const json& j) {
		for (auto& reg : registry_list) {
			if (reg->getName() != "Not Saved") {
				reg->fromJSON(j.at(reg->getName()));
			}
		}

		//auto& container = players;

		//if (j.contains("players")) {
		//	const json& players_json = j["players"];
		//	for (const auto& entry : players_json) {
		//		try {
		//			unsigned int entity_id = entry["entity"].get<unsigned int>();

		//			// find Entity or create it if it doesn't exist
		//			// Entity entity = container.get(entity_id);

		//			// deserialize the component
		//			json component_data = entry["component"];
		//			//container.deserialize(entity, component_data);
		//		} catch (const nlohmann::json::exception& e) {
		//			std::cout << "Error deserializing Player: " << e.what() << std::endl;
		//		}
		//	}
		//}

	}
	

};

extern ECSRegistry registry;
