// internal
#include "render_system.hpp"
#include <SDL.h>

#include "common.hpp"
#include "components.hpp"
#include "physics_system.hpp"
#include "world_system.hpp"

#include "tiny_ecs_registry.hpp"

// glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// etc.

#include <iostream>

// THIS FILE IS TAKEN FROM ASSIGNMENT 1 TEMPLATE


void RenderSystem::drawTexturedMesh(Entity entity, const mat3 &projection)
{
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
    // exclude background from using motion (it does not have one)
    if (registry.backgrounds.has(entity)) {
        Background& back = registry.backgrounds.get(entity);
        transform.translate(back.position);
        transform.scale(back.scale);
	}
	else if (registry.titleScreens.has(entity)) {
		TitleScreen& title = registry.titleScreens.get(entity);
		transform.translate(title.position);
		transform.scale(title.scale);
	}
	else {
        Motion &motion = registry.motions.get(entity);
        transform.translate(motion.position);
        transform.rotate(motion.angle);
        transform.scale(motion.scale);
    }

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::COLOURED ) {

        GLint in_position_loc = glGetAttribLocation(program, "in_position");
        GLint in_color_loc = glGetAttribLocation(program, "in_color");
        gl_has_errors();

        glEnableVertexAttribArray(in_position_loc);
        glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
                              sizeof(ColoredVertex), (void *)0);
        gl_has_errors();

        glEnableVertexAttribArray(in_color_loc);
        glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
                              sizeof(ColoredVertex), (void *)sizeof(vec3));
        gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}


		// Getting uniform locations for glUniform* calls
		GLint color_uloc = glGetUniformLocation(program, "fcolor");
		const vec4 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec4(1.0);
		glUniform4fv(color_uloc, 1, (float*)&color);
		gl_has_errors();

	if (render_request.used_effect != EFFECT_ASSET_ID::COLOURED) {

		GLint col_uloc = glGetUniformLocation(program, "col");
		const float col = registry.animationStates.has(entity) ? registry.animationStates.get(entity).col : 0.0;
		glUniform1f(col_uloc, (float)col);

		gl_has_errors();
		GLint row_uloc = glGetUniformLocation(program, "row");
		const float row = registry.animationStates.has(entity) ? registry.animationStates.get(entity).row : 0.0;
		glUniform1f(row_uloc, (float)row);

		gl_has_errors();
		GLint num_cols_uloc = glGetUniformLocation(program, "num_cols");
		const float num_cols = registry.animationStates.has(entity) ? registry.animationStates.get(entity).num_cols : 1.0;
		glUniform1f(num_cols_uloc, (float)num_cols);

		gl_has_errors();
		GLint num_rows_uloc = glGetUniformLocation(program, "num_rows");
		const float num_rows = registry.animationStates.has(entity) ? registry.animationStates.get(entity).num_rows : 1.0;
		glUniform1f(num_rows_uloc, (float)num_rows);
	}

    GLint tile_x_uloc = glGetUniformLocation(program, "tile_x");
    GLint tile_y_uloc = glGetUniformLocation(program, "tile_y");
    float tile_x, tile_y;
    if (registry.backgrounds.has(entity)) {
        tile_x = registry.backgrounds.get(entity).tiling_factor.x;
        tile_y = registry.backgrounds.get(entity).tiling_factor.y;
    } else if (registry.obstacles.has(entity)) {
		tile_x = registry.obstacles.get(entity).tiling_factor.x;
		tile_y = registry.obstacles.get(entity).tiling_factor.y;
	} else if (registry.canMoveThroughs.has(entity)) {
		tile_x = registry.canMoveThroughs.get(entity).tiling_factor.x;
		tile_y = registry.canMoveThroughs.get(entity).tiling_factor.y;
	} else {
        tile_x = 1.0f;
        tile_y = 1.0f;
	}
    glUniform1f(tile_x_uloc, (float) tile_x);
    glUniform1f(tile_y_uloc, (float) tile_y);

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "model");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
    GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

// draw the intermediate texture to the screen, with some distortion to simulate low health
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the water texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::LOWHEALTH]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint lowhealth_program = effects[(GLuint)EFFECT_ASSET_ID::LOWHEALTH];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(lowhealth_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(lowhealth_program, "darken_screen_factor");
	GLuint isHealthy_uloc = glGetUniformLocation(lowhealth_program, "isHealthy"); // Get the location of isBasic
	GLuint isBlinded_uloc = glGetUniformLocation(lowhealth_program, "isBlinded");
    GLuint isWet_uloc = glGetUniformLocation(lowhealth_program, "isWet");

    //std::cout << "isHealth" << isHealthy << std::endl;
	glUniform1i(isHealthy_uloc, isHealthy);
	glUniform1i(isBlinded_uloc, isBlinded);
    glUniform1i(isWet_uloc, isWet);

    glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(lowhealth_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	// glClearColor(0.76f, 0.70f, 0.50f, 1.0);
	glClearColor(149.0f/255.0f, 126/255.0f, 54.0f/255.0f, 1.0);
//    glClearColor(GLfloat(172 / 255), GLfloat(216 / 255), GLfloat(255 / 255), 1.0);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();
    // create the view matrix for the camera
    Transform view = createCameraTransform();

	const GLuint program = (GLuint)effects[(GLuint)EFFECT_ASSET_ID::TEXTURED];
	glUseProgram(program);
	gl_has_errors();
	// set up uniforms for the projection and view matrices
	GLuint projection_loc = glGetUniformLocation(program, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection_2D);
	GLuint view_loc = glGetUniformLocation(program, "view");
	glUniformMatrix3fv(view_loc, 1, GL_FALSE, (float *)&view);
	gl_has_errors();

	const GLuint programC= (GLuint)effects[(GLuint)EFFECT_ASSET_ID::COLOURED];
	glUseProgram(programC);
	gl_has_errors();
	projection_loc = glGetUniformLocation(programC, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection_2D);
	view_loc = glGetUniformLocation(programC, "view");
	glUniformMatrix3fv(view_loc, 1, GL_FALSE, (float*)&view);
	gl_has_errors();
  
    // render the background first
    // NOTE: assumes we have exactly one background
    drawTexturedMesh(registry.backgrounds.entities[0], projection_2D);

	// then render things that should be rendered next

	for (Entity entity : registry.renderGroups.entities) {
		if (!registry.motions.has(entity) || !registry.backLayers.has(entity))
			continue;

		// iterate over indices to select RenderRequests
		RenderGroup& rg = registry.renderGroups.get(entity);
		for (unsigned int idx : rg.indices) {
			Entity e = rg.entities[idx];
			// check if the entity is actually on screen
			Motion& motion = registry.motions.get(e);
			if (OFFSCREEN_RENDERING || isVisibleOnScreen(motion))
				drawTexturedMesh(e, projection_2D);
		}
	}

	// then render most things

	for (Entity entity: registry.renderGroups.entities) {
        if (!registry.motions.has(entity) || registry.uiElements.has(entity) || registry.backLayers.has(entity))
            continue;

        // iterate over indices to select RenderRequests
        RenderGroup &rg = registry.renderGroups.get(entity);
        for (unsigned int idx: rg.indices) {
            Entity e = rg.entities[idx];
            // check if the entity is actually on screen
            Motion &motion = registry.motions.get(e);
            if (OFFSCREEN_RENDERING || isVisibleOnScreen(motion))
                drawTexturedMesh(e, projection_2D);
        }
    }

	// render particles instanced
	drawParticlesInstanced(view.mat, projection_2D);

	// render the screen shader
	if (registry.deathTimers.size() == 0) {
		drawToScreen();
	}

	// now draw UI elements
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	for (Entity entity : registry.uiElements.entities)
	{
		if (!registry.motions.has(entity) || !registry.renderRequests.has(entity) || registry.inventoryImages.has(entity) || registry.inventoryBGs.has(entity) || registry.helpPopupBGs.has(entity) || registry.titleScreenButtons.has(entity))
			continue;
		
		drawTexturedMesh(entity, projection_2D);
	}

	// draw title screen if it is required
	// NOTE: assumes we have exactly one title screen
	if (registry.titleScreens.components[0].doRender) {
		drawTexturedMesh(registry.titleScreens.entities[0], projection_2D);
	}
  
	// now draw inventory / help BGs

	for (Entity entity : registry.uiElements.entities)
	{
		if (!registry.motions.has(entity) || !registry.renderRequests.has(entity) || !(registry.inventoryBGs.has(entity) || registry.helpPopupBGs.has(entity) || registry.titleScreenButtons.has(entity)))
			continue;

		drawTexturedMesh(entity, projection_2D);
	}

	// now draw inventory images
	for (Entity entity : registry.uiElements.entities)
	{
		if (!registry.motions.has(entity) || !registry.renderRequests.has(entity) || !registry.inventoryImages.has(entity))
			continue;

		drawTexturedMesh(entity, projection_2D);
	}

	Transform text_trans;
	text_trans.scale(vec2(1.0, -1.0));
	// render our text entities here...

	for (IsText it : registry.texts.components) {
		if (it.doRender) {
			renderText(it.text, it.pos.x, it.pos.y, 1.0, it.color, mat4(text_trans.mat), it.follows_player ? mat4(view.mat) : mat4(1.0f));
		}
	}

	// render the screen shader
	if (registry.deathTimers.size() > 0) {
		drawToScreen();
	}

	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float) window_width_px;
	float bottom = (float) window_height_px;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

// TODO: split up this function, its doing too many things
Transform RenderSystem::createCameraTransform()
{
	Entity& player = registry.players.entities[0];
    Player& player_data = registry.players.get(player);
	Motion& player_motion = registry.motions.get(player);
    Background& bg = registry.backgrounds.components[0];
	Transform view;
    // get the world boundaries, in world coordinates
    const float max_x = bg.tiling_factor.x / 2 * SPRITE_SIZE_STANDARD;
    const float max_y = bg.tiling_factor.y / 2 * SPRITE_SIZE_STANDARD;
	// update camera position and deltas
	player_data.camera_pos_old = player_data.camera_pos;
    player_data.camera_pos = player_motion.position;
    const float half_width = window_width_px / 2.0f;
    const float half_height = window_height_px / 2.0f;

    // check if the player is near a world boundary in the x direction
    if (max_x - abs(player_motion.position.x) <= half_width) {
        player_data.camera_pos.x = max_x - half_width;
        if (player_motion.position.x < 0) {
            player_data.camera_pos.x *= -1;
        }
    }
    // ditto for the y direction
    if (max_y - abs(player_motion.position.y) <= half_height) {
        player_data.camera_pos.y = max_y - half_height;
        if (player_motion.position.y < 0) {
            player_data.camera_pos.y *= -1;
        }
    }

	// update hud positions with camera delta
	vec2 camera_delta = player_data.camera_pos_old - player_data.camera_pos;
	for (Entity& ui_elem : registry.uiElements.entities) {
		if (registry.motions.has(ui_elem)) {
			Motion& ui_elem_motion = registry.motions.get(ui_elem);
			ui_elem_motion.position -= camera_delta;

		}
	}

	// center the camera on the player (or a point near the player)
	vec2 centered = vec2(-(player_data.camera_pos.x - half_width), 
                         -(player_data.camera_pos.y - half_height));
	// scale the camera with respect to the window dimensions
	view.translate(vec2(half_width, half_height));
	view.scale(player_data.camera_scale);
	view.translate(vec2(-half_width, -half_height));
	view.translate(centered);

    return view;
}

// checks if the given position is visible from the camera
// adds a small margin around the screen to prevent entities
// from suddenly popping up
bool RenderSystem::isVisibleOnScreen(Motion& motion)
{
	Entity player = registry.players.entities[0];
	Player& player_data = registry.players.get(player);
	vec2 camera_pos = player_data.camera_pos;
	vec2 position = motion.position;
	vec2 scale = motion.scale;
	const float half_width = window_width_px / 2.0f;
	const float half_height = window_height_px / 2.0f;
	const float sx = scale.x * 0.5f;
	const float sy = scale.y * 0.5f;

	// padding to prevent rendering issues
	const float padding = SPRITE_SIZE_STANDARD * 2;

	return position.x-sx <= camera_pos.x+half_width+padding &&
		   position.x+sx >= camera_pos.x-half_width-padding &&
		   position.y-sy <= camera_pos.y+half_height+padding &&
		   position.y+sy >= camera_pos.y-half_height-padding;
}

// render all particles, using instanced rendering
void RenderSystem::drawParticlesInstanced(const mat3& view, const mat3& projection) 
{
	if (registry.particles.size() == 0) return;

	std::vector<mat3> transforms;
	std::vector<vec4> colors;

	for (Entity& entity : registry.particles.entities) {
		Motion& motion = registry.motions.get(entity);
		Transform transform;
		transform.translate(motion.position);
        transform.rotate(motion.angle);
        transform.scale(motion.scale);

		mat3 full_transform = projection * view * transform.mat;
		transforms.push_back(full_transform);

		vec4& color = registry.colors.get(entity);
		colors.push_back(color);
	}
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::PARTICLE]);

	glBindBuffer(GL_ARRAY_BUFFER, part_transform_buffer);
	glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(mat3), transforms.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, part_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec4), colors.data(), GL_DYNAMIC_DRAW);

	// Vertex data setup (positions and base colors)
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::PARTICLE]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::PARTICLE]);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void *)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void *)sizeof(vec3));

	glBindBuffer(GL_ARRAY_BUFFER, part_transform_buffer);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * 3 * sizeof(GLfloat), (void*)(0));
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1); // Make instanced

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * 3 * sizeof(GLfloat), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1); // Make instanced

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * 3 * sizeof(GLfloat), (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1); // Make instanced

	glBindBuffer(GL_ARRAY_BUFFER, part_color_buffer);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(5);
	glVertexAttribDivisor(5, 1); // Make instanced

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, registry.particles.size());
}


// THIS FUNCTION IS TAKEN FROM SIMPLEGL (LECTURE)

void RenderSystem::renderText(std::string text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans, const glm::mat4& view)
{
	// TODO: use program, load variables, bind to VAO, then iterate thru chars


	const GLuint m_font_shaderProgram = effects[(GLuint)EFFECT_ASSET_ID::FONT];
	// activate the shader program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::FONT]);

	// get shader uniforms
	GLint textColor_location =
		glGetUniformLocation(m_font_shaderProgram, "textColor");
	glUniform3f(textColor_location, color.x, color.y, color.z);

	GLint transformLoc =
		glGetUniformLocation(m_font_shaderProgram, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

	GLint viewLoc =
		glGetUniformLocation(m_font_shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glBindVertexArray(m_font_VAO);

	// iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = m_ftCharacters[*c];

		float xpos = x + ch.Bearing.x * scale;
	
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);

		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// glDrawArrays(GL_TRIANGLES, 0, 3);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)

	}
	glBindVertexArray(vao);
	glBindTexture(GL_TEXTURE_2D, 0);
}
/**/
