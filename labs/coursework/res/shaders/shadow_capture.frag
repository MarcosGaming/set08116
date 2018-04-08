#version 440

// Incoming vertex position
layout (location = 0) in vec4 position;

// Outgoing pixel colour
layout (location = 0) out vec4 colour;

void main()
{
	float depth = position.z / position.w ;
	depth = depth * 0.5 + 0.5;			

	float moment1 = depth;
	float moment2 = depth * depth;

	float dx = dFdx(depth);
	float dy = dFdy(depth);
	moment2 += 0.25*(dx*dx+dy*dy);

	colour = vec4( moment1,moment2, 0.0, 0.0 );
}