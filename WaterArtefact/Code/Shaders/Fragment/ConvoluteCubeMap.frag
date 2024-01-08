#version 330 core

in  vec3 localPosition;
out vec4 FragColor;

uniform samplerCube enviromentMap;

const float PI = 3.14159265359;

void main()
{
	vec3 normal = normalize(localPosition);

	vec3 irradiance = vec3(0.0);

	vec3 up    = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, normal));
	     up    = normalize(cross(normal, right));

	float sampleDelta     = 0.02;
	float numberOfSamples = 0.0;

	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// Convert from sphereical to cartesian space
			vec3 tangentSpace = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			
			// Tangent to world space
			vec3 sampleVector = tangentSpace.x * right + tangentSpace.y * up + tangentSpace.z * normal;

			// * cos(theta) because lighting becomes weaker at larger angles
			// * sin(theta) because there are smaller sample sizes as we go up the hemisphere
			irradiance += texture(enviromentMap, sampleVector).rgb * cos(theta) * sin(theta);

			numberOfSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(numberOfSamples));

	FragColor = vec4(irradiance, 1.0);
}