#pragma once
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <memory>
#include <vector>
#include <wrl/client.h>
#include "../UtilsDX.h"
#include "UtilsDX11.h"

namespace DX
{
	namespace DX11
	{
		class BufferFactory;
		class DeviceContext;

		template <typename T>
		class Shader;

		class SwapChain;

		class GraphicsEngine
		{
		public:
			GraphicsEngine(HWND hwnd, int width, int height);
			~GraphicsEngine();

			void Update(float deltaTime);
			void Render();

		private:
			std::vector<Vertex> m_cubeVertices;
			std::vector<UINT> m_cubeIndices;
			MVP m_mvp;
			DirectionalLight m_lightObject;

			void Dispose();
			void GetAdapterAndFactory();
			void CreateDevice();
			void CreateInfoQueue();
			void LoadDefaultCube();
			void LoadDefaultShaderAndCreateBuffers(int width, int height);
			void CreateRasterizerState();
			void CreateDepthStencilState();

			inline static const D3D_FEATURE_LEVEL m_featureLevels[] =
			{
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0
			};

			Microsoft::WRL::ComPtr<IDXGIAdapter4> m_adapter = nullptr;
			Microsoft::WRL::ComPtr<IDXGIFactory7> m_factory = nullptr;
			Microsoft::WRL::ComPtr<ID3D11Device5> m_device = nullptr;
			Microsoft::WRL::ComPtr<ID3D11InfoQueue> m_infoQueue = nullptr;
			Microsoft::WRL::ComPtr<ID3D11RasterizerState2> m_rasterizerState = nullptr;
			D3D_FEATURE_LEVEL m_featureLevel = {};
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer = nullptr;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer = nullptr;
			Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout = nullptr;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_mvpConstantBuffer = nullptr;
			Microsoft::WRL::ComPtr<ID3D11Buffer> m_directionalLightConstantBuffer = nullptr;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState = nullptr;

			std::unique_ptr<SwapChain> m_swapChain = nullptr;
			std::unique_ptr<DeviceContext> m_deviceContext = nullptr;

			// These are temporarily stored here.
			// After resource manager is implemented the ownership will be transferred.
			std::unique_ptr<Shader<ID3D11VertexShader>> m_vertexShader = nullptr;
			std::unique_ptr<Shader<ID3D11PixelShader>> m_pixelShader = nullptr;
			
			int m_width = 0;
			int m_height = 0;
		};
	}
}

