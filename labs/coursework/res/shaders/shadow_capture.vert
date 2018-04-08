#version 440

// Model view projection matrix
uniform mat4 MVP;

//Incoming vertex position
layout (location = 0) in vec3 position;

//Outgoing vertex position
layout (location = 0) out vec4 vertex_position;

void main()
{
		gl_Position = MVP * vec4(position, 1.0);;
		vertex_position = gl_Position;
}