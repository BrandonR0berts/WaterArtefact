#version 330 core

layout (location = 0) in vec3 vertexPosition;

// Offset buffer provided by the water simulation
uniform sampler2D positionalBuffer;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;

void main()
{
	//texture(positionalBuffer, vec2(0.0, 0.0));

	gl_Position = projectionMat * modelMat * vec4(vertexPosition.xyz, 1.0);
}