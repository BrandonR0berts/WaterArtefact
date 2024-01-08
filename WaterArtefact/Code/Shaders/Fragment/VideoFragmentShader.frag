#version 330 core

in  vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D imageToRender; // Float image reading

void main()
{
	FragColor = texture(imageToRender, textureCoords);
}