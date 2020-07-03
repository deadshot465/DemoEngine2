#pragma once
#include <DirectXMath.h>

namespace DX
{
	struct MVP
	{
		DirectX::XMFLOAT4X4A Model;
		DirectX::XMFLOAT4X4A View;
		DirectX::XMFLOAT4X4A Projection;
	};

	struct alignas(16) DirectionalLight
	{
		DirectX::XMFLOAT4 Diffuse;
		DirectX::XMFLOAT3 LightDirection;
		float AmbientIntensity;
		float SpecularIntensity;
	};
}