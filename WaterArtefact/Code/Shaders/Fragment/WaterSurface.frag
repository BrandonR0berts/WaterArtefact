#version 330 core

out vec4 FragColor;

// ----------------------------------------------------------------

uniform sampler2D normalBuffer;
uniform sampler2D tangentBuffer;
uniform sampler2D binormalBuffer;

uniform sampler2D positionalBuffer;

uniform vec3      cameraPosition;
uniform vec3      directionalLightDirection;

uniform samplerCube skyboxImage;

uniform vec3        ambientColour;

uniform bool        renderingSineGeneration;

// ----------------------------------------------------------------

in vec2 textureCoords;
in vec3 worldPosition;

// ----------------------------------------------------------------

vec3 unpackMappedValues(vec3 value)
{
	return vec3((value * 2.0) - 1.0);
}

// ----------------------------------------------------------------

void main()
{
	// Read data from buffers
	vec4 normal         = texture(normalBuffer,   textureCoords);
	vec3 unpackedNormal = normalize(unpackMappedValues(normal.xyz));

	vec4 readTangent  = texture(tangentBuffer,  textureCoords);
	vec4 readBinormal = texture(binormalBuffer, textureCoords);

	vec3 tangent  = unpackMappedValues(readTangent.xyz);
	vec3 binormal = unpackMappedValues(readBinormal.xyz);

	// Need to differentiate due to sine waves outputting their data in world space and gerstner waves in tangent space
	if(!renderingSineGeneration)
	{
		// Calculate the TBN matrix to convert from texture space into world space
		mat3 surfaceToWorldMatrix = mat3(tangent, binormal, normal);

		unpackedNormal = surfaceToWorldMatrix * unpackedNormal;
		tangent        = surfaceToWorldMatrix * tangent;
		binormal       = surfaceToWorldMatrix * binormal;
	}

	vec3 toPixel = normalize(worldPosition - cameraPosition);

	vec3 reflectedVector = reflect(toPixel, unpackedNormal);

	// Now find what it has reflected from the skybox
	vec4 reflectedColour = texture(skyboxImage, reflectedVector);



	//FragColor = vec4(ambientColour, 1.0);
	//FragColor = vec4(finalNormal.xyz, 1.0);
	//FragColor = vec4(normal.xyz, 1.0);
	FragColor = vec4(reflectedColour.rgb, 1.0);
	//FragColor = vec4(textureCoords.xy, 0.0, 1.0);
}

// ----------------------------------------------------------------