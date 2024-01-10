#version 330 core

layout (location = 0) in vec2 vertexPosition;

// Offset buffer provided by the water simulation
uniform sampler2D positionalBuffer;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projectionMat;

// Used for texture coord calculations
uniform float maxDistanceFromOrigin;

out vec2 textureCoords;

void main()
{
	// Calculate texture coords based off of world position
	vec2 offsettedVertexPosition = vertexPosition;
	offsettedVertexPosition.x += maxDistanceFromOrigin;
	offsettedVertexPosition.y += maxDistanceFromOrigin;

	// We now have the vertex position in the range 0 -> dimensions * distance between verticies
	// We need to convert that position into a 0 -> 1 range
	float totalDistance = maxDistanceFromOrigin * 2.0;
	textureCoords = vec2(offsettedVertexPosition.x / totalDistance, offsettedVertexPosition.y / totalDistance);

	vec4 position = texture(positionalBuffer, textureCoords);

	gl_Position = projectionMat * viewMat * modelMat * vec4(vertexPosition.x, position.x, vertexPosition.y, 1.0);
}