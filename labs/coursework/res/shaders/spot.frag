// Spot light data
#ifndef SPOT_LIGHT
#define SPOT_LIGHT
struct spot_light 
{
  vec4 light_colour;
  vec3 position;
  vec3 direction;
  float constant;
  float linear;
  float quadratic;
  float power;
};
#endif

// Material data
#ifndef MATERIAL
#define MATERIAL
struct material 
{
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};
#endif

// Spot light calculation
vec4 calculate_spot(in spot_light spot, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour) {
  // Calculate direction to the light
  vec3 light_dir = normalize(spot.position - position);
  // Calculate distance to light
  float distance = distance(spot.position,position);
  // Calculate attenuation value
  float attenuation = spot.constant + (spot.linear*distance) + (spot.quadratic*pow(distance,2));
  // Calculate spot light intensity
  float light_intensity = pow(max(dot(-spot.direction,light_dir),0.0f),spot.power);
  // Calculate light colour
  vec4 light_colour = (light_intensity*spot.light_colour)/attenuation;
  // Now use standard phong shading but using calculated light colour and direction
  // - note no ambient
  float k_diffuse = max(dot(normal,light_dir),0.0f);
  vec4 diffuse = k_diffuse * (light_colour * mat.diffuse_reflection);
  vec3 half_vector = normalize(light_dir+view_dir);
  float k_specular = pow(max(dot(normal,half_vector),0.0f),mat.shininess);
  vec4 specular = k_specular * (light_colour * mat.specular_reflection);
  vec4 primary = mat.emissive + diffuse;
  vec4 colour = (primary*tex_colour) + specular;
  colour.a = 1.0f;
  return colour;
}