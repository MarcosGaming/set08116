#version 440

// Spot light data
#ifndef SPOT_LIGHT
#define SPOT_LIGHT
struct spot_light {
  vec4 light_colour;
  vec3 position;
  vec3 direction;
  float constant;
  float linear;
  float quadratic;
  float power;
};
#endif

// Point light information
#ifndef POINT_LIGHT
#define POINT_LIGHT
struct point_light {
  vec4 light_colour;
  vec3 position;
  float constant;
  float linear;
  float quadratic;
};
#endif

// A material structure
#ifndef MATERIAL
#define MATERIAL
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};
#endif

// Forward declarations of used functions
vec4 calculate_spot(in spot_light spot, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour);
vec3 calc_normal(in vec3 normal, in vec3 tangent, in vec3 binormal, in sampler2D normal_map, in vec2 tex_coord);
vec4 calculate_point(in point_light point, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour);
float calculate_shadow(in sampler2D shadow_map, in vec4 light_space_pos, in vec3 normal, in spot_light spot, in vec3 position);

// Point light
uniform point_light point;
// Spot light 
uniform spot_light spot;
// Material of the object being rendered
uniform material mat;
// Position of the eye
uniform vec3 eye_pos;
// Texture to sample from
uniform sampler2D tex;
// Normal map to sample from
uniform sampler2D normal_map;
// Shadow map to sample from
uniform sampler2D shadow_map;

// Incoming vertex position
layout(location = 0) in vec3 position;
// Incoming texture coordinate
layout(location = 1) in vec2 tex_coord;
// Incoming normal
layout(location = 2) in vec3 normal;
// Incoming tangent
layout(location = 3) in vec3 tangent;
// Incoming binormal
layout(location = 4) in vec3 binormal;
// Incoming light space position
layout(location = 5) in vec4 light_space_pos;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // Sample texture
	vec4 tex_colour = texture(tex, tex_coord);
  // Calculate view direction
	vec3 view_dir = normalize(eye_pos - position);
  // Calculate normal from normal map
	vec3 map_normal = calc_normal(normal, tangent, binormal, normal_map, tex_coord);
  // Calculate spot light and point light
	colour += calculate_spot(spot, mat, position, map_normal, view_dir, tex_colour);
	colour += calculate_point(point, mat, position, map_normal, view_dir, tex_colour);
	// Calculate shade factor
	float shade_factor = calculate_shadow(shadow_map, light_space_pos, map_normal, spot, position);
	// Scale colour by shade
    colour *= shade_factor;
	colour.a = 1.0f;
}