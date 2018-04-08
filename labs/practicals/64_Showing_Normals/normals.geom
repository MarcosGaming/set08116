#version 440

// MVP transformation
uniform mat4 MVP;

// Layout of incoming data
layout(triangles) in;
// Layout of outgoing data
layout(line_strip, max_vertices = 6) out;

// Incoming normals for vertexs
layout(location = 0) flat in vec3 normal[];

void main() {
  // Calculate for each vertex
  for (int i = 0; i < 3; ++i) {
    // *********************************
    // Ensure normal is normalized
	normalize(normal[i]);

    // Output normal position for start of line
    // - remember to transform
	gl_Position =  MVP * gl_in[i].gl_Position;;
    // Emit
	EmitVertex();
    // Output position + normal for end of line
    // - remember to transform
	gl_Position = MVP * (gl_in[i].gl_Position + vec4(normal[i],0.0f));
    // Emit
	EmitVertex();
    // End the primitive
	EndPrimitive();
    // *********************************
  }
}