#include "ShaderData.hlsli"

// Entry point for the vertex shader - will be executed for each pixel
float4 main(VertexOutput vertex_output) : SV_TARGET
{
	//return vertex_output.colour;
	return float4(1.0f, 0.0f, 0.0f, 1.0f);
}