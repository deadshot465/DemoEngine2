#pragma once
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace DX
{
	class ISwapChain
	{
	public:
		ISwapChain(int width, int height, UINT bufferCount, BOOL allowTearing);
		virtual ~ISwapChain();

		virtual void Present(bool vsync) = 0;

	protected:
		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain = nullptr;
		BOOL m_allowTearing = FALSE;
		DXGI_SWAP_CHAIN_DESC1 m_swapChainDesc = {};
	};
}

