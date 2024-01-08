#version 330 core

layout (location = 0) in vec3 position;

out vec3 textureCoords;

uniform mat4 projectionMat;
uniform mat4 viewMat;

void main()
{
	textureCoords = position;
	vec4    pos   = projectionMat * viewMat * vec4(position, 1.0);

	// this is done so that when the divide by w is done, the z component (depth part) will always equal 1 and fail the depth test if something is already there
	// Taking full advantage of early out depth testing that is built into hardware
	gl_Position   = pos.xyww;
}