#version 440
//QUESTION : Is this the correct way of blending four textures?
// Main textures
uniform sampler2D tex[4];
// Blend map
uniform sampler2D blend[3];

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;
// Outgoing fragment colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Sample the two main textures
  vec4 colour1 = texture(tex[0],tex_coord);
  vec4 colour2 = texture(tex[1],tex_coord);
  vec4 colour3 = texture(tex[2],tex_coord);
  vec4 colour4 = texture(tex[3],tex_coord);
  // Sample the blend texture
  vec4 blend_map1 = texture(blend[0],tex_coord);
  vec4 blend_map2 = texture(blend[1],tex_coord);
  vec4 blend_map3 = texture(blend[2],tex_coord);
  // Mix the main samples using r component from blend value	Question : I cant see any difference from using different colour values(rgb or alpha)
  vec4 colour12 = mix(colour1,colour2,blend_map1.r);
  vec4 colour34 = mix(colour3,colour4,blend_map2.r);
  colour = mix(colour12,colour34,blend_map3.r);
  // *********************************
}