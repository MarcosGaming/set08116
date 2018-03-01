#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> model_meshes;
map<string, texture> textures;
map<string, effect> effects;
array<mesh, 6> pyramid_meshes;
mesh skybox;
mesh plane;
cubemap cube_map;
directional_light moon_light;
free_camera cam;
double cursor_x = 0.0;
double cursor_y = 0.0;


void normalMappingRendering(mat4 &MVP, mesh &m, texture &tex, texture &normal_map)
{
	renderer::bind(effects["normal_map_eff"]);
	// Set MVP matrix uniform
	glUniformMatrix4fv(effects["normal_map_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set M matrix uniform
	glUniformMatrix4fv(effects["normal_map_eff"].get_uniform_location("M"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
	// Set N matrix uniform
	glUniformMatrix3fv(effects["normal_map_eff"].get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
	// Bind material
	renderer::bind(m.get_material(), "mat");
	// Bind light
	renderer::bind(moon_light, "light");
	// Bind texture
	renderer::bind(tex, 0);
	// Set tex uniform
	glUniform1i(effects["normal_map_eff"].get_uniform_location("tex"), 0);
	// Bind normal_map
	renderer::bind(normal_map, 1);
	// Set normal_map uniform
	glUniform1i(effects["normal_map_eff"].get_uniform_location("normal_map"), 1);
	// Set eye position
	glUniform3fv(effects["normal_map_eff"].get_uniform_location("eye_pos"), 1, value_ptr(cam.get_position()));
}

bool initialise() {
	// Set input mode - hide the cursor
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Capture initial mouse position
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	// *********************************
	return true;
}

bool load_content() {
  //The plane for the scene
  plane = mesh(geometry_builder::create_plane());
  //Create skybox
  skybox = mesh(geometry_builder::create_box());
  //Rock and blade
  model_meshes["rock"] = mesh(geometry("res/models/Rock/Rock.obj"));
  model_meshes["blade"] = mesh(geometry("res/models/Blade_Of_Olympus/olympus.obj"));
  model_meshes["dragon"] = mesh(geometry("res/models/Dragon/dargon1.obj"));
  model_meshes["column1"] = mesh(geometry("res/models/column.obj"));
  model_meshes["column2"] = mesh(geometry("res/models/column.obj"));
  //Rotating torus around pyramids
  pyramid_meshes[0] = mesh(geometry_builder::create_pyramid());
  pyramid_meshes[1] = mesh(geometry_builder::create_pyramid());
  pyramid_meshes[2] = mesh(geometry_builder::create_torus(30, 30, 0.5f, 8.0f));
  pyramid_meshes[3] = mesh(geometry_builder::create_torus(30, 30, 0.5f, 8.0f));
  pyramid_meshes[4] = mesh(geometry_builder::create_torus(30, 30, 0.5f, 8.0f));
  pyramid_meshes[5] = mesh(geometry_builder::create_torus(30, 30, 0.5f, 8.0f));

  //Transform model meshes
  model_meshes["rock"].get_transform().scale *= 5;
  model_meshes["rock"].get_transform().translate(vec3(220.0f, 10.0f, 8.0f));
  model_meshes["blade"].get_transform().scale *= 5.0f;
  model_meshes["blade"].get_transform().rotate(vec3(-half_pi<float>() / 1.5, 0.0f, 0.0f));
  model_meshes["blade"].get_transform().position = model_meshes["rock"].get_transform().position;
  model_meshes["blade"].get_transform().translate(vec3(5.0f, 2.0f, -3.0f));
  model_meshes["column1"].get_transform().scale /= 4;
  model_meshes["column1"].get_transform().position = model_meshes["rock"].get_transform().position;
  model_meshes["column1"].get_transform().translate(vec3(5.0f, -10.0f, 20.0f));
  model_meshes["column2"].get_transform().scale /= 4;
  model_meshes["column2"].get_transform().position = model_meshes["rock"].get_transform().position;
  model_meshes["column2"].get_transform().translate(vec3(5.0f, -10.0f, -34.0f));
  model_meshes["dragon"].get_transform().scale *= 20;
  model_meshes["dragon"].get_transform().translate(vec3(-20.0f, -5.0f, -15.0f));
  //Transform pyramid meshes
  pyramid_meshes[0].get_transform().scale *= 8.0f;
  pyramid_meshes[0].get_transform().translate(vec3(-30.0f, 10.0f, 30.0f));
  pyramid_meshes[0].get_transform().rotate(vec3(pi<float>(), 0.0f, 0.0f));
  pyramid_meshes[1].get_transform().translate(vec3(0.0f, -1.0f, 0.0f));
  pyramid_meshes[1].get_transform().rotate(vec3(pi<float>(), 0.0f, 0.0f));
  pyramid_meshes[2].get_transform().scale /= 5;
  pyramid_meshes[2].get_transform().translate(vec3(0.0f, -0.5f, 0.0f));
  pyramid_meshes[3].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
  pyramid_meshes[4].get_transform().rotate(vec3(half_pi<float>()/2.0f, 0.0f, 0.0f));
  pyramid_meshes[5].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
  //Transform skybox
  skybox.get_transform().scale *= 500;
  skybox.get_transform().position = plane.get_transform().position;
  skybox.get_transform().translate(vec3(0.0f, -10.0f, 0.0f));
  //Transform plane
  plane.get_transform().scale *= 5;

  //Set blade material properties
  model_meshes["blade"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  model_meshes["blade"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
  model_meshes["blade"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  model_meshes["blade"].get_material().set_shininess(25.0f);
  //Set Rock material properties
  model_meshes["rock"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  model_meshes["rock"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
  model_meshes["rock"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  model_meshes["rock"].get_material().set_shininess(25.0f);
  // Set light properties
  moon_light.set_ambient_intensity(vec4(0.3f, 0.3f, 0.3f,1.0f));
  moon_light.set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  moon_light.set_direction(vec3(1.0f,1.0f,1.0f));

  //Create cubemap
  array<string, 6> skyboxFiles = 
  {
	  "res/textures/desert_night_ft.jpg", "res/textures/desert_night_bk.jpg", "res/textures/desert_night_up.jpg", 
	  "res/textures/desert_night_dn.jpg", "res/textures/desert_night_rt.jpg", "res/textures/desert_night_lf.jpg",
  };
  cube_map = cubemap(skyboxFiles);

  //Standard effect
  effects["eff"].add_shader("res/shaders/basic_textured.vert", GL_VERTEX_SHADER);
  effects["eff"].add_shader("res/shaders/basic_textured.frag", GL_FRAGMENT_SHADER);
  effects["eff"].build();
  //Skybox effect
  effects["sky_eff"].add_shader("res/shaders/skybox.vert", GL_VERTEX_SHADER);
  effects["sky_eff"].add_shader("res/shaders/skybox.frag", GL_FRAGMENT_SHADER);
  effects["sky_eff"].build();
  //Normal map effect
  effects["normal_map_eff"].add_shader("res/shaders/shader.vert", GL_VERTEX_SHADER);
  effects["normal_map_eff"].add_shader("res/shaders/shader.frag", GL_FRAGMENT_SHADER);
  effects["normal_map_eff"].add_shader("res/shaders/normal_map.frag", GL_FRAGMENT_SHADER);
  effects["normal_map_eff"].add_shader("res/shaders/direction.frag", GL_FRAGMENT_SHADER);
  effects["normal_map_eff"].build();
  //Blended effect
  effects["blend_eff"].add_shader("res/shaders/blend.vert", GL_VERTEX_SHADER);
  effects["blend_eff"].add_shader("res/shaders/blend.frag", GL_FRAGMENT_SHADER);
  effects["blend_eff"].build();

  //Load textures
  textures["tex"] = texture("res/textures/check_1.png");
  textures["blade_texture"] = texture("res/models/Blade_Of_Olympus/texture.jpg");
  textures["blade_map"] = texture("res/models/Blade_Of_Olympus/normal_map.jpg");
  textures["rock_texture"] = texture("res/models/Rock/texture.jpg");
  textures["rock_map"] = texture("res/models/Rock/normal_map.jpg");
  textures["column_texture"] = texture("res/textures/marble.jpg");
  textures["column_map"] = texture("res/textures/column_map.jpg");

  // Set camera properties
  cam.set_position(vec3(50.0f, 10.0f, 50.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
  return true;
}

bool top = false;
bool update(float delta_time) {
	//Move and rotate pyramid meshes
	if (pyramid_meshes[0].get_transform().position.y <= 15.0f && top == false)
	{
		pyramid_meshes[0].get_transform().translate(vec3(0.0f, 2 * delta_time, 0.0f));
	}
	if(pyramid_meshes[0].get_transform().position.y >= 15.0f || top == true)
	{
		top = true;
		pyramid_meshes[0].get_transform().translate(vec3(0.0f, -2 * delta_time, 0.0f));
	}
	if (pyramid_meshes[0].get_transform().position.y < 10.0f)
	{
		top = false;
	}
	pyramid_meshes[0].get_transform().rotate(vec3(0.0f, half_pi<float>()*delta_time, 0.0f));
	pyramid_meshes[2].get_transform().rotate(vec3(-half_pi<float>()*delta_time, 0.0f, 0.0f));
	pyramid_meshes[3].get_transform().rotate(vec3(-half_pi<float>()*delta_time, half_pi<float>()*delta_time, 0.0f));
	pyramid_meshes[4].get_transform().rotate(vec3(0.0f, 0.0f, -half_pi<float>()*delta_time));
	pyramid_meshes[5].get_transform().rotate(vec3(-half_pi<float>()*delta_time, -half_pi<float>()*delta_time, -half_pi<float>()*delta_time));
	
	//Free camera
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height =
		(quarter_pi<float>() *
		(static_cast<float>(renderer::get_screen_height()) / static_cast<float>(renderer::get_screen_width()))) /
		static_cast<float>(renderer::get_screen_height());

	double current_x;
	double current_y;
	// *********************************
	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	float delta_x = current_x - cursor_x;
	float delta_y = current_y - cursor_y;
	// Multiply deltas by ratios - gets actual change in orientation
	delta_x *= ratio_width;
	delta_y *= ratio_height;
	// Rotate cameras by delta
	// delta_y - x-axis rotation
	// delta_x - y-axis rotation
	cam.rotate(delta_x, -delta_y);
	// Use keyboard to move the camera - WSAD
	if (glfwGetKey(renderer::get_window(), 'W'))
	{
		cam.move(vec3(0.0f, 0.0f, 2.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'S'))
	{
		cam.move(vec3(0.0f, 0.0f, -2.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'A'))
	{
		cam.move(vec3(-2.0f, 0.0f, 0.0f));
	}
	if (glfwGetKey(renderer::get_window(), 'D'))
	{
		cam.move(vec3(2.0f, 0.0f, 0.0f));
	}
	// Update the camera
	cam.update(delta_time);
	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;
	// *********************************
	return true;
}

bool render() {
  //View and projection matrices
  auto V = cam.get_view();
  auto P = cam.get_projection();

  //Render skybox
  // Disable depth test,depth mask,face culling
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glCullFace(GL_FRONT);
  // Bind skybox effect
  renderer::bind(effects["sky_eff"]);
  // Calculate MVP for the skybox
  auto skyboxM = skybox.get_transform().get_transform_matrix();
  auto MVP = P * V * skyboxM;
  // Set MVP matrix uniform
  glUniformMatrix4fv(effects["sky_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  // Set cubemap uniform
  renderer::bind(cube_map, 0);
  glUniform1i(effects["sky_eff"].get_uniform_location("cubemap"), 0);
  // Render skybox
  renderer::render(skybox);
  // Enable depth test,depth mask,face culling
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glCullFace(GL_BACK);


  // Bind effect
  renderer::bind(effects["eff"]);
  //Bind texture 
  renderer::bind(textures["tex"], 0);

  //Render plane
  auto planeM = plane.get_transform().get_transform_matrix();
  auto planeMVP = P * V * planeM;
  glUniformMatrix4fv(effects["eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(planeMVP));
  glUniform1i(effects["eff"].get_uniform_location("tex"), 0);
  renderer::render(plane);

  // Render standard meshes 
  for (auto &e : model_meshes)
  {
	  auto m = e.second;
	  //Create MVP
	  auto M = m.get_transform().get_transform_matrix();
	  auto V = cam.get_view();
	  auto P = cam.get_projection();
	  auto MVP = P * V * M;
	  //Render blade
	  if (m.get_geometry().get_array_object() == model_meshes["blade"].get_geometry().get_array_object())
	  {
		  normalMappingRendering(MVP, m, textures["blade_texture"], textures["blade_map"]);
	  }
	  //Render rock
	  else if (m.get_geometry().get_array_object() == model_meshes["rock"].get_geometry().get_array_object())
	  {
		  normalMappingRendering(MVP, m, textures["rock_texture"], textures["rock_map"]);
	  }
	  //Render columns
	  else if(m.get_geometry().get_array_object() == model_meshes["column1"].get_geometry().get_array_object() || m.get_geometry().get_array_object() == model_meshes["column2"].get_geometry().get_array_object())
	  {
		  normalMappingRendering(MVP, m, textures["column_texture"], textures["column_map"]);
	  }
	  else
	  {
		  renderer::bind(effects["eff"]);
		  renderer::bind(textures["tex"], 0);
		  // Set MVP matrix uniform
		  glUniformMatrix4fv(effects["eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		  //Set texture
		  glUniform1i(effects["eff"].get_uniform_location("tex"), 0);
	  }
	  // Render geometry
	  renderer::render(m);
  }

  renderer::bind(effects["eff"]);
  renderer::bind(textures["tex"], 0);
  // Render rotating torus around pyramids
  for (size_t i = 0; i < pyramid_meshes.size(); i++)
  {
	  auto M = pyramid_meshes[i].get_transform().get_transform_matrix();
	  //Apply hierarchy chain
	  for (size_t j = i; j > 0; j--)
	  {
		  M = pyramid_meshes[j - 1].get_transform().get_transform_matrix() * M;
	  }
	  auto MVP = P * V * M;
	  // Set MVP matrix uniform
	  glUniformMatrix4fv(effects["eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	  //Set texture
	  glUniform1i(effects["eff"].get_uniform_location("tex"), 0);
	  // Render geometry
	  renderer::render(pyramid_meshes[i]);
  }

  return true;
}

void main() {
  // Create application
  app application("Graphics Coursework");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_initialise(initialise);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}