#include "ISwapChainDX.h"
#include <cassert>

DX::ISwapChain::ISwapChain(int width, int height, UINT bufferCount, BOOL allowTearing)
	: m_allowTearing(allowTearing)
{
#if _DEBUG
	assert(bufferCount >= 3);
#endif

	ZeroMemory(&m_swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	m_swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	m_swapChainDesc.BufferCount = bufferCount;
	m_swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_swapChainDesc.Flags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : NULL;
	m_swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	m_swapChainDesc.Height = static_cast<UINT>(height);
	m_swapChainDesc.SampleDesc.Count = 1;
	m_swapChainDesc.SampleDesc.Quality = 0;
	m_swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	m_swapChainDesc.Stereo = FALSE;
	m_swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	m_swapChainDesc.Width = static_cast<UINT>(width);
}

DX::ISwapChain::~ISwapChain()
{
	m_swapChain.Reset();
}
