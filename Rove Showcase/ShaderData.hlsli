// Vertex input
struct VertexInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tex_diffuse : TEXCOORD0;
};

// Vertex output / pixel input structure
struct PixelInput
{
	float4 positionClipSpace : SV_POSITION;
	float3 position : POSITION;
	float3 normal : NORMAL;
};

// Camera buffer
cbuffer CameraBuffer : register(b0)
{
	matrix cView;
	matrix cProjection;
	float4 cCameraPosition;
}

// World constant buffer
cbuffer WorldBuffer : register(b1)
{
	matrix cWorld;
	matrix cWorldInverse;
}

// World constant buffer
cbuffer LocalWorldBuffer : register(b3)
{
	matrix cLocalWorld;
}

// Point light structure
struct PointLight
{
	float4 lightPointPosition;
	float4 lightPointDiffuse;
	float4 lightPointAmbient;
	float4 lightPointSpecular;
};

// Point light buffer
cbuffer PointLightBuffer : register(b2)
{
	int cLightCount;
	float3 padding;

	PointLight cPointLight[255];
}