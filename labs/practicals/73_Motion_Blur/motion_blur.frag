#version 440 core

// Current frame being rendered
uniform sampler2D tex;
// Previous frame
uniform sampler2D previous_frame;

// Blend factor between frames
uniform float blend_factor;

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Sample the two textures
  vec4 tex_colour1 = texture(previous_frame, tex_coord);
  vec4 tex_colour2 = texture(tex, tex_coord);
  // Mix between these two colours
  colour = mix(tex_colour1,tex_colour2,blend_factor);
  // Ensure alpha is 1.0
  colour.a = 1.0f;
  // *********************************
}