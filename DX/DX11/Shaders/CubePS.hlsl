struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
};

cbuffer DirectionalLight : register(b1)
{
	float4 Diffuse;
	float3 LightDirection;
	float AmbientIntensity;
	float SpecularIntensity;
}

float4 main(PS_INPUT input) : SV_TARGET
{
	// Ambient
	float4 ambient = Diffuse * AmbientIntensity;

	// Diffuse Light
	float4 light_direction = normalize(-float4(LightDirection, 0.0));
	float4 normal = normalize(float4(input.Normal, 0.0));
	float intensity = max(dot(normal, light_direction), 0.0f);
	float4 diffuse = intensity * Diffuse;

	float4 result = (ambient + diffuse) * float4(1.0, 1.0, 0.0, 1.0);

	return result;
}