#version 330 core

layout (location = 0) in vec4 vertexData;

out vec2 textureCoords;

uniform mat4 modelMat;
uniform mat4 projectionMat;

uniform bool flipV;

void main()
{
	// Pass through the texture coord data
	if(flipV)
		textureCoords = vec2(vertexData.z, 1.0 - vertexData.w);
	else
		textureCoords = vec2(vertexData.z, vertexData.w);

	gl_Position = projectionMat * modelMat * vec4(vertexData.xy, 0.0, 1.0);
}