#version 330 core

in  vec3 textureCoords;
out vec4 FragColor;

uniform samplerCube skyboxImage;

void main()
{
	FragColor = texture(skyboxImage, textureCoords);
}