#pragma once
#include <array>
#include <d3d11_4.h>
#include <DirectXMath.h>
#include <stdexcept>
#include <string_view>
#include <Windows.h>

namespace DX
{
	namespace DX11
	{
		struct Vertex
		{
			static std::array<D3D11_INPUT_ELEMENT_DESC, 3> GetInputElementDescs(UINT inputSlot = 0, D3D11_INPUT_CLASSIFICATION inputClassification = D3D11_INPUT_PER_VERTEX_DATA, UINT instanceDataStepRate = 0, UINT semanticIndex = 0) noexcept
			{
				auto descs = std::array<D3D11_INPUT_ELEMENT_DESC, 3>();
				descs[0].AlignedByteOffset = static_cast<UINT>(offsetof(Vertex, Position));
				descs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				descs[0].InputSlot = inputSlot;
				descs[0].InputSlotClass = inputClassification;
				descs[0].InstanceDataStepRate = instanceDataStepRate;
				descs[0].SemanticIndex = semanticIndex;
				descs[0].SemanticName = "POSITION";

				descs[1].AlignedByteOffset = static_cast<UINT>(offsetof(Vertex, Normal));
				descs[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
				descs[1].InputSlot = inputSlot;
				descs[1].InputSlotClass = inputClassification;
				descs[1].InstanceDataStepRate = instanceDataStepRate;
				descs[1].SemanticIndex = semanticIndex;
				descs[1].SemanticName = "NORMAL";

				descs[2].AlignedByteOffset = static_cast<UINT>(offsetof(Vertex, TexCoord));
				descs[2].Format = DXGI_FORMAT_R32G32_FLOAT;
				descs[2].InputSlot = inputSlot;
				descs[2].InputSlotClass = inputClassification;
				descs[2].InstanceDataStepRate = instanceDataStepRate;
				descs[2].SemanticIndex = semanticIndex;
				descs[2].SemanticName = "TEXCOORD";

				return descs;
			}

			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT2 TexCoord;
		};

		inline void ThrowIfFailed(HRESULT result, std::string_view message)
		{
			if (FAILED(result))
				throw std::runtime_error(message.data());
		}
	}
}