#pragma once
#include <d3d11_4.h>
#include <vector>
#include "../../Interfaces/ISwapChainDX.h"

namespace DX
{
	namespace DX11
	{
		class SwapChain
			: public ISwapChain
		{
		public:
			SwapChain(ID3D11Device5* device, IDXGIFactory7* factory, HWND hwnd, int width, int height, UINT bufferCount, BOOL allowTearing);
			~SwapChain();

			// ISwapChain ÇâÓÇµÇƒåpè≥Ç≥ÇÍÇ‹ÇµÇΩ
			virtual void Present(bool vsync = false) override;

			ID3D11RenderTargetView1* GetRenderTargetView() const noexcept { return m_rtv.Get(); }
			ID3D11DepthStencilView* GetDepthStencilView() const noexcept { return m_dsv.Get(); }

		private:
			void CreateRenderTargetView(ID3D11Device5* device);
			void CreateDepthStencilView(ID3D11Device5* device, int width, int height);

			Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> m_rtv = nullptr;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv = nullptr;
		};
	}
}
