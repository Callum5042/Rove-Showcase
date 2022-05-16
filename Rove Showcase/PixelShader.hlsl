#include "ShaderData.hlsli"

// Point lighting
float4 CalculatePointLighting(float3 position, float3 normal)
{
	float4 diffuse_light = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 ambient_light = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 specular_light = float4(0.0f, 0.0f, 0.0f, 1.0f);

	for (int i = 0; i < cLightCount; ++i)
	{
		float4 diffuse_light_colour = cPointLight[i].lightPointDiffuse;
		float4 ambient_light_colour = cPointLight[i].lightPointAmbient;
		float4 specular_light_colour = cPointLight[i].lightPointSpecular;

		// Diffuse lighting
		float3 light_vector = normalize(cPointLight[i].lightPointPosition.xyz - position);
		diffuse_light += saturate(dot(light_vector, normal)) * diffuse_light_colour;

		// Ambient lighting
		ambient_light += ambient_light_colour;

		// Specular lighting
		float3 view_direction = normalize(cCameraPosition.xyz - position);
		float3 reflect_direction = reflect(-light_vector, normal);
		float specular_factor = pow(max(dot(view_direction, reflect_direction), 0.0), 16.0f);
		specular_light += float4(specular_factor * specular_light_colour.xyz, 1.0f);
	}

	return diffuse_light + ambient_light + specular_light;
}

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
	input.normal = normalize(input.normal);

	// Calculate directional light
	float4 light_colour = CalculatePointLighting(input.position, input.normal);

	return light_colour;
}