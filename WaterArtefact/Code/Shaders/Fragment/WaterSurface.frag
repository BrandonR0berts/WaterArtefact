#version 330 core

out vec4 FragColor;

uniform sampler2D normalBuffer;
uniform sampler2D tangentBuffer;
uniform sampler2D binormalBuffer;

uniform sampler2D positionalBuffer;

in vec2 textureCoords;

void main()
{
	vec4 normal   = texture(normalBuffer,   textureCoords);
	vec4 tangent  = texture(tangentBuffer,  textureCoords);
	vec4 binormal = texture(binormalBuffer, textureCoords);

	vec4 position = texture(positionalBuffer, textureCoords);

	//FragColor = vec4(0.0, 1.0, 0.0, 1.0);
	FragColor = vec4(position.xy, 0.0, 1.0);
}