#include "DeviceContext.h"
#include <utility>
#include "UtilsDX11.h"

DX::DX11::DeviceContext::DeviceContext(Microsoft::WRL::ComPtr<ID3D11DeviceContext4>& deviceContext)
	: m_deviceContext(std::move(deviceContext))
{
}

DX::DX11::DeviceContext::~DeviceContext()
{
	m_deviceContext.Reset();
}

void DX::DX11::DeviceContext::ClearRenderTargetView(ID3D11RenderTargetView1* rtv, const std::array<FLOAT, 4>& clearColor)
{
	m_deviceContext->ClearRenderTargetView(rtv, clearColor.data());
}

void DX::DX11::DeviceContext::ClearDepthStencilView(ID3D11DepthStencilView* dsv, D3D11_CLEAR_FLAG clearFlags)
{
	m_deviceContext->ClearDepthStencilView(dsv, clearFlags, 1.0f, 0);
}

void DX::DX11::DeviceContext::SetViewport(int width, int height)
{
	auto vp = D3D11_VIEWPORT();
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<FLOAT>(width);
	vp.Height = static_cast<FLOAT>(height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	m_deviceContext->RSSetViewports(1u, &vp);
}

void DX::DX11::DeviceContext::SetRenderTarget(ID3D11RenderTargetView1* rtv, ID3D11DepthStencilView* dsv)
{
	ID3D11RenderTargetView* rtvs[] = { rtv };

	m_deviceContext->OMSetRenderTargets(static_cast<UINT>(_countof(rtvs)), rtvs, dsv);
}

void DX::DX11::DeviceContext::SetRasterizerState(ID3D11RasterizerState2* rasterizerState)
{
	m_deviceContext->RSSetState(rasterizerState);
}

void DX::DX11::DeviceContext::SetDepthStencilState(ID3D11DepthStencilState* dss)
{
	m_deviceContext->OMSetDepthStencilState(dss, 0u);
}

void DX::DX11::DeviceContext::SetVertexBuffer(ID3D11Buffer* buffer, ID3D11InputLayout* inputLayout)
{
	ID3D11Buffer* buffers[] = { buffer };
	UINT stride = static_cast<UINT>(sizeof(Vertex));
	UINT offset = 0;

	m_deviceContext->IASetVertexBuffers(0u, 1u, buffers, &stride, &offset);
	m_deviceContext->IASetInputLayout(inputLayout);
}

void DX::DX11::DeviceContext::SetIndexBuffer(ID3D11Buffer* buffer)
{
	m_deviceContext->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, 0u);
}

void DX::DX11::DeviceContext::DrawIndices(UINT indexCount)
{
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->DrawIndexed(indexCount, 0u, 0u);
}

void DX::DX11::DeviceContext::UpdateDynamicResource(ID3D11Buffer* buffer, const void* bufferData, size_t dataSize)
{
	auto mapped = D3D11_MAPPED_SUBRESOURCE();

	auto res = m_deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, NULL, &mapped);
	ThrowIfFailed(res, "Failed to map the subresource.");

	memcpy(mapped.pData, bufferData, dataSize);

	m_deviceContext->Unmap(buffer, 0);
}
