#pragma once

#include <array>
#include <utility>
#include <map>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE
// FONT INITIALIZATION IS TAKEN FROM SIMPLEGL-3 



// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;
	std::map<char, Character> m_ftCharacters;

	GLuint vao;
	GLuint m_font_VAO;
	GLuint m_font_VBO;
	GLuint m_part_VAO;

	GLuint part_transform_buffer;
	GLuint part_color_buffer;

    // Make sure these paths remain in sync with the associated enumerators.
    // Associated id with .obj path
    const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
            {
                    std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CACTUS_MESH, mesh_path("cactus.obj"))
                    // specify meshes of other assets here
            };

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("background.png"),
            textures_path("player.png"), 
			textures_path("enemy.png"),
			textures_path("cactus.png"),
			textures_path("barrel.png"),
			textures_path("fence.png"),
			textures_path("bullet.png"),
			textures_path("gun_revolver.png"),
			textures_path("gun_revolver2.png"),
			textures_path("ammo.png"),
			textures_path("rkey.png"),
			textures_path("arm.png"),
			textures_path("arm_enemy.png"),
			textures_path("gun_shotgun.png"),
			textures_path("gun_rifle.png"),
			textures_path("elixir.png"),
			textures_path("powder.png"),
			textures_path("padding.png"),
			textures_path("gunmod.png"),
			textures_path("syringe.png"),
			textures_path("chicken.png"),
			textures_path("landmine.png"),
			textures_path("tnt.png"),
			textures_path("tnt_explode.png"),
			textures_path("explosionTNT.png"),
			textures_path("explosion.png"),
			textures_path("hearty_stew.png"),
			textures_path("border.png"),
			textures_path("alt_enemy.png"),
			textures_path("firebottle.png"),
			textures_path("gun_firebottle.png"),
			textures_path("knife.png"),
			textures_path("potion.png"),
			textures_path("blank_weapon_size.png"),
			textures_path("dirt.png"),
			textures_path("blood_1.png"),
			textures_path("blood_2.png"),
			textures_path("blood_3.png"),
			textures_path("bush.png"),
			textures_path("thorny.png"),
			textures_path("stump.png"),
			textures_path("mud.png"),
			textures_path("oasis3.png"),
			textures_path("puddle.png"),
			textures_path("puddle2.png"),
			textures_path("puddle3.png"),
			textures_path("rock.png"),
			textures_path("minecart_empty.png"),
			textures_path("minecart_full.png"),
			textures_path("grass.png"),
			textures_path("stone.png"),
			textures_path("i_move.png"),
			textures_path("i_dash.png"),
			textures_path("i_shoot.png"),
			textures_path("i_reload.png"),
			textures_path("i_change_weapon.png"),
			textures_path("i_consumable.png"),
			textures_path("title_screen.png")
    };


	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("textured"),
		shader_path("lowhealth"),
		shader_path("coloured"),
		shader_path("font"),
		shader_path("particle")};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	bool initializeFonts();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the wind
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	void renderText(std::string text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans, const glm::mat4& view);

	mat3 createProjectionMatrix();
	Transform createCameraTransform();


private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();
	void drawParticlesInstanced(const mat3& view, const mat3& projection);
	bool isVisibleOnScreen(Motion& motion);

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
