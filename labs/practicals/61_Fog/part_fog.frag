#version 440
// Types of fog
#ifndef FOG_TYPES
#define FOG_TYPES
#define FOG_LINEAR 0
#define FOG_EXP 1
#define FOG_EXP2 2
#endif

float calculate_fog(in float fog_coord, in vec4 fog_colour, in float fog_start, in float fog_end, in float fog_density,
                    in int fog_type) {
  // Fog value to return
  float result = 0.0;

  // Calculate fog
  if (fog_type == FOG_LINEAR) {
    // Linear fog
    result = (fog_end - fog_coord) / (fog_end - fog_start);
  } else if (fog_type == FOG_EXP) {
    // *********************************
    // Exponential fog
	  result = exp(-fog_density * fog_coord);
    // *********************************
  } else if (fog_type == FOG_EXP2) {
    // *********************************
    // Exponential squared fog
	  result = exp(-(pow((fog_density*fog_coord), 2)));
    // *********************************
  }
  // *********************************
  // Result is 1 minus result clamped to 1.0 to 0.0
  result = clamp(result, 0.0f, 1.0f);
  // *********************************
  return result;
}