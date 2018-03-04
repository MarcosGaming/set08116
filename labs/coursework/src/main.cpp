#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;

//Meshes
map<string, mesh> model_meshes;
array<mesh, 6> pyramid_meshes;
mesh skybox;
mesh plane;
//Textures
map<string, texture> textures;
cubemap cube_map;
//Effects
map<string, effect> effects;
//Lights
directional_light moon_light;
//Cameras
free_camera free_cam;
target_camera target_cam;
arc_ball_camera arc_cam;
//Additional variables
bool top = false;
string camera_on = "Free Camera";
double free_cam_cursor_x = 0.0;
double free_cam_cursor_y = 0.0;
double arc_cam_cursor_x = 0.0;
double arc_cam_cursor_y = 0.0;

//Render Methods
void normalMappingRendering(mat4 &MVP, mesh &m, texture &tex, texture &normal_map)
{
	renderer::bind(effects["normal_map_eff"]);
	//Set MVP uniform
	glUniformMatrix4fv(effects["normal_map_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	//Set M uniform
	glUniformMatrix4fv(effects["normal_map_eff"].get_uniform_location("M"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
	//Set N uniform
	glUniformMatrix3fv(effects["normal_map_eff"].get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
	//Bind material
	renderer::bind(m.get_material(), "mat");
	//Bind light
	renderer::bind(moon_light, "light");
	//Bind texture
	renderer::bind(tex, 0);
	//Set tex uniform
	glUniform1i(effects["normal_map_eff"].get_uniform_location("tex"), 0);
	//Bind normal_map
	renderer::bind(normal_map, 1);
	//Set normal_map uniform
	glUniform1i(effects["normal_map_eff"].get_uniform_location("normal_map"), 1);
	//Set eye position
	glUniform3fv(effects["normal_map_eff"].get_uniform_location("eye_pos"), 1, value_ptr(free_cam.get_position()));
}
void blendMappingRendering(mat4 &MVP, mesh &m, array<texture,2> &textures, texture &blend_map)
{
	//Bind effect
	renderer::bind(effects["blend_eff"]);
	//Set MVP uniform
	glUniformMatrix4fv(effects["blend_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	//Bind textures
	renderer::bind(textures[0], 0);
	renderer::bind(textures[1], 1);
	renderer::bind(blend_map, 2);
	//Set the uniform values for textures
	static int tex_indices[] = { 0, 1};
	glUniform1iv(effects["blend_eff"].get_uniform_location("tex"), 2, tex_indices);
	glUniform1i(effects["blend_eff"].get_uniform_location("blend"), 2);
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
	if (glfwGetKey(renderer::get_window(), '5'))
	{
		camera_on = "Free Camera";
		free_cam.set_target(vec3(0.0f, 50.0f, 0.0f));
	}
	free_cam.update(delta_time);
	free_cam_cursor_x = current_x;
	free_cam_cursor_y = current_y;
}
void updateTargetCamera(float delta_time)
{
	if (glfwGetKey(renderer::get_window(), '1'))
	{
		target_cam.set_position(vec3(225.0f, 10.0f, 225.0f));
		target_cam.set_target(vec3(0.0f, 50.0f, 0.0f));
		camera_on = "Target Camera";
	}
	if (glfwGetKey(renderer::get_window(), '2'))
	{
		target_cam.set_position(vec3(150.0f, 100.0f, -80.0f));
		target_cam.set_target(model_meshes["dragon"].get_transform().position + vec3(0.0f, 80.0f, -80.0f));
		camera_on = "Target Camera";
	}
	if (glfwGetKey(renderer::get_window(), '3'))
	{
		target_cam.set_position(vec3(-110.0f, 30.0f, 0.0f));
		target_cam.set_target(model_meshes["grave"].get_transform().position);
		camera_on = "Target Camera";
	}
	target_cam.update(delta_time);
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
	if (glfwGetKey(renderer::get_window(), '4'))
	{
		camera_on = "Arc Camera";
	}
	// Update the camera
	arc_cam.update(delta_time);
	// Update cursor pos
	arc_cam_cursor_x = current_x;
	arc_cam_cursor_y = current_y;
}


bool initialise() {
	//Hide cursor
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//Capture initial mouse position
	glfwGetCursorPos(renderer::get_window(), &free_cam_cursor_x, &free_cam_cursor_y);
	glfwGetCursorPos(renderer::get_window(), &arc_cam_cursor_x, &arc_cam_cursor_y);
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
  model_meshes["grave"] = mesh(geometry("res/models/Grave/grave.obj"));
  //Rotating torus around pyramids meshes
  pyramid_meshes[0] = mesh(geometry_builder::create_pyramid());
  pyramid_meshes[1] = mesh(geometry_builder::create_pyramid());
  pyramid_meshes[2] = mesh(geometry_builder::create_torus(30, 30, 0.5f, 8.0f));
  pyramid_meshes[3] = mesh(geometry_builder::create_torus(30, 30, 0.5f, 8.0f));
  pyramid_meshes[4] = mesh(geometry_builder::create_torus(30, 30, 0.5f, 8.0f));
  pyramid_meshes[5] = mesh(geometry_builder::create_torus(30, 30, 0.5f, 8.0f));

  //Transform model meshes
  model_meshes["rock"].get_transform().scale *= 10.0f;
  model_meshes["rock"].get_transform().translate(vec3(-220.0f, 20.0f, 8.0f));
  model_meshes["blade"].get_transform().scale *= 12.0f;
  model_meshes["blade"].get_transform().rotate(vec3(-half_pi<float>() / 1.5, 0.0f, 0.0f));
  model_meshes["blade"].get_transform().position = model_meshes["rock"].get_transform().position;
  model_meshes["blade"].get_transform().translate(vec3(12.0f, 6.0f, -5.0f));
  model_meshes["column1"].get_transform().scale /= 2.5f;
  model_meshes["column1"].get_transform().position = model_meshes["rock"].get_transform().position;
  model_meshes["column1"].get_transform().translate(vec3(5.0f, -20.0f, 20.0f));
  model_meshes["column2"].get_transform().scale /= 2.5f;
  model_meshes["column2"].get_transform().position = model_meshes["rock"].get_transform().position;
  model_meshes["column2"].get_transform().translate(vec3(5.0f, -20.0f, -47.0f));
  mat3x3 shearing = {
	  1.0f, 0.0f, 0.0f,
	  1.5f, 1.0f, 0.0f,
	  0.0f, 0.0f, 1.0f,
  };
  model_meshes["grave"].get_transform().scale = (shearing * model_meshes["grave"].get_transform().scale)/3.0f;
  model_meshes["grave"].get_transform().rotate(vec3(0.0f, half_pi<float>(), 0.0f));
  model_meshes["grave"].get_transform().position = model_meshes["rock"].get_transform().position;
  model_meshes["grave"].get_transform().translate(vec3(-15.0f, 5.0f, -14.0f));
  model_meshes["dragon"].get_transform().scale *= 50.0f;
  model_meshes["dragon"].get_transform().translate(vec3(50.0f, 35.0f, 50.0f));
  //Transform rotating torus and pyramid meshes
  pyramid_meshes[0].get_transform().scale *= 12.0f;
  pyramid_meshes[0].get_transform().position = vec3(0.0f,0.0f,0.0f);
  pyramid_meshes[0].get_transform().translate(vec3(5.0f, 30.0f, -7.0f));
  pyramid_meshes[0].get_transform().rotate(vec3(pi<float>(), 0.0f, 0.0f));
  pyramid_meshes[1].get_transform().translate(vec3(0.0f, -1.0f, 0.0f));
  pyramid_meshes[1].get_transform().rotate(vec3(pi<float>(), 0.0f, 0.0f));
  pyramid_meshes[2].get_transform().scale /= 7;
  pyramid_meshes[2].get_transform().translate(vec3(0.0f, -0.5f, 0.0f));
  pyramid_meshes[3].get_transform().scale *= 1.1;
  pyramid_meshes[3].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
  pyramid_meshes[4].get_transform().scale *= 1.15;
  pyramid_meshes[4].get_transform().rotate(vec3(half_pi<float>()/2.0f, 0.0f, 0.0f));
  pyramid_meshes[5].get_transform().scale *= 1.15;
  pyramid_meshes[5].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
  //Transform skybox
  skybox.get_transform().scale *= 500;
  skybox.get_transform().position = plane.get_transform().position;
  skybox.get_transform().translate(vec3(0.0f, 200.0f, 0.0f));
  //Transform plane
  plane.get_transform().scale *= 5;

  //Set plane material properties
  plane.get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  plane.get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
  plane.get_material().set_specular(vec4(0.5f, 0.5f, 0.5f, 0.5f));
  plane.get_material().set_shininess(50.0f);
  //Set blade material properties
  model_meshes["blade"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  model_meshes["blade"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
  model_meshes["blade"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  model_meshes["blade"].get_material().set_shininess(15.0f);
  //Set Rock material properties
  model_meshes["rock"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  model_meshes["rock"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
  model_meshes["rock"].get_material().set_specular(vec4(0.5f, 0.5f, 0.5f, 1.0f));
  model_meshes["rock"].get_material().set_shininess(50.0f);
  //Set columns material properties
  model_meshes["column1"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  model_meshes["column1"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
  model_meshes["column1"].get_material().set_specular(vec4(0.7f, 0.7f, 0.7f, 1.0f));
  model_meshes["column1"].get_material().set_shininess(5.0f);
  model_meshes["column2"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  model_meshes["column2"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
  model_meshes["column2"].get_material().set_specular(vec4(0.7f, 0.7f, 0.7f, 1.0f));
  model_meshes["column2"].get_material().set_shininess(50.0f);
  //Set grave material properties
  model_meshes["grave"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  model_meshes["grave"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
  model_meshes["grave"].get_material().set_specular(vec4(0.5f, 0.5f, 0.5f, 1.0f));
  model_meshes["grave"].get_material().set_shininess(50.0f);
  //Set dragon material properties
  model_meshes["dragon"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  model_meshes["dragon"].get_material().set_diffuse(vec4(0.53f, 0.45f, 0.37f, 1.0f));
  model_meshes["dragon"].get_material().set_specular(vec4(0.8f, 0.8f, 0.8f, 0.8f));
  model_meshes["dragon"].get_material().set_shininess(30.0f);
  //Set pyramids material properties
  for (int i = 0; i < 2; i++)
  {
	  pyramid_meshes[i].get_material().set_emissive(vec4(0.0f, 0.8f, 0.0f, 1.0f));
	  pyramid_meshes[i].get_material().set_diffuse(vec4(0.48f, 0.50f, 0.37f, 1.0f));
	  pyramid_meshes[i].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	  pyramid_meshes[i].get_material().set_shininess(15.0f);
  }
  //Set torus material properties
  for (int i = 2; i < 6; i++)
  {
	  pyramid_meshes[i].get_material().set_emissive(vec4(0.0f, 0.0f, 0.5f, 1.0f));
	  pyramid_meshes[i].get_material().set_diffuse(vec4(0.53f, 0.60f, 0.37f, 1.0f));
	  pyramid_meshes[i].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	  pyramid_meshes[i].get_material().set_shininess(30.0f);
  }

  //Set light properties
  moon_light.set_ambient_intensity(vec4(0.3f, 0.3f, 0.3f,1.0f));
  moon_light.set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  moon_light.set_direction(vec3(1.0f,1.0f,1.0f));

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
  effects["normal_map_eff"].add_shader("res/shaders/shader.frag", GL_FRAGMENT_SHADER);
  effects["normal_map_eff"].add_shader("res/shaders/normal_map.frag", GL_FRAGMENT_SHADER);
  effects["normal_map_eff"].add_shader("res/shaders/direction.frag", GL_FRAGMENT_SHADER);
  effects["normal_map_eff"].build();
  //Blend map effect
  effects["blend_eff"].add_shader("res/shaders/blend.vert", GL_VERTEX_SHADER);
  effects["blend_eff"].add_shader("res/shaders/blend.frag", GL_FRAGMENT_SHADER);
  effects["blend_eff"].build();

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
  textures["dragon_texture"] = texture("res/textures/dragon_texture.jpg");
  textures["dragon_map"] = texture("res/textures/dragon_map.jpg");

  //Set free camera properties
  free_cam.set_position(vec3(225.0f, 10.0f, 225.0f));
  free_cam.set_target(vec3(0.0f,50.0f,0.0f));
  free_cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
  //Set target camera properties
  target_cam.set_position(vec3(225.0f, 10.0f, 225.0f));
  target_cam.set_target(vec3(0.0f, 50.0f, 0.0f));
  target_cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
  //Set arc ball camera properties
  arc_cam.set_target(pyramid_meshes[0].get_transform().position);
  arc_cam.set_distance(70.0f);
  arc_cam.translate(vec3(0.0f, 10.0f, 0.0f));
  arc_cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

  return true;
}

bool update(float delta_time) 
{
	//Move pyramid meshes up and down
	if (pyramid_meshes[0].get_transform().position.y <= (plane.get_transform().position.y + 40.0f) && top == false)
	{
		pyramid_meshes[0].get_transform().translate(vec3(0.0f, 4.0f * delta_time, 0.0f));
		arc_cam.translate(vec3(0.0f, 4.0f * delta_time, 0.0f));
	}
	if(pyramid_meshes[0].get_transform().position.y >= (plane.get_transform().position.y + 40.0f) || top == true)
	{
		top = true;
		pyramid_meshes[0].get_transform().translate(vec3(0.0f, -4.0f * delta_time, 0.0f));
		arc_cam.translate(vec3(0.0f, -4.0f * delta_time, 0.0f));
	}
	if (pyramid_meshes[0].get_transform().position.y <= (plane.get_transform().position.y + 20.0f))
	{
		top = false;
	}
	//Rotate pyramid meshes
	pyramid_meshes[0].get_transform().rotate(vec3(0.0f, half_pi<float>()*delta_time, 0.0f));
	pyramid_meshes[2].get_transform().rotate(vec3(-half_pi<float>()*delta_time, 0.0f, 0.0f));
	pyramid_meshes[3].get_transform().rotate(vec3(-half_pi<float>()*delta_time, half_pi<float>()*delta_time, 0.0f));
	pyramid_meshes[4].get_transform().rotate(vec3(0.0f, 0.0f, -half_pi<float>()*delta_time));
	pyramid_meshes[5].get_transform().rotate(vec3(-half_pi<float>()*delta_time, -half_pi<float>()*delta_time, -half_pi<float>()*delta_time));
	
	//Target camera
	updateTargetCamera(delta_time);
	//Free camera
	updateFreeCamera(delta_time);
	//Arc ball camera
	updateArcBallCamera(delta_time);

	return true;
}

bool render() {
  //View and projection matrices
  mat4 V;
  mat4 P;
  if(camera_on._Equal("Free Camera"))
  {
	  V = free_cam.get_view();
	  P = free_cam.get_projection();
  }
  else if(camera_on._Equal("Target Camera"))
  {
	  V = target_cam.get_view();
	  P = target_cam.get_projection();
  }
  else if (camera_on._Equal("Arc Camera"))
  {
	  V = arc_cam.get_view();
	  P = arc_cam.get_projection();
  }
  
  //Skybox
  //Disable depth test,depth mask,face culling
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);
  glCullFace(GL_FRONT);
  //Bind skybox effect
  renderer::bind(effects["sky_eff"]);
  //Calculate MVP for the skybox
  auto skyboxM = skybox.get_transform().get_transform_matrix();
  auto MVP = P * V * skyboxM;
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

  //Plane
  //Create plane MVP
  auto planeM = plane.get_transform().get_transform_matrix();
  auto planeMVP = P * V * planeM;
  //Textures to use in the blend mapping
  array<texture, 2> plane_textures;
  plane_textures[0] = textures["plane_texture1"];
  plane_textures[1] = textures["plane_texture2"];
  //Render plane
  blendMappingRendering(planeMVP, plane, plane_textures, textures["plane_map"]);
  renderer::render(plane);

  // Render model meshes 
  for (auto &e : model_meshes)
  {
	  auto m = e.second;
	  //Create MVP
	  auto M = m.get_transform().get_transform_matrix();
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
		  array<texture, 2> blend_textures;
		  blend_textures[0] = textures["column_texture1"];
		  blend_textures[1] = textures["column_texture2"];
		  blendMappingRendering(MVP, m, blend_textures, textures["column_map"]);
	  }
	  //Render Grave
	  else if (m.get_geometry().get_array_object() == model_meshes["grave"].get_geometry().get_array_object())
	  {
		  normalMappingRendering(MVP, m, textures["grave_texture"], textures["grave_map"]);
	  }
	  //Render dragon
	  else if(m.get_geometry().get_array_object() == model_meshes["dragon"].get_geometry().get_array_object())
	  {
		  normalMappingRendering(MVP, m, textures["dragon_texture"], textures["dragon_map"]);
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
	  auto MVP = P * V * M;
	  //Render the two pyramids(they are in positions 0 and 1 of the array)
	  if (i == 0 || i == 1)
	  {
		  normalMappingRendering(MVP, pyramid_meshes[i], textures["pyramid_texture"], textures["pyramid_map"]);
	  }
	  //Render the rotating torus
	  else
	  {
		  normalMappingRendering(MVP, pyramid_meshes[i], textures["torus_texture"], textures["torus_map"]);
	  }
	  renderer::render(pyramid_meshes[i]);
  }

  return true;
}

void main() {
  app application("Graphics Coursework");
  application.set_load_content(load_content);
  application.set_initialise(initialise);
  application.set_update(update);
  application.set_render(render);
  application.run();
}