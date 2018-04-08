#version 440

// Sampler used to get texture colour
uniform sampler2D tex;
// Alpha map
uniform sampler2D alpha_map;


// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;
// Outgoing colour
layout(location = 0) out vec4 out_colour;

void main() 
{
  // Sample textures
  vec4 tex_colour1 = texture(tex,tex_coord);
  vec4 tex_colour2 = texture(alpha_map, tex_coord);
  // Mix textures for colour
  out_colour = tex_colour1 * tex_colour2;
  //Ensure colour.a is 1
  out_colour.a = 1;
}