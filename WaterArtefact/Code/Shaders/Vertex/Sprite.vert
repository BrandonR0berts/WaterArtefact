#version 330 core

layout (location = 0) in vec4 vertexData;

out vec2 textureCoords;

uniform mat4 projectionMatrix;

uniform vec2 position;
uniform vec2 size;

void main()
{
	// Pass through the texture coord data
	textureCoords = vec2(vertexData.z, 1.0 - vertexData.w);

	vec2 finalPosition    = position;
	     finalPosition.x += vertexData.x * size.x;
	     finalPosition.y += vertexData.y * size.y;

	gl_Position = projectionMatrix * vec4(finalPosition.xy, 0.0, 1.0);
}