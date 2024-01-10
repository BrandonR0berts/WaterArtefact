#version 330 core

layout (location = 0) in vec2 vertexPosition;

// Offset buffer provided by the water simulation
uniform sampler2D positionalBuffer;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;

void main()
{
	//texture(positionalBuffer, vec2(0.0, 0.0));

	gl_Position = projectionMat * viewMat * modelMat * vec4(vertexPosition.x, 0.0, vertexPosition.y, 1.0);
}