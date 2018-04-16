#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

//Meshes
map<string, mesh> model_meshes;
array<mesh, 2> terrain_meshes;
array<mesh, 7> pyramid_meshes;
mesh skybox;
//Textures
map<string, texture> textures;
//Cube map
cubemap cube_map;
//Shadow map
shadow_map shadow_spot;
//Effects
map<string, effect> effects;
//Lights
spot_light moon_light;
spot_light shadow_light;
point_light gem_light;
//Cameras
free_camera free_cam;
target_camera target_cam;
arc_ball_camera arc_cam;
//Model matrices for the instances
array<mat4, 70> model_matrices;
//Vbo for the instances
GLuint position_vbo;
//Postprocessing elements
frame_buffer frame;
geometry screen_quad;
//Additional variables
bool top = false;
string camera_on = "Target Camera";
double free_cam_cursor_x = 0.0;
double free_cam_cursor_y = 0.0;
double arc_cam_cursor_x = 0.0;
double arc_cam_cursor_y = 0.0;
vec2 uv_scroll;
bool thermal_vision_on = false;

//Render Methods
void setUniforms(mat4 &MVP, mesh &m, effect &eff)
{
	renderer::bind(eff);
	//Set MVP uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	//Set M uniform
	glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(m.get_transform().get_transform_matrix()));
	//Set N uniform
	glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
	//Create lightMVP
	auto lightM = m.get_transform().get_transform_matrix();
	auto lightV = shadow_spot.get_view();
	mat4 LightProjectionMat = perspective<float>(90.f, renderer::get_screen_aspect(), 0.1f, 2000.f);
	auto lightMVP = LightProjectionMat * lightV * lightM;
	// Set lightMVP uniform
	glUniformMatrix4fv(eff.get_uniform_location("lightMVP"), 1, GL_FALSE, value_ptr(lightMVP));
	//Bind material
	renderer::bind(m.get_material(), "mat");
	//Bind spot light
	renderer::bind(shadow_light, "spot");
	//Bind point light
	renderer::bind(gem_light, "point");
	//Set eye position
	if (camera_on._Equal("Free Camera"))
	{
		glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(free_cam.get_position()));
	}
	if (camera_on._Equal("Target Camera"))
	{
		glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(target_cam.get_position()));
	}
	if (camera_on._Equal("Arc Camera"))
	{
		glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(arc_cam.get_position()));
	}
}
void setTerrainTextures(mesh &m, array<texture, 4> &textures)
{
	//Bind textures
	renderer::bind(textures[0], 0);
	renderer::bind(textures[1], 1);
	renderer::bind(textures[2], 2);
	renderer::bind(textures[3], 3);
	//Set uniform values for textures
	static int tex_indices[] = { 0, 1, 2, 3 };
	glUniform1iv(effects["terrain_eff"].get_uniform_location("tex"), 4, tex_indices);
	// Bind shadow map texture
	renderer::bind(shadow_spot.buffer->get_depth(), 4);
	// Set the shadow_map uniform
	glUniform1i(effects["terrain_eff"].get_uniform_location("shadow_map"), 4);
}
void setNormalMappingTextures(mesh &m, texture &tex, texture &normal_map)
{
	//Bind texture
	renderer::bind(tex, 0);
	//Set tex uniform
	glUniform1i(effects["normal_map_eff"].get_uniform_location("tex"), 0);
	//Bind normal_map
	renderer::bind(normal_map, 1);
	//Set normal_map uniform
	glUniform1i(effects["normal_map_eff"].get_uniform_location("normal_map"), 1);
	// Bind shadow map texture
	renderer::bind(shadow_spot.buffer->get_depth(), 2);
	// Set the shadow_map uniform
	glUniform1i(effects["normal_map_eff"].get_uniform_location("shadow_map"), 2);
}
void setBlendMappingTextures(mesh &m, array<texture, 2> &textures, texture &blend_map)
{
	//Bind textures
	renderer::bind(textures[0], 0);
	renderer::bind(textures[1], 1);
	renderer::bind(blend_map, 2);
	//Set the uniform values for textures
	static int tex_indices[] = { 0, 1 };
	glUniform1iv(effects["blend_eff"].get_uniform_location("tex"), 2, tex_indices);
	glUniform1i(effects["blend_eff"].get_uniform_location("blend"), 2);
	// Bind shadow map texture
	renderer::bind(shadow_spot.buffer->get_depth(), 3);
	// Set the shadow_map uniform
	glUniform1i(effects["blend_eff"].get_uniform_location("shadow_map"), 3);
}
//Postprocessing rendering
void postprocessingRendering(texture &alpha_map, effect &eff)
{
	//Bind effect
	renderer::bind(eff);
	//MVP is now the identity matrix
	auto postprocessingMVP = mat4();
	//Set MVP matrix uniform
	glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(postprocessingMVP));
	//Bind texture from frame buffer
	renderer::bind(frame.get_frame(), 0);
	//Set the tex uniform
	glUniform1i(eff.get_uniform_location("tex"), 0);
	//Bind alphamap
	renderer::bind(alpha_map, 1);
	//Set the alphamap uniform
	glUniform1i(eff.get_uniform_location("alpha_map"), 1);
	//Render the screen quad
	renderer::render(screen_quad);
}
//Shadow mapping rendering
void spotShadowing()
{
	//Set render target to shadow map
	renderer::set_render_target(shadow_spot);
	//Clear depth buffer bit
	glClear(GL_DEPTH_BUFFER_BIT);
	//Set face cull mode to front
	glCullFace(GL_FRONT);
	//Bind shader
	renderer::bind(effects["shadow_spot_eff"]);
	//View and projection matrices
	auto V = shadow_spot.get_view();
	mat4 LightProjectionMat = perspective<float>(90.f, renderer::get_screen_aspect(), 0.1f, 2000.f);
	//Render terrain(only the island)
	auto terrainM = terrain_meshes[0].get_transform().get_transform_matrix();
	auto terrainMVP = LightProjectionMat * V * terrainM;
	glUniformMatrix4fv(effects["shadow_spot_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(terrainMVP));
	renderer::render(terrain_meshes[0]);
	//Render model meshes
	for (auto &e : model_meshes) 
	{
		auto m = e.second;
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		auto MVP = LightProjectionMat * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(effects["shadow_spot_eff"].get_uniform_location("MVP"),1, GL_FALSE, value_ptr(MVP));                       								
		renderer::render(m);
	}
	//Render pyramid meshes
	for (int i = 0; i < pyramid_meshes.size(); i++)
	{
		auto M = pyramid_meshes[i].get_transform().get_transform_matrix();
		//Apply hierarchy chain
		for (size_t j = i; j > 0; j--)
		{
			M = pyramid_meshes[j - 1].get_transform().get_transform_matrix() * M;
		}
		//Create MVP
		auto MVP = LightProjectionMat * V * M;
		glUniformMatrix4fv(effects["shadow_spot_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		renderer::render(pyramid_meshes[i]);
	}
	//Render isntanced swords
	renderer::bind(effects["instancing_shadow_spot_eff"]);
	//Set view and projection matrix independently as we need to use the model matrix of each instance
	glUniformMatrix4fv(effects["instancing_eff"].get_uniform_location("V"), 1, GL_FALSE, value_ptr(V));
	glUniformMatrix4fv(effects["instancing_eff"].get_uniform_location("P"), 1, GL_FALSE, value_ptr(LightProjectionMat));
	//Bind VAO
	glBindVertexArray(model_meshes["blade"].get_geometry().get_array_object());
	//Bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	//Load VBO data
	glBufferData(GL_ARRAY_BUFFER, model_matrices.size() * sizeof(glm::mat4), &model_matrices[0], GL_STATIC_DRAW);
	//Render instances
	glDrawElementsInstanced(GL_TRIANGLES, model_meshes["blade"].get_geometry().get_index_count(), GL_UNSIGNED_INT, 0, model_matrices.size());
	glBindVertexArray(0);
	// Set render target back to the screen
	renderer::set_render_target();
	// Set face cull mode to back
	glCullFace(GL_BACK);
}
//Update camera methods
void updateFreeCamera(float delta_time)
{
	//Free camera
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height = (quarter_pi<float>() *(static_cast<float>(renderer::get_screen_height()) / static_cast<float>(renderer::get_screen_width()))) / static_cast<float>(renderer::get_screen_height());
	double current_x;
	double current_y;
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	float delta_x = current_x - free_cam_cursor_x;
	float delta_y = current_y - free_cam_cursor_y;
	// Multiply deltas by ratios - gets actual change in orientation
	delta_x *= ratio_width;
	delta_y *= ratio_height;
	// Rotate cameras by delta
	free_cam.rotate(delta_x, -delta_y);
	//Move the camera using WSAD
	if (glfwGetKey(renderer::get_window(), 'W'))
	{
		free_cam.move(vec3(0.0f, 0.0f, 2.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'S'))
	{
		free_cam.move(vec3(0.0f, 0.0f, -2.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'A'))
	{
		free_cam.move(vec3(-2.0f, 0.0f, 0.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'D'))
	{
		free_cam.move(vec3(2.0f, 0.0f, 0.0f));
	}
	//Camera change
	if (glfwGetKey(renderer::get_window(), '4'))
	{
		camera_on = "Free Camera";
		free_cam.set_position(vec3(10.0f, 100.0f, 225.0f));
		free_cam.set_target(vec3(0.0f, 0.0f, 0.0f));
	}
	free_cam.update(delta_time);
	if (camera_on == "Free Camera")
	{
		//Center of the skybox is the camera
		skybox.get_transform().position = free_cam.get_position();
	}
	free_cam_cursor_x = current_x;
	free_cam_cursor_y = current_y;
}
void updateTargetCamera(float delta_time)
{
	if (glfwGetKey(renderer::get_window(), '1'))
	{
		target_cam.set_position(vec3(500.0f, 100.0f, 300.0f));
		target_cam.set_target(model_meshes["blade"].get_transform().position);
		camera_on = "Target Camera";
	}
	if (glfwGetKey(renderer::get_window(), '2'))
	{
		target_cam.set_position(vec3(500.0f, 40.0f, 0.0f));
		target_cam.set_target(model_meshes["grave"].get_transform().position);
		camera_on = "Target Camera";
	}
	target_cam.update(delta_time);
	if (camera_on == "Target Camera")
	{
		//Center of the skybox is the camera
		skybox.get_transform().position = target_cam.get_position();
	}
}
void updateArcBallCamera(float delta_time)
{
	// The ratio of pixels to rotation - remember the fov
	static const float sh = static_cast<float>(renderer::get_screen_height());
	static const float sw = static_cast<float>(renderer::get_screen_height());
	static const double ratio_width = quarter_pi<float>() / sw;
	static const double ratio_height = (quarter_pi<float>() * (sh / sw)) / sh;
	double current_x;
	double current_y;
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	float delta_x = current_x - arc_cam_cursor_x;
	float delta_y = current_y - arc_cam_cursor_y;
	// Multiply deltas by ratios and delta_time - gets actual change in orientation
	delta_x *= ratio_width;
	delta_y *= ratio_height;
	// Rotate cameras by delta
	arc_cam.rotate(delta_y, delta_x);
	//Camera change
	if (glfwGetKey(renderer::get_window(), '3'))
	{
		camera_on = "Arc Camera";
	}
	// Update the camera
	arc_cam.update(delta_time);
	if (camera_on == "Arc Camera")
	{
		//Center of the skybox is the camera
		skybox.get_transform().position = arc_cam.get_position();
	}
	// Update cursor pos
	arc_cam_cursor_x = current_x;
	arc_cam_cursor_y = current_y;
}
//Generate terrain
void generate_terrain(geometry &geom, const texture &height_map, unsigned int width, unsigned int depth,
	float height_scale) {
	// Contains our position data
	vector<vec3> positions;
	// Contains our normal data
	vector<vec3> normals;
	// Contains our texture coordinate data
	vector<vec2> tex_coords;
	// Contains our texture weights
	vector<vec4> tex_weights;
	// Contains our index data
	vector<unsigned int> indices;

	// Extract the texture data from the image
	glBindTexture(GL_TEXTURE_2D, height_map.get_id());
	auto data = new vec4[height_map.get_width() * height_map.get_height()];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, (void *)data);

	// Determine ratio of height map to geometry
	float width_point = static_cast<float>(width) / static_cast<float>(height_map.get_width());
	float depth_point = static_cast<float>(depth) / static_cast<float>(height_map.get_height());

	// Point to work on
	vec3 point;

	// Part 1 - Iterate through each point, calculate vertex and add to vector
	for (int x = 0; x < height_map.get_width(); ++x) 
	{
		// Calculate x position of point
		point.x = -(width / 2.0f) + (width_point * static_cast<float>(x));

		for (int z = 0; z < height_map.get_height(); ++z) 
		{
			// Calculate z position of point
			point.z = -(depth / 2.0f) + (depth_point* static_cast<float>(z));
			// Y position based on red component of height map data
			point.y = data[(z * height_map.get_width()) + x].y * height_scale;
			// Add point to position data
			positions.push_back(point);
		}
	}

	// Part 1 - Add index data
	for (unsigned int x = 0; x < height_map.get_width() - 1; ++x) 
	{
		for (unsigned int y = 0; y < height_map.get_height() - 1; ++y) 
		{
			// Get four corners of patch
			unsigned int top_left = (y * height_map.get_width()) + x;
			unsigned int top_right = (y * height_map.get_width()) + x + 1;
			unsigned int bottom_left = ((y + 1) * height_map.get_width()) + x;
			unsigned int bottom_right = ((y + 1) * height_map.get_height()) + x + 1;
			// Push back indices for triangle 1 (tl,br,bl)
			indices.push_back(top_left);
			indices.push_back(bottom_right);
			indices.push_back(bottom_left);
			// Push back indices for triangle 2 (tl,tr,br)
			indices.push_back(top_left);
			indices.push_back(top_right);
			indices.push_back(bottom_right);
		}
	}

	// Resize the normals buffer
	normals.resize(positions.size());

	// Part 2 - Calculate normals for the height map
	for (unsigned int i = 0; i < indices.size() / 3; ++i) 
	{
		// Get indices for the triangle
		auto idx1 = indices[i * 3];
		auto idx2 = indices[i * 3 + 1];
		auto idx3 = indices[i * 3 + 2];

		// Calculate two sides of the triangle
		vec3 side1 = positions[idx1] - positions[idx3];
		vec3 side2 = positions[idx1] - positions[idx2];

		// Normal is normal(cross product) of these two sides
		auto n = normalize(cross(side2, side1));
		// Add to normals in the normal buffer using the indices for the triangle
		normals[idx1] = normals[idx1] + n;
		normals[idx2] = normals[idx2] + n;
		normals[idx3] = normals[idx3] + n;
	}

	// Normalize all the normals
	for (auto &n : normals) 
	{
		normalize(n);
	}

	// Part 3 - Add texture coordinates for geometry
	for (unsigned int x = 0; x < height_map.get_width(); ++x) 
	{
		for (unsigned int z = 0; z < height_map.get_height(); ++z) 
		{
			tex_coords.push_back(vec2(width_point * x, depth_point * z));
		}
	}

	// Part 4 - Calculate texture weights for each vertex
	for (unsigned int x = 0; x < height_map.get_width(); ++x) 
	{
		for (unsigned int z = 0; z < height_map.get_height(); ++z) 
		{
			// Calculate tex weight
			vec4 tex_weight(clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.0f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.15f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.5f) / 0.25f, 0.0f, 1.0f),
				clamp(1.0f - abs(data[(height_map.get_width() * z) + x].y - 0.9f) / 0.25f, 0.0f, 1.0f));

			// Sum the components of the vector
			auto total = tex_weight.x + tex_weight.y + tex_weight.z + tex_weight.w;
			// Divide weight by sum
			tex_weight /= total;
			// Add tex weight to weights
			tex_weights.push_back(tex_weight);
		}
	}

	// Add necessary buffers to the geometry
	geom.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	geom.add_buffer(normals, BUFFER_INDEXES::NORMAL_BUFFER);
	geom.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	geom.add_buffer(tex_weights, BUFFER_INDEXES::TEXTURE_COORDS_1);
	geom.add_index_buffer(indices);

	// Delete data
	delete[] data;
}


bool initialise() 
{
	//Hide cursor
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//Capture initial mouse position
	glfwGetCursorPos(renderer::get_window(), &free_cam_cursor_x, &free_cam_cursor_y);
	glfwGetCursorPos(renderer::get_window(), &arc_cam_cursor_x, &arc_cam_cursor_y);
	return true;
}

bool load_content() 
{
	// Create frame buffer - use screen width and height
	frame = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	// Create screen quad
	vector<vec3> positions{ vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f) };
	vector<vec2> tex_coords{ vec2(0.0, 0.0), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };
	screen_quad.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	screen_quad.set_type(GL_TRIANGLE_STRIP);
	screen_quad.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	//Geometry for the terrain
	geometry island;
	geometry lava;
	//Load heightmaps
	textures["island_heightmap"] = texture("res/textures/island_heightmap.jpg");
	textures["lava_heightmap"] = texture("res/textures/lava_heightmap.jpg");
	//Generate terrain
	generate_terrain(island, textures["island_heightmap"], 20, 20, 2.0f);
	generate_terrain(lava, textures["lava_heightmap"], 50, 50, 2.0f);
	//Generate meshes with the geometry
	terrain_meshes[0] = mesh(island);
	terrain_meshes[0].get_transform().position = vec3(0.0f, -60.0f, 0.0f);
	terrain_meshes[0].get_transform().scale *= 100;
	terrain_meshes[0].get_transform().rotate(vec3(0.0f, pi<float>(), 0.0f));
	terrain_meshes[1] = mesh(lava);
	terrain_meshes[1].get_transform().position = vec3(0.0f, -69.0f, 0.0f);
	terrain_meshes[1].get_transform().scale *= 40;
	//Create shadow map
	shadow_spot = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());
	//Create skybox
	skybox = mesh(geometry_builder::create_box());
	//Model meshes
	model_meshes["rock"] = mesh(geometry("res/models/Rock/Rock.obj"));
	model_meshes["blade"] = mesh(geometry("res/models/Blade_Of_Olympus/olympus.obj"));
	model_meshes["column1"] = mesh(geometry("res/models/pillar.obj"));
	model_meshes["column2"] = mesh(geometry("res/models/pillar.obj"));
	model_meshes["grave"] = mesh(geometry("res/models/Grave/grave.obj"));
	//Rotating torus around pyramids meshes
	pyramid_meshes[0] = mesh(geometry_builder::create_pyramid());
	pyramid_meshes[1] = mesh(geometry_builder::create_pyramid());
	pyramid_meshes[2] = mesh(geometry_builder::create_sphere());
	pyramid_meshes[3] = mesh(geometry_builder::create_torus(30, 30, 0.5f, 8.0f));
	pyramid_meshes[4] = mesh(geometry_builder::create_torus(30, 30, 0.5f, 8.0f));
	pyramid_meshes[5] = mesh(geometry_builder::create_torus(30, 30, 0.5f, 8.0f));
	pyramid_meshes[6] = mesh(geometry_builder::create_torus(30, 30, 0.5f, 8.0f));

	//Transform model meshes
	model_meshes["rock"].get_transform().scale *= 10.0f;
	model_meshes["rock"].get_transform().translate(vec3(300.0f, 15.0f, -60.0f));
	model_meshes["blade"].get_transform().scale *= 12.0f;
	model_meshes["blade"].get_transform().rotate(vec3(-half_pi<float>() / 1.5, 0.0f, 0.0f));
	model_meshes["blade"].get_transform().position = model_meshes["rock"].get_transform().position;
	model_meshes["blade"].get_transform().translate(vec3(12.0f, 6.0f, -5.0f));
	model_meshes["column1"].get_transform().scale /= 6.0f;
	model_meshes["column1"].get_transform().position = model_meshes["rock"].get_transform().position;
	model_meshes["column1"].get_transform().translate(vec3(5.0f, -20.0f, 20.0f));
	model_meshes["column2"].get_transform().scale /= 6.0f;
	model_meshes["column2"].get_transform().position = model_meshes["rock"].get_transform().position;
	model_meshes["column2"].get_transform().translate(vec3(5.0f, -20.0f, -47.0f));
	mat3x3 shearing = {
		1.0f, 0.0f, 0.0f,
		1.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	};
	model_meshes["grave"].get_transform().scale = (shearing * model_meshes["grave"].get_transform().scale) / 3.0f;
	model_meshes["grave"].get_transform().rotate(vec3(0.0f, half_pi<float>(), 0.0f));
	model_meshes["grave"].get_transform().position = model_meshes["rock"].get_transform().position;
	model_meshes["grave"].get_transform().translate(vec3(-15.0f, 4.5f, -14.0f));
	//Transform rotating torus and pyramid meshes
	pyramid_meshes[0].get_transform().scale *= 12.0f;
	pyramid_meshes[0].get_transform().position = vec3(0.0f, 0.0f, 0.0f);
	pyramid_meshes[0].get_transform().translate(vec3(-220.0f, 180.0f, -300.0f));
	pyramid_meshes[0].get_transform().rotate(vec3(pi<float>(), 0.0f, 0.0f));
	pyramid_meshes[1].get_transform().translate(vec3(0.0f, -1.2f, 0.0f));
	pyramid_meshes[1].get_transform().rotate(vec3(pi<float>(), 0.0f, 0.0f));
	pyramid_meshes[2].get_transform().translate(vec3(0.0f, -0.6f, 0.0f));
	pyramid_meshes[2].get_transform().scale /= 10.0f;
	pyramid_meshes[3].get_transform().scale *= 10.0f;
	pyramid_meshes[3].get_transform().translate(vec3(0.0f, 0.6f, 0.0f));
	pyramid_meshes[3].get_transform().scale /= 6.5f;
	pyramid_meshes[3].get_transform().translate(vec3(0.0f, -0.6f, 0.0f));
	pyramid_meshes[4].get_transform().scale *= 1.1f;
	pyramid_meshes[4].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
	pyramid_meshes[5].get_transform().scale *= 1.15f;
	pyramid_meshes[5].get_transform().rotate(vec3(half_pi<float>() / 2.0f, 0.0f, 0.0f));
	pyramid_meshes[6].get_transform().scale *= 1.15f;
	pyramid_meshes[6].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
	//Transform skybox
	skybox.get_transform().scale *= 1000;

	//Set terrain properties
	terrain_meshes[0].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	terrain_meshes[0].get_material().set_diffuse(vec4(0.1f, 0.1f, 0.1f, 1.0f));
	terrain_meshes[0].get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	terrain_meshes[0].get_material().set_shininess(0.5f);
	terrain_meshes[1].get_material().set_emissive(vec4(1.0f, 0.0f, 0.0f, 1.0f));
	terrain_meshes[1].get_material().set_diffuse(vec4(0.1f, 0.1f, 0.1f, 1.0f));
	terrain_meshes[1].get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	terrain_meshes[1].get_material().set_shininess(0.5f);
	//Set blade material properties
	model_meshes["blade"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	model_meshes["blade"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
	model_meshes["blade"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	model_meshes["blade"].get_material().set_shininess(30.0f);
	//Set Rock material properties
	model_meshes["rock"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	model_meshes["rock"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
	model_meshes["rock"].get_material().set_specular(vec4(0.1f, 0.1f, 0.1f, 1.0f));
	model_meshes["rock"].get_material().set_shininess(50.0f);
	//Set columns material properties
	model_meshes["column1"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	model_meshes["column1"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
	model_meshes["column1"].get_material().set_specular(vec4(0.7f, 0.7f, 0.7f, 1.0f));
	model_meshes["column1"].get_material().set_shininess(20.0f);
	model_meshes["column2"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	model_meshes["column2"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
	model_meshes["column2"].get_material().set_specular(vec4(0.7f, 0.7f, 0.7f, 1.0f));
	model_meshes["column2"].get_material().set_shininess(20.0f);
	//Set grave material properties
	model_meshes["grave"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	model_meshes["grave"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
	model_meshes["grave"].get_material().set_specular(vec4(0.1f, 0.1f, 0.1f, 1.0f));
	model_meshes["grave"].get_material().set_shininess(70.0f);
	//Set pyramids material properties
	for (int i = 0; i < 3; i++)
	{
		pyramid_meshes[i].get_material().set_emissive(vec4(0.0f, 3.0f, 0.0f, 1.0f));
		pyramid_meshes[i].get_material().set_diffuse(vec4(0.48f, 0.50f, 0.37f, 1.0f));
		pyramid_meshes[i].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
		pyramid_meshes[i].get_material().set_shininess(30.0f);
	}
	//Set torus material properties
	for (int i = 3; i < 7; i++)
	{
		pyramid_meshes[i].get_material().set_emissive(vec4(0.0f, 1.0f, 0.0f, 1.0f));
		pyramid_meshes[i].get_material().set_diffuse(vec4(0.48f, 0.50f, 0.37f, 1.0f));
		pyramid_meshes[i].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
		pyramid_meshes[i].get_material().set_shininess(30.0f);
	}

	//Set spot light for shadows properties
	shadow_light.set_position(vec3(450.0f, 60.0f, -52.0f));
	shadow_light.set_light_colour(vec4(0.5f, 0.5f, 0.5f, 1.0f));
	shadow_light.set_direction(vec3(-1.0f, -1.0f, 0.0f));
	shadow_light.set_range(1000.0f);
	shadow_light.set_power(5.0f);
	//Set point light properties
	gem_light.set_position(pyramid_meshes[0].get_transform().position);
	gem_light.move(vec3(0.0f,-1.8f,0.0f));
	gem_light.set_light_colour(vec4(0.0f, 1.0f, 0.0f, 1.0f));
	gem_light.set_range(50.0f);

	//Create cubemap
	array<string, 6> skyboxFiles =
	{
		"res/textures/skybox_ft.png", "res/textures/skybox_bk.png", "res/textures/skybox_up.png",
		"res/textures/skybox_dn.png", "res/textures/skybox_rt.png", "res/textures/skybox_lf.png",
	};
	cube_map = cubemap(skyboxFiles);

	//Skybox effect
	effects["sky_eff"].add_shader("res/shaders/skybox.vert", GL_VERTEX_SHADER);
	effects["sky_eff"].add_shader("res/shaders/skybox.frag", GL_FRAGMENT_SHADER);

	effects["sky_eff"].build();
	//Normal map effect
	effects["normal_map_eff"].add_shader("res/shaders/shader.vert", GL_VERTEX_SHADER);
	vector<string> normal_map_shaders{ "res/shaders/shader.frag", "res/shaders/normal_map.frag", "res/shaders/spot.frag", "res/shaders/point.frag","res/shaders/shadow.frag" };
	effects["normal_map_eff"].add_shader(normal_map_shaders, GL_FRAGMENT_SHADER);
	effects["normal_map_eff"].build();
	//Blend map effect
	effects["blend_eff"].add_shader("res/shaders/blend.vert", GL_VERTEX_SHADER);
	vector<string> blend_shaders{ "res/shaders/blend.frag", "res/shaders/spot.frag", "res/shaders/point.frag","res/shaders/shadow.frag" };
	effects["blend_eff"].add_shader(blend_shaders, GL_FRAGMENT_SHADER);
	effects["blend_eff"].build();
	//Shadow map effect
	effects["shadow_spot_eff"].add_shader("res/shaders/shadow_spot.vert",GL_VERTEX_SHADER);
	effects["shadow_spot_eff"].add_shader("res/shaders/shadow_spot.frag",GL_FRAGMENT_SHADER);
	effects["shadow_spot_eff"].build();
	//Shadow map effect for instancing
	effects["instancing_shadow_spot_eff"].add_shader("res/shaders/instancing_shadow_spot.vert", GL_VERTEX_SHADER);
	effects["instancing_shadow_spot_eff"].add_shader("res/shaders/instancing_shadow_spot.frag", GL_FRAGMENT_SHADER);
	effects["instancing_shadow_spot_eff"].build();
	//Instancing effect
	effects["instancing_eff"].add_shader("res/shaders/instancing.vert", GL_VERTEX_SHADER);
	vector<string> instancing_shaders{ "res/shaders/instancing.frag", "res/shaders/normal_map.frag", "res/shaders/spot.frag", "res/shaders/point.frag","res/shaders/shadow.frag" };
	effects["instancing_eff"].add_shader(instancing_shaders, GL_FRAGMENT_SHADER);
	effects["instancing_eff"].build();
	//Terrain effect
	effects["terrain_eff"].add_shader("res/shaders/terrain.vert", GL_VERTEX_SHADER);
	vector<string> terrain_shaders{"res/shaders/terrain.frag","res/shaders/part_weighted_texture_4.frag",  "res/shaders/spot.frag", "res/shaders/point.frag","res/shaders/shadow.frag" };
	effects["terrain_eff"].add_shader(terrain_shaders, GL_FRAGMENT_SHADER);
	effects["terrain_eff"].build();
	//Render without postprocessing effect
	effects["postprocessing_eff"].add_shader("res/shaders/basic_shader.vert", GL_VERTEX_SHADER);
	effects["postprocessing_eff"].add_shader("res/shaders/basic_shader.frag", GL_FRAGMENT_SHADER);
	effects["postprocessing_eff"].build();
	//Postprocessing effect
	effects["thermal_eff"].add_shader("res/shaders/basic_shader.vert", GL_VERTEX_SHADER);
	effects["thermal_eff"].add_shader("res/shaders/thermal_vision.frag", GL_FRAGMENT_SHADER);
	effects["thermal_eff"].build();

	//Load textures
	textures["plane_texture1"] = texture("res/textures/plane_texture1.jpg");
	textures["plane_texture2"] = texture("res/textures/plane_texture2.jpg");
	textures["plane_map"] = texture("res/textures/blend_map.png");
	textures["blade_texture"] = texture("res/models/Blade_Of_Olympus/texture.jpg");
	textures["blade_map"] = texture("res/models/Blade_Of_Olympus/normal_map.jpg");
	textures["rock_texture"] = texture("res/models/Rock/texture.jpg");
	textures["rock_map"] = texture("res/models/Rock/normal_map.jpg");
	textures["column_texture1"] = texture("res/textures/old_marble.jpg");
	textures["column_texture2"] = texture("res/textures/moss.jpg");
	textures["column_map"] = texture("res/textures/blend_map2.jpg");
	textures["grave_texture"] = texture("res/models/Grave/tex/texture.png");
	textures["grave_map"] = texture("res/models/Grave/tex/normal_map.png");
	textures["pyramid_texture"] = texture("res/textures/pyramid_texture.jpg");
	textures["pyramid_map"] = texture("res/textures/pyramid_map.jpg");
	textures["torus_texture"] = texture("res/textures/torus_texture.jpg");
	textures["torus_map"] = texture("res/textures/torus_map.jpg");
	textures["old_metal"] = texture("res/textures/old_metal.jpg");
	textures["lava_texture"] = texture("res/textures/lava_texture.png");
	textures["island_texture1"] = texture("res/textures/sand.jpg");
	textures["island_texture2"] = texture("res/textures/grass.jpg");
	textures["island_texture3"] = texture("res/textures/stone.jpg");
	textures["island_texture4"] = texture("res/textures/snow.jpg");
	textures["binoculars"] = texture("res/textures/binoculars_alphamap.jpg");
	textures["controllers"] = texture("res/textures/controllers.png");

	//Set free camera properties
	free_cam.set_position(vec3(10.0f, 100.0f, 225.0f));
	free_cam.set_target(vec3(0.0f, 0.0f, 0.0f));
	free_cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 2000.0f);
	//Set target camera properties
	target_cam.set_position(vec3(500.0f, 100.0f, 300.0f));
	target_cam.set_target(model_meshes["blade"].get_transform().position);
	target_cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 2000.0f);
	//Set arc ball camera properties
	arc_cam.set_target(pyramid_meshes[0].get_transform().position);
	arc_cam.set_distance(70.0f);
	arc_cam.translate(vec3(0.0f, 10.0f, 0.0f));
	arc_cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 2000.0f);

	//Generate semi-random model matrices for the instances
	srand(glfwGetTime());
	float radius = 100.0;
	float offset = 23.0f;
	for (int i = 0; i < model_matrices.size(); i++)
	{
		mat4 M;
		//Translate M 
		M = glm::translate(M, vec3(300.0f, 10.0f, -60.0f));
		float angle = (float)i / (float)model_matrices.size() * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		M = glm::translate(M, glm::vec3(x, 0, z));
		//Scale M
		M = glm::scale(M, model_meshes["blade"].get_transform().scale);
		//Rotate M
		float rotate = rand() % 3 + 1.5;
		M = glm::rotate(M, (-pi<float>() / rotate), vec3(1.0f, 0.0, 0.0f));
		model_matrices[i] = M;
	}
	//Bind VAO
	glBindVertexArray(model_meshes["blade"].get_geometry().get_array_object());
	//Generate VBO for instancing
	glGenBuffers(1, &position_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	//Enable VAO attributes to store the model matrix instances
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
	//Attributes change per instance
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
	glVertexAttribDivisor(8, 1);
	glBindVertexArray(0);

	return true;
}

bool update(float delta_time)
{
	//UV scroll for the lava movement
	uv_scroll += vec2(delta_time * 0.2, delta_time * 0.2);
	//Move the skybox with the camera
	skybox.get_transform().position = free_cam.get_position();
	//Rotate pyramid meshes
	pyramid_meshes[0].get_transform().rotate(vec3(0.0f, half_pi<float>()*delta_time, 0.0f));
	pyramid_meshes[3].get_transform().rotate(vec3(-half_pi<float>()*delta_time, 0.0f, 0.0f));
	pyramid_meshes[4].get_transform().rotate(vec3(-half_pi<float>()*delta_time, half_pi<float>()*delta_time, 0.0f));
	pyramid_meshes[5].get_transform().rotate(vec3(0.0f, 0.0f, -half_pi<float>()*delta_time));
	pyramid_meshes[6].get_transform().rotate(vec3(-half_pi<float>()*delta_time, -half_pi<float>()*delta_time, -half_pi<float>()*delta_time));

	//Thermal vision on
	if (glfwGetKey(renderer::get_window(), 'O'))
	{
		thermal_vision_on = true;
	}
	//Thermal vision off
	if (glfwGetKey(renderer::get_window(), 'F'))
	{
		thermal_vision_on = false;
	}
	// Press U to save
	if (glfwGetKey(renderer::get_window(), 'U') == GLFW_PRESS) {
		shadow_spot.buffer->save("test.png");
	}
	//Target camera
	updateTargetCamera(delta_time);
	//Free camera
	updateFreeCamera(delta_time);
	//Arc ball camera
	updateArcBallCamera(delta_time);
	//Update shadow map light position and direction
	shadow_spot.light_position = shadow_light.get_position();
	shadow_spot.light_dir = shadow_light.get_direction();

	return true;
}

bool render() {
	//View and projection matrices
	mat4 V;
	mat4 P;
	mat4 PV;
	if (camera_on._Equal("Free Camera"))
	{
		V = free_cam.get_view();
		P = free_cam.get_projection();
		PV = P * V;
	}
	else if (camera_on._Equal("Target Camera"))
	{
		V = target_cam.get_view();
		P = target_cam.get_projection();
		PV = P * V;
	}
	else if (camera_on._Equal("Arc Camera"))
	{
		V = arc_cam.get_view();
		P = arc_cam.get_projection();
		PV = P * V;
	}

	//Shadowing spot rendering
	spotShadowing();

	// Set render target to frame buffer
	renderer::set_render_target(frame);
	// Clear frame
	renderer::clear();

	//Skybox
	//Disable depth test,depth mask,face culling
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glCullFace(GL_FRONT);
	//Bind skybox effect
	renderer::bind(effects["sky_eff"]);
	//Calculate MVP for the skybox
	auto skyboxM = skybox.get_transform().get_transform_matrix();
	auto MVP = PV * skyboxM;
	//Set MVP matrix uniform
	glUniformMatrix4fv(effects["sky_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	//Set cubemap uniform
	renderer::bind(cube_map, 0);
	glUniform1i(effects["sky_eff"].get_uniform_location("cubemap"), 0);
	//Render skybox
	renderer::render(skybox);
	//Enable depth test,depth mask,face culling
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);

	//Render terrain
	for (int i = 0; i < terrain_meshes.size(); i++)
	{
		auto M = terrain_meshes[i].get_transform().get_transform_matrix();
		auto MVP = PV * M;
		setUniforms(MVP, terrain_meshes[i], effects["terrain_eff"]);
		array<texture, 4> terrain_textures;
		//Set UV scroll just for the lava, the shadows just for the island, and the corresponding textures
		if (i == 0)
		{
			glUniform2fv(effects["terrain_eff"].get_uniform_location("UV_SCROLL"), 1, value_ptr(vec2(0.0f, 0.0f)));
			glUniform1i(effects["terrain_eff"].get_uniform_location("shadows_on"), 1);
			terrain_textures[0] = textures["island_texture1"];
			terrain_textures[1] = textures["island_texture2"];
			terrain_textures[2] = textures["island_texture3"];
			terrain_textures[3] = textures["island_texture4"];

		}
		else if (i == 1)
		{
			glUniform2fv(effects["terrain_eff"].get_uniform_location("UV_SCROLL"), 1, value_ptr(uv_scroll));
			glUniform1i(effects["terrain_eff"].get_uniform_location("shadows_on"), 0);
			terrain_textures[0] = textures["lava_texture"];
			terrain_textures[1] = textures["lava_texture"];
			terrain_textures[2] = textures["lava_texture"];
			terrain_textures[3] = textures["lava_texture"];
		}
		setTerrainTextures(terrain_meshes[i], terrain_textures);
		renderer::render(terrain_meshes[i]);
	}

	// Render model meshes 
	for (auto &e : model_meshes)
	{
		auto m = e.second;
		//Create MVP
		auto M = m.get_transform().get_transform_matrix();
		auto MVP = PV * M;
		//Render blade
		if (m.get_geometry().get_array_object() == model_meshes["blade"].get_geometry().get_array_object())
		{
			setUniforms(MVP, m, effects["normal_map_eff"]);
			setNormalMappingTextures(m, textures["blade_texture"], textures["blade_map"]);
			glUniform1i(effects["normal_map_eff"].get_uniform_location("shadows_on"), 1);
		}
		//Render rock
		else if (m.get_geometry().get_array_object() == model_meshes["rock"].get_geometry().get_array_object())
		{
			setUniforms(MVP, m, effects["normal_map_eff"]);
			setNormalMappingTextures(m, textures["rock_texture"], textures["rock_map"]);
			glUniform1i(effects["normal_map_eff"].get_uniform_location("shadows_on"), 1);
		}
		//Render columns
		else if (m.get_geometry().get_array_object() == model_meshes["column1"].get_geometry().get_array_object() || m.get_geometry().get_array_object() == model_meshes["column2"].get_geometry().get_array_object())
		{
			array<texture, 2> blend_textures;
			blend_textures[0] = textures["column_texture1"];
			blend_textures[1] = textures["column_texture2"];
			setUniforms(MVP, m, effects["blend_eff"]);
			setBlendMappingTextures(m, blend_textures, textures["column_map"]);
		}
		//Render Grave
		else if (m.get_geometry().get_array_object() == model_meshes["grave"].get_geometry().get_array_object())
		{
			setUniforms(MVP, m, effects["normal_map_eff"]);
			setNormalMappingTextures(m, textures["grave_texture"], textures["grave_map"]);
			glUniform1i(effects["normal_map_eff"].get_uniform_location("shadows_on"), 1);
		}
		renderer::render(m);
	}

	//Render rotating torus around pyramids
	for (size_t i = 0; i < pyramid_meshes.size(); i++)
	{
		auto M = pyramid_meshes[i].get_transform().get_transform_matrix();
		//Apply hierarchy chain
		for (size_t j = i; j > 0; j--)
		{
			M = pyramid_meshes[j - 1].get_transform().get_transform_matrix() * M;
		}
		//Create MVP
		auto MVP = PV * M;
		//Render the two pyramids and the sphere(they are in positions 0, 1 and 2 of the array)
		if (i <= 2)
		{
			setUniforms(MVP, pyramid_meshes[i], effects["normal_map_eff"]);
			setNormalMappingTextures(pyramid_meshes[i], textures["pyramid_texture"], textures["pyramid_map"]);
			glUniform1i(effects["normal_map_eff"].get_uniform_location("shadows_on"), 0);
		}
		//Render the rotating torus
		else
		{
			setUniforms(MVP, pyramid_meshes[i], effects["normal_map_eff"]);
			setNormalMappingTextures(pyramid_meshes[i], textures["torus_texture"], textures["torus_map"]);
			glUniform1i(effects["normal_map_eff"].get_uniform_location("shadows_on"), 0);
		}
		renderer::render(pyramid_meshes[i]);
	}

	//Instance Rendering
	renderer::bind(effects["instancing_eff"]);
	//Set view and projection matrix independently as we need to use the model matrix of each instance and we also need the view matrix independently
	glUniformMatrix4fv(effects["instancing_eff"].get_uniform_location("V"), 1, GL_FALSE, value_ptr(V));
	glUniformMatrix4fv(effects["instancing_eff"].get_uniform_location("P"), 1, GL_FALSE, value_ptr(P));
	auto lightV = shadow_spot.get_view();
	mat4 LightProjectionMat = perspective<float>(90.f, renderer::get_screen_aspect(), 0.1f, 2000.f);
	mat4 lightPV = LightProjectionMat * lightV;
	glUniformMatrix4fv(effects["instancing_eff"].get_uniform_location("lightPV"), 1, GL_FALSE, value_ptr(lightPV));
	//Bind material
	renderer::bind(model_meshes["blade"].get_material(), "mat");
	//Bind spot light
	renderer::bind(shadow_light, "spot");
	//Bind point light
	renderer::bind(gem_light, "point");
	//Set eye position
	if (camera_on._Equal("Free Camera"))
	{
		glUniform3fv(effects["instancing_eff"].get_uniform_location("eye_pos"), 1, value_ptr(free_cam.get_position()));
	}
	if (camera_on._Equal("Target Camera"))
	{
		glUniform3fv(effects["instancing_eff"].get_uniform_location("eye_pos"), 1, value_ptr(target_cam.get_position()));
	}
	if (camera_on._Equal("Arc Camera"))
	{
		glUniform3fv(effects["instancing_eff"].get_uniform_location("eye_pos"), 1, value_ptr(arc_cam.get_position()));
	}
	//Bind texture
	renderer::bind(textures["old_metal"], 0);
	//Set tex uniform
	glUniform1i(effects["instancing_eff"].get_uniform_location("tex"), 0);
	//Bind normal_map
	renderer::bind(textures["blade_map"], 1);
	//Set normal_map uniform
	glUniform1i(effects["instancing_eff"].get_uniform_location("normal_map"), 1);
	// Bind shadow map texture
	renderer::bind(shadow_spot.buffer->get_depth(), 2);
	// Set the shadow_map uniform
	glUniform1i(effects["instancing_eff"].get_uniform_location("shadow_map"), 2);
	//Bind VAO
	glBindVertexArray(model_meshes["blade"].get_geometry().get_array_object());
	//Bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, position_vbo);
	//Load VBO data
	glBufferData(GL_ARRAY_BUFFER, model_matrices.size() * sizeof(glm::mat4), &model_matrices[0], GL_STATIC_DRAW);
	//Render instances
	glDrawElementsInstanced(GL_TRIANGLES, model_meshes["blade"].get_geometry().get_index_count(), GL_UNSIGNED_INT, 0, model_matrices.size());
	glBindVertexArray(0);

	//Set render target back to the screen
	renderer::set_render_target();
	//Render without thermal vision
	if (!thermal_vision_on)
	{
		postprocessingRendering(textures["controllers"], effects["postprocessing_eff"]);
	}
	//Render with thermal vision
	else
	{
		postprocessingRendering(textures["binoculars"], effects["thermal_eff"]);
	}

	return true;
}

void main() 
{
	app application("Graphics Coursework");
	application.set_load_content(load_content);
	application.set_initialise(initialise);
	application.set_update(update);
	application.set_render(render);
	application.run();
}