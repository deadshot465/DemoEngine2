#pragma once
#include <array>
#include <d3d11_4.h>
#include <wrl/client.h>

namespace DX
{
	namespace DX11
	{
		class BufferFactory
		{
		public:
			BufferFactory() = delete;
			~BufferFactory() = delete;

			static void CreateBuffer(ID3D11Device5* device, D3D11_BIND_FLAG bindFlags, UINT byteWidth, D3D11_USAGE usage, Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer, const void* initialData = nullptr, UINT cpuAccessFlags = NULL);

			static void CreateInputLayout(ID3D11Device5* device, const std::array<D3D11_INPUT_ELEMENT_DESC, 3>& inputElementDescs, const void* shaderByteCode, SIZE_T byteCodeLength, Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);

		private:
		};
	}
}

