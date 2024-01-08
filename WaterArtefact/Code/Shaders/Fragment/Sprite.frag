#version 330 core

in  vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D imageToRender;

void main()
{
	FragColor = texture(imageToRender, textureCoords);
}