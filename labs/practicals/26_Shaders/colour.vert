#version 440

// Model view projection matrix, question: The term uniform is used to describe a variable that
//is the same for all the running versions of this shader. That means that is the same for all the vertex?Can you even declare other elements?
//Or if a shader is supposed to be a program like colour.vert can you create another one with a different mvp matrix?
//indices render framework?
uniform mat4 MVP;

// Incoming value for the position
//What is that position?
layout(location = 0) in vec3 position;

// Main vertex shader function
void main() {
  // Calculate screen position of vertex
  gl_Position = MVP * vec4(position, 1.0);
}