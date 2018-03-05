#version 440
// Directional light structure
#ifndef DIRECTIONAL_LIGHT
#define DIRECTIONAL_LIGHT
struct directional_light
{
	vec4 ambient_intensity;
	vec4 light_colour;
	vec3 light_dir;
};
#endif

// A material structure
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

// Calculates the directional light
vec4 calculate_direction(in directional_light light, in material mat, in vec3 normal, in vec3 view_dir, in vec4 tex_colour)
{
	// Calculate ambient component
	vec4 ambient = light.ambient_intensity + mat.diffuse_reflection;
	// Calculate diffuse component :  (diffuse reflection * light_colour) *  max(dot(normal, light direction), 0)
	float k_diffuse = max(dot(normal,light.light_dir),0.0f);
	vec4 diffuse = k_diffuse * (light.light_colour * mat.diffuse_reflection);
	// Calculate normalized half vector 
	vec3 half_vector = normalize(light.light_dir + view_dir);
	// Calculate specular component : (specular reflection * light_colour) * (max(dot(normal, half vector), 0))^mat.shininess
	float k_specular = pow(max(dot(normal,half_vector),0.0f),mat.shininess);
	vec4 specular = k_specular * (light.light_colour * mat.specular_reflection);
	// Calculate colour to return
	vec4 colour = ((mat.emissive + ambient + diffuse) * tex_colour) + specular;
	colour.a = 1.0;
	// Return colour
	return colour;

}
