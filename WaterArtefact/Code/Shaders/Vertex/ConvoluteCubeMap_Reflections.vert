#version 330 core

layout (location = 0) in vec3 vertexPosition;

out vec3 localPosition;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	localPosition = vertexPosition;
	gl_Position   = projectionMatrix * viewMatrix * vec4(localPosition, 1.0);
}