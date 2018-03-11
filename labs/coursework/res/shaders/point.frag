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

// Point light calculation
vec4 calculate_point(in point_light point, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour) {
  // Get distance between point light and vertex
  float distance = distance(point.position,position);
  // Calculate attenuation factor
  float attenuation = point.constant + (point.linear*distance) + (point.quadratic*pow(distance,2));
  // Calculate light colour
  vec4 light_colour = point.light_colour/attenuation;
  // Calculate light dir
  vec3 light_dir = normalize(point.position - position);
  // Now use standard phong shading but using calculated light colour and direction
  float k_diffuse = max(dot(normal,light_dir),0.0f);
  vec4 diffuse = k_diffuse * (light_colour * mat.diffuse_reflection);
  vec3 half_vector = normalize(light_dir + view_dir);
  float k_specular = pow(max(dot(normal,half_vector),0.0f),mat.shininess);
  vec4 specular = k_specular * (light_colour * mat.specular_reflection);
  vec4 primary = mat.emissive + diffuse;
  vec4 colour = (primary * tex_colour) + specular;
  colour.a = 1.0f;
  return colour;
}