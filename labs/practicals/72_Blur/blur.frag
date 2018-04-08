#version 430 core

// Incoming frame data
uniform sampler2D tex;

// 1.0f / screen width
uniform float inverse_width;
// 1.0f / screen height
uniform float inverse_height;

// Surrounding pixels to sample and their scale
const vec4 samples[4] = vec4[4](vec4(-1.0, 0.0, 0.0, 0.25), vec4(1.0, 0.0, 0.0, 0.25), vec4(0.0, 1.0, 0.0, 0.25),vec4(0.0, -1.0, 0.0, 0.25));
const vec4 edge_detection_samples[6] = vec4[6](vec4(-1.0f,1.0f,0.0f,1.0f),vec4(0.0f,1.0f,0.0f,2.0f),vec4(1.0f,1.0f,0.0f,1.0f),vec4(-1.0f,-1.0f,0.0f,-1.0f),vec4(0.0f,-1.0f,0.0f,-2.0f), vec4(1.0f,-1.0f,0.0f,-1.0f));
const vec4 sharpening_samples[5] = vec4[5](vec4(0.0, 0.0, 0.0, 11.0 / 3), vec4(-1.0, 0.0, 0.0, -2.0/3), vec4(1.0, 0.0, 0.0, -2.0/3), vec4(0.0, 1.0, 0.0, -2.0/3), vec4(0.0, -1.0, 0.0, -2.0/3)); 
const vec3 gaussian_samples[7] = vec3[7](vec3(-3.0, 0.0, 1.0 / 64), vec3(-2.0, 0.0, 6.0 / 64), vec3(-1.0, 0.0, 15.0 / 64), vec3(0.0, 0.0, 20.0 / 64), vec3(1.0, 0.0, 15.0 / 64), vec3(2.0, 0.0, 6.0 / 64), vec3(3.0, 0.0, 1.0 / 64));
const vec3 gaussian_samples2[7] = vec3[7](vec3(0.0, -3.0, 1.0 / 64), vec3(0.0, -2.0, 6.0 / 64), vec3(0.0, -1.0, 15.0 / 64), vec3(0.0, 0.0, 20.0 / 64), vec3(0.0, 1.0, 15.0 / 64), vec3(0.0, 2.0, 6.0 / 64), vec3(0.0, 3.0, 1.0 / 64));

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;

// Outgoing colour
layout(location = 0) out vec4 colour;

void main() {
  // *********************************
  // Start with colour as black
  colour = vec4(0.0f,0.0f,0.0f,1.0f);
  // Loop through each sample vector
 /* for(int i = 0; i < 4; i++)
  {
	// Calculate tex coord to sample
	vec2 uv = tex_coord + vec2(samples[i].x * inverse_width, samples[i].y * inverse_height);
    // Sample the texture and scale appropriately
    // - scale factor stored in w component
	vec4 tex_colour = texture(tex, uv);
	tex_colour *= samples[i].w;
	colour += tex_colour;
  }*/
  //Gaussian BLUR
  for (int i = 0; i < 7; ++i)
  {
	  vec2 uv = tex_coord + vec2(gaussian_samples[i].x * inverse_width, gaussian_samples[i].y * inverse_height);

	  vec4 tex_col = texture(tex, uv);
	  tex_col *= gaussian_samples[i].z;

	  colour += tex_col;
  }
  for (int i = 0; i < 7; ++i)
  {
	  vec2 uv = tex_coord + vec2(gaussian_samples2[i].x * inverse_width, gaussian_samples2[i].y * inverse_height);

	  vec4 tex_col = texture(tex, uv);
	  tex_col *= gaussian_samples2[i].z;

	  colour += tex_col;
  }
  // Ensure alpha is 1.0
  colour.a = 1.0f;
  // *********************************
}