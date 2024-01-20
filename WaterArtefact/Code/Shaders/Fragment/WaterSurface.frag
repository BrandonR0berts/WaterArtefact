#version 330 core

out vec4 FragColor;

// ----------------------------------------------------------------

uniform vec3 cameraPosition;

uniform sampler2D normalBuffer;
uniform sampler2D tangentBuffer;
uniform sampler2D binormalBuffer;

uniform sampler2D positionalBuffer;

uniform samplerCube skyboxImage;

// ----------------------------------------------------------------

in vec2 textureCoords;
in vec4 fragCoord;

// ----------------------------------------------------------------

vec3 unpackMappedValues(vec4 value)
{
	return vec3((value * 2.0) - 1.0);
}

// ----------------------------------------------------------------

void main()
{
	vec4 normal      = texture(normalBuffer,   textureCoords);
	vec3 finalNormal = unpackMappedValues(normal);

	vec4 tangent  = texture(tangentBuffer,  textureCoords);
	vec4 binormal = texture(binormalBuffer, textureCoords);

	//vec4 position = texture(positionalBuffer, textureCoords);


	// Get the vector from the camera to the pixel
	vec3 toPixel = fragCoord.xyz - cameraPosition;

	// Reflect along normal
	vec3 reflectedVector = reflect(toPixel, finalNormal);

	// Now find what it has reflected from the skybox
	vec4 reflectedColour = texture(skyboxImage, reflectedVector);



	FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	//FragColor = vec4(finalNormal.xyz, 1.0);
	//FragColor = vec4(reflectedColour.rgb, 1.0);
	//FragColor = vec4(textureCoords.xy, 0.0, 1.0);
}

// ----------------------------------------------------------------