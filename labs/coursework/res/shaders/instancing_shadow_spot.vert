#version 440

// Projection matrix
uniform mat4 P;
// View matrix
uniform mat4 V;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 2) in vec3 normal;
//Instance model matrix
layout (location = 5) in mat4 instance_matrix;
// Incoming texture coordinate
layout(location = 10) in vec2 tex_coord_in;

// Outgoing position
layout(location = 0) out vec3 vertex_position;
// Outgoing transformed normal
layout(location = 1) out vec3 transformed_normal;
// Outgoing texture coordinate
layout(location = 2) out vec2 tex_coord_out;

void main() 
{
  mat3 N = mat3(transpose(inverse(V * instance_matrix)));
  // Calculate screen position
  gl_Position = P * V * instance_matrix * vec4(position, 1.0);
  // Output other values to fragment shader
  vertex_position = vec3(instance_matrix * vec4(position,1.0f));
  transformed_normal = N * normal;
  tex_coord_out = tex_coord_in;
}