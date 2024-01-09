#version 330 core

out vec4 FragColor;

uniform sampler2D normalBuffer;
uniform sampler2D tangentBuffer;
uniform sampler2D binormalBuffer;

void main()
{
	vec4 normal   = texture(normalBuffer,   vec2(0.0, 0.0));
	vec4 tangent  = texture(tangentBuffer,  vec2(0.0, 0.0));
	vec4 binormal = texture(binormalBuffer, vec2(0.0, 0.0));

	FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}