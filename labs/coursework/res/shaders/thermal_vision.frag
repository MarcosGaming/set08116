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
  //Calculate thermal colour
  vec3 tc = vec3(1.0, 0.0, 0.0);
  vec3 pixcol = texture(tex, tex_coord).rgb;
  vec3 colors[3];
  colors[0] = vec3(0.,0.,1.);
  colors[1] = vec3(1.,1.,0.);
  colors[2] = vec3(1.,0.,0.);
  float lum = (pixcol.r+pixcol.g+pixcol.b)/3.;
  int ix = (lum < 0.5)? 0:1;
  tc = mix(colors[ix],colors[ix+1],(lum-float(ix)*0.5)/0.5);
  vec4 thermal_colour = vec4(tc, 1.0);
  //Mix thermal colour with alpha map
  vec4 alpha_map_colour = texture(alpha_map,tex_coord);
  out_colour = thermal_colour * alpha_map_colour;
  out_colour.a = 1;
}