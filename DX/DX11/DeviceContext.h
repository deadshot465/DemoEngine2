#pragma once
#include <array>
#include <d3d11_4.h>
#include <type_traits>
#include <wrl/client.h>

namespace DX
{
	namespace DX11
	{
		class DeviceContext
		{
		public:
			explicit DeviceContext(Microsoft::WRL::ComPtr<ID3D11DeviceContext4>& deviceContext);
			~DeviceContext();

			void ClearRenderTargetView(ID3D11RenderTargetView1* rtv, const std::array<FLOAT, 4>& clearColor);
			void ClearDepthStencilView(ID3D11DepthStencilView* dsv, D3D11_CLEAR_FLAG clearFlags);
			void SetViewport(int width, int height);
			void SetRenderTarget(ID3D11RenderTargetView1* rtv, ID3D11DepthStencilView* dsv);
			void SetRasterizerState(ID3D11RasterizerState2* rasterizerState);
			void SetDepthStencilState(ID3D11DepthStencilState* dss);
			void SetVertexBuffer(ID3D11Buffer* buffer, ID3D11InputLayout* inputLayout);
			void SetIndexBuffer(ID3D11Buffer* buffer);
			
			template <typename T>
			void SetConstantBuffer(T* shader, ID3D11Buffer* buffer, UINT slot = 0u);

			template <typename T>
			void SetShader(T* shader);

			void DrawIndices(UINT vertexCount);

			void UpdateDynamicResource(ID3D11Buffer* buffer, const void* bufferData, size_t dataSize);

			ID3D11DeviceContext4* GetDeviceContext() { return m_deviceContext.Get(); }

		private:
			Microsoft::WRL::ComPtr<ID3D11DeviceContext4> m_deviceContext = nullptr;
		};

		template<typename T>
		inline void DeviceContext::SetConstantBuffer(T* shader, ID3D11Buffer* buffer, UINT slot)
		{
			ID3D11Buffer* buffers[] = { buffer };

			if constexpr (std::is_same_v<T, ID3D11VertexShader>)
			{
				m_deviceContext->VSSetConstantBuffers(slot, _countof(buffers), buffers);
			}
			else if constexpr (std::is_same_v<T, ID3D11PixelShader>)
			{
				m_deviceContext->PSSetConstantBuffers(slot, _countof(buffers), buffers);
			}
		}

		template<typename T>
		inline void DeviceContext::SetShader(T* shader)
		{
			if constexpr (std::is_same_v<T, ID3D11VertexShader>)
			{
				m_deviceContext->VSSetShader(shader, nullptr, 0u);
			}
			else if constexpr (std::is_same_v<T, ID3D11PixelShader>)
			{
				m_deviceContext->PSSetShader(shader, nullptr, 0u);
			}
		}
	}
}

