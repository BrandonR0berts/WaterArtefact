#version 330 core

out vec4 FragColor;

// ----------------------------------------------------------------

uniform sampler2D normalBuffer;
uniform sampler2D tangentBuffer;
uniform sampler2D binormalBuffer;

uniform sampler2D positionalBuffer;

// ----------------------------------------------------------------

// Eye position
uniform vec3        cameraPosition;

// Sun direction
uniform vec3        directionalLightDirection;

// Skybox being reflected
uniform samplerCube skyboxImage;

// How clear the reflection of the sky is visible in the water
uniform float       reflectionProportion;

// The colour of the water
uniform vec3        waterColour;

// The ambient brightness of the ocean
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

vec3 CalculateDiffuse(vec3 normal)
{
	return vec3(max(dot(normal, directionalLightDirection), 0.0));
}

// ----------------------------------------------------------------

vec3 CalculateSpecular(vec3 normal, vec3 viewDirection)
{
	float specularStrength = 0.5;

	vec3 reflectDirection = reflect(-directionalLightDirection, normal);
	float specularPortion = pow(max(dot(viewDirection, reflectDirection), 0.0), 64);
	
	return vec3(specularPortion * specularStrength);
}

// ----------------------------------------------------------------

void main()
{
	// ----------------------------------------------------------------

	// Read data from buffers
	vec4 normal         = texture(normalBuffer,   textureCoords);
	vec3 unpackedNormal = normalize(unpackMappedValues(normal.xyz));

	vec4 readTangent    = texture(tangentBuffer,  textureCoords);
	vec4 readBinormal   = texture(binormalBuffer, textureCoords);

	vec3 tangent        = unpackMappedValues(readTangent.xyz);
	vec3 binormal       = unpackMappedValues(readBinormal.xyz);

	// ----------------------------------------------------------------

	// Need to differentiate due to sine waves outputting their data in world space and gerstner waves in tangent space
	if(!renderingSineGeneration)
	{
		// Calculate the TBN matrix to convert from texture space into world space
		mat3 surfaceToWorldMatrix = mat3(tangent, binormal, normal);

		unpackedNormal = surfaceToWorldMatrix * unpackedNormal;
		tangent        = surfaceToWorldMatrix * tangent;
		binormal       = surfaceToWorldMatrix * binormal;
	}

	// ----------------------------------------------------------------

	// pixel to camera direction
	vec3 toCamera = normalize(cameraPosition - worldPosition);

	// Calculate fresnel effect
	float refractiveFactor = dot(toCamera, unpackedNormal);

	//vec3 finalColour 

	// Mix in the colour of the water
	//finalColour = mix(finalColour, waterColour, 0.2);

	FragColor = vec4(0.0, 0.2, 0.7, 1.0);
}

// ----------------------------------------------------------------