#version 410

// Previous pass capture
uniform sampler2D tex;
// Initial render capture
uniform sampler2D sharp;
// Initial render depth capture
uniform sampler2D depth;

// The range value of the dof
uniform float range;
// The focus distance of the dof
uniform float focus;

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Sample sharp texture
  vec4 tex_colour1 = texture(sharp, tex_coord);
  // Sample blur texture
  vec4 tex_colour2 = texture(tex, tex_coord);
  // Calculate distance from the camera - based on depth sample
  float depth_sample = length(texture(depth, tex_coord));
  float distance = clamp(range * abs(focus - depth_sample), 0.0f, 1.0f);
  // Mix samples together based on distance
  colour = mix(tex_colour1, tex_colour2, distance);
  // Ensure alpha is 1.0
  colour.a = 1.0f;
  // *********************************
}