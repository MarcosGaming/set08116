#version 440

//Uniform mvp
uniform mat4 MVP;

// Incoming value for the position
//What is that position?
layout(location = 0) in vec3 position;

// Main vertex shader function
void main() {
  // Calculate screen position of vertex
  gl_Position = MVP * vec4(position, 1.0);
}