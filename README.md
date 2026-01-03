# Outlaw's Ascent (Team 14)

Action-packed Wild West themed roguelite 

## Citations: 

This game uses the assignment 1 template code as a base. Many code and shader files are based off files from that template.

## IMPORTANT NOTE ON SAVES:
- A crash can potentially occur if the game tries to load a save created with an older version.
- To solve this, simply delete the save file located under `/data/save.json`, and relaunch the game.

## To add more passive items:
- update character's passive item map in components.hpp (HasPassiveItem), edit player's instance of this map when passive item picked up
- update storage of passive items in common.hpp (ITEM_NAME_MAP, ITEM_EFFECT_MAP)
- and all the other changes required for adding a new textured entity

## Notes on creative components:
- Particles use instanced rendering to improve performance. The relevant code can be found under `render_system.cpp`, in the `drawParticlesInstanced` function.
- Complex prescribed motion is implemented in the "firebomb" enemy. The projectiles move in an arc according to a quadratic bezier equation. The relevant code for setting up the curves be found under `world_init.cpp`, in the `createFireBomb` constructor. For moving the object, see the `move_fixed_entities` function under PhysicsSystem. Also note that the effect is more pronounced in the east/west directions. Towards north/south, the projectile will be thrown in a straight path instead.