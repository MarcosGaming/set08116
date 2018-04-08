#version 440

// Projection matrix
uniform mat4 P;
// View matrix
uniform mat4 V;
// The light projection matrix
uniform mat4 lightPV;

//Incoming vertex position
layout (location = 0) in vec3 position;
// Incoming normal
layout(location = 2) in vec3 normal;
// Incoming binormal
layout(location = 3) in vec3 binormal;
// Incoming tangent
layout(location = 4) in vec3 tangent;
//Instance model matrix
layout (location = 5) in mat4 instance_matrix;
// Incoming texture coordinate
layout(location = 10) in vec2 tex_coord_in;

// Outgoing vertex position
layout(location = 0) out vec3 vertex_position;
// Outgoing texture coordinate
layout(location = 1) out vec2 tex_coord_out;
// Outgoing normal
layout(location = 2) out vec3 transformed_normal;
// Outgoing tangent
layout(location = 3) out vec3 tangent_out;
// Outgoing binormal
layout(location = 4) out vec3 binormal_out;
// Outgoing position in light space
layout (location = 5) out vec4 vertex_light;

void main()
{
	mat3 N = mat3(transpose(inverse(V * instance_matrix)));
	// Calculate screen position of vertex
	gl_Position = P * V * instance_matrix * vec4(position, 1.0);
	// Transform position into world space
    vertex_position = (instance_matrix * vec4(position, 1.0)).xyz;
	// Pass through texture coordinate
	tex_coord_out = tex_coord_in;
	// Transform normal
	transformed_normal = N * normal;
	// Transform tangent
	tangent_out = N * tangent;
	// Transform binormal
	binormal_out = N * binormal;
	// Transform position into light space
	vertex_light = lightPV * instance_matrix * vec4(position, 1.0f);
}