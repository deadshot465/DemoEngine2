#include "SwapChainDX11.h"
#include <cassert>
#include "UtilsDX11.h"

using namespace Microsoft::WRL;

DX::DX11::SwapChain::SwapChain(ID3D11Device5* device, IDXGIFactory7* factory, HWND hwnd, int width, int height, UINT bufferCount, BOOL allowTearing)
	: ISwapChain(width, height, bufferCount, allowTearing)
{
#if _DEBUG
	assert(device);
#endif
	
	ComPtr<IDXGISwapChain1> swap_chain = nullptr;

	auto res = factory->CreateSwapChainForHwnd(device, hwnd, &m_swapChainDesc, nullptr, nullptr, swap_chain.GetAddressOf());
	ThrowIfFailed(res, "Failed to create DXGI swap chain.");
	ThrowIfFailed(swap_chain.As(&m_swapChain), "Failed to query DXGI swap chain.");

	CreateRenderTargetView(device);
	CreateDepthStencilView(device, width, height);

	swap_chain.Reset();
}

DX::DX11::SwapChain::~SwapChain()
{
	m_dsv.Reset();
	m_rtv.Reset();
}

void DX::DX11::SwapChain::Present(bool vsync)
{
	UINT flags = m_allowTearing ? DXGI_PRESENT_ALLOW_TEARING : NULL;
	UINT interval = (vsync && !m_allowTearing) ? 1 : 0;
	m_swapChain->Present(interval, flags);
}

void DX::DX11::SwapChain::CreateRenderTargetView(ID3D11Device5* device)
{
	ComPtr<ID3D11Texture2D1> buffer = nullptr;
	UINT index = 0;

	auto res = m_swapChain->GetBuffer(index, IID_PPV_ARGS(&buffer));
	ThrowIfFailed(res, "Failed to get back buffer from swap chain.");

	res = device->CreateRenderTargetView1(buffer.Get(), nullptr, m_rtv.GetAddressOf());
	ThrowIfFailed(res, "Failed to create render target view.");

	buffer.Reset();
}

void DX::DX11::SwapChain::CreateDepthStencilView(ID3D11Device5* device, int width, int height)
{
	auto texture = ComPtr<ID3D11Texture2D1>();
	
	auto desc = D3D11_TEXTURE2D_DESC1();
	desc.ArraySize = 1u;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = NULL;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.Height = static_cast<UINT>(height);
	desc.MipLevels = 0u;
	desc.MiscFlags = NULL;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.TextureLayout = D3D11_TEXTURE_LAYOUT_UNDEFINED;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.Width = static_cast<UINT>(width);

	auto res = device->CreateTexture2D1(&desc, nullptr, texture.GetAddressOf());
	ThrowIfFailed(res, "Failed to create texture for depth stencil view.");

	res = device->CreateDepthStencilView(texture.Get(), nullptr, m_dsv.GetAddressOf());
	ThrowIfFailed(res, "Failed to create depth stencil view.");
}
