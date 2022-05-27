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
		float roughness = (1.0f - cRoughnessFactor); // 0.5f;
		float3 view_direction = normalize(cCameraPosition.xyz - position);
		float3 reflect_direction = reflect(-light_vector, normal);
		float specular_factor = mul(pow(max(dot(view_direction, reflect_direction), 0.0), 16.0f), roughness);
		specular_light += float4(specular_factor * specular_light_colour.xyz, 1.0f);
	}

	return diffuse_light + ambient_light + specular_light;
}

// Normal mapping
float3 CalculateNormalsFromNormalMap(PixelInput input)
{
	float3 normalMapSample = TextureNormal.Sample(SamplerStateAnisotropic, input.tex_coord).rgb;

	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = normalize(normalMapSample * 2.0f - 1.0f);

	// Build orthonormal basis.
	float3 N = normalize(input.normal); // Normal
	float3 T = normalize(input.tangent - dot(input.tangent, N) * N); // Tangent
	float3 B = cross(N, T); // Bi-Tangent

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return normalize(bumpedNormalW);
}

// Entry point for the vertex shader - will be executed for each pixel
float4 main(PixelInput input) : SV_TARGET
{
	// Interpolating normal can unnormalize it, so normalize it.
	input.normal = normalize(input.normal);

	// Calculate normals from sampling the normal map
	float3 bumped_normal = input.normal;
	if (cMaterialNormalTexture == 1)
	{
		bumped_normal = CalculateNormalsFromNormalMap(input);
	}

	// Calculate directional light
	float4 light_colour = CalculatePointLighting(input.position, bumped_normal);

	// Apply diffuse texture
	float4 diffuse_texture = TextureDiffuse.Sample(SamplerStateAnisotropic, input.tex_coord);

	return light_colour * diffuse_texture;
}