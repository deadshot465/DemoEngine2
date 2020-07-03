#include "GraphicsEngineDX11.h"
#include <array>
#include <chrono>
#include <cassert>
#include "BufferFactory.h"
#include "DeviceContext.h"
#include "ShaderDX11.h"
#include "SwapChainDX11.h"
#include "../UtilsDX.h"

using namespace Microsoft::WRL;

DX::DX11::GraphicsEngine::GraphicsEngine(HWND hwnd, int width, int height)
	: m_width(width), m_height(height)
{
	try
	{
		GetAdapterAndFactory();
		CreateDevice();
		CreateInfoQueue();

		BOOL allow_tearing = FALSE;
		auto res = m_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, static_cast<UINT>(sizeof(BOOL)));

		m_swapChain = std::make_unique<SwapChain>(m_device.Get(), m_factory.Get(), hwnd, width, height, 3, allow_tearing);

		LoadDefaultCube();
		LoadDefaultShaderAndCreateBuffers(width, height);

		CreateRasterizerState();
		CreateDepthStencilState();
	}
	catch (const std::exception&)
	{
		Dispose();
		throw;
	}
}

DX::DX11::GraphicsEngine::~GraphicsEngine()
{
	Dispose();
}

void DX::DX11::GraphicsEngine::Update(float deltaTime)
{
	using namespace std::chrono;

	static auto start_time = high_resolution_clock::now();
	auto current_time = high_resolution_clock::now();
	auto duration_between = duration<float, seconds::period>(current_time - start_time).count();

	static auto elapsed_time_since_last_frame = 0.0f;
	elapsed_time_since_last_frame += deltaTime;
	

	auto model = DirectX::XMMatrixIdentity();
	auto rotation_x = DirectX::XMMatrixRotationX(duration_between * DirectX::XMConvertToRadians(90.0f));
	auto rotation_y = DirectX::XMMatrixRotationY(duration_between * DirectX::XMConvertToRadians(90.0f));
	auto rotation_z = DirectX::XMMatrixRotationZ(duration_between * DirectX::XMConvertToRadians(90.0f));

	model = rotation_z * rotation_y * rotation_x * model;
	DirectX::XMStoreFloat4x4A(&m_mvp.Model, model);
	m_deviceContext->UpdateDynamicResource(m_mvpConstantBuffer.Get(), &m_mvp, sizeof(MVP));
}

void DX::DX11::GraphicsEngine::Render()
{
	static const std::array<FLOAT, 4> clear_colors = {
		0.0f, 0.0f, 0.0f, 1.0f
	};

	m_deviceContext->ClearRenderTargetView(m_swapChain->GetRenderTargetView(), clear_colors);
	m_deviceContext->ClearDepthStencilView(m_swapChain->GetDepthStencilView(), D3D11_CLEAR_DEPTH);
	m_deviceContext->SetRenderTarget(m_swapChain->GetRenderTargetView(), m_swapChain->GetDepthStencilView());

	m_deviceContext->SetViewport(m_width, m_height);
	m_deviceContext->SetRasterizerState(m_rasterizerState.Get());
	m_deviceContext->SetDepthStencilState(m_depthStencilState.Get());

	m_deviceContext->SetShader(m_vertexShader->GetShader());
	m_deviceContext->SetShader(m_pixelShader->GetShader());
	m_deviceContext->SetVertexBuffer(m_vertexBuffer.Get(), m_inputLayout.Get());
	m_deviceContext->SetIndexBuffer(m_indexBuffer.Get());
	m_deviceContext->SetConstantBuffer(m_vertexShader->GetShader(), m_mvpConstantBuffer.Get());
	m_deviceContext->SetConstantBuffer(m_pixelShader->GetShader(), m_directionalLightConstantBuffer.Get(), 1u);
	
	m_deviceContext->DrawIndices(static_cast<UINT>(m_cubeIndices.size()));
	m_swapChain->Present();
}

void DX::DX11::GraphicsEngine::Dispose()
{
	m_depthStencilState.Reset();
	m_rasterizerState.Reset();
	m_mvpConstantBuffer.Reset();
	m_inputLayout.Reset();
	m_indexBuffer.Reset();
	m_vertexBuffer.Reset();
	m_pixelShader.reset();
	m_vertexShader.reset();
	m_deviceContext.reset();
	m_swapChain.reset();
	m_infoQueue.Reset();
	m_device.Reset();
	m_adapter.Reset();
	m_factory.Reset();
}

void DX::DX11::GraphicsEngine::GetAdapterAndFactory()
{
	auto res = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_factory));
	ThrowIfFailed(res, "Failed to create DXGI factory.");

	ComPtr<IDXGIAdapter1> adapter = nullptr;
	SIZE_T dedicated_memory = 0;

	for (UINT i = 0; m_factory->EnumAdapters1(i, adapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		auto desc = DXGI_ADAPTER_DESC1();
		adapter->GetDesc1(&desc);

		if (desc.DedicatedVideoMemory > dedicated_memory &&
			((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0) &&
			(SUCCEEDED(D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_DEBUG, m_featureLevels, _countof(m_featureLevels), D3D11_SDK_VERSION, nullptr, nullptr, nullptr))))
		{
			dedicated_memory = desc.DedicatedVideoMemory;
			res = adapter.As(&m_adapter);
			ThrowIfFailed(res, "Failed to query adapter.");
		}
	}

	adapter.Reset();
}

void DX::DX11::GraphicsEngine::CreateDevice()
{
#if _DEBUG
	assert(m_adapter);
#endif

	ComPtr<ID3D11Device> device = nullptr;
	ComPtr<ID3D11DeviceContext> context = nullptr;

	auto res = D3D11CreateDevice(m_adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, D3D11_CREATE_DEVICE_DEBUG, m_featureLevels, _countof(m_featureLevels), D3D11_SDK_VERSION, device.GetAddressOf(), &m_featureLevel, context.GetAddressOf());
	ThrowIfFailed(res, "Failed to create D3D11 device.");

	ThrowIfFailed(device.As(&m_device), "Failed to query D3D11 device.");
	
	ComPtr<ID3D11DeviceContext4> _context = nullptr;
	ThrowIfFailed(context.As(&_context), "Failed to query device context.");

	m_deviceContext = std::make_unique<DeviceContext>(_context);
	
	context.Reset();
	device.Reset();
}

void DX::DX11::GraphicsEngine::CreateInfoQueue()
{
	ThrowIfFailed(m_device.As(&m_infoQueue), "Failed to query info queue.");

	m_infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	m_infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_INFO, TRUE);
	m_infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, TRUE);

	D3D11_MESSAGE_SEVERITY ignore_severities[] = {
		D3D11_MESSAGE_SEVERITY_INFO,
	};

	auto filter = D3D11_INFO_QUEUE_FILTER();
	filter.DenyList.NumSeverities = _countof(ignore_severities);
	filter.DenyList.pSeverityList = ignore_severities;

	ThrowIfFailed(m_infoQueue->PushStorageFilter(&filter), "Failed to push filter into storage.");
}

void DX::DX11::GraphicsEngine::LoadDefaultCube()
{
	m_cubeVertices =
	{
		// Front Face
		{ { -0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },

		// Top Face
		{ { -0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },

		// Back Face
		{ {  0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },

		// Bottom Face
		{ { -0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },

		// Left Face
		{ { -0.5f, -0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
		{ { -0.5f, -0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },

		// Right Face
		{ {  0.5f, -0.5f, -0.5f }, {  1.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f, -0.5f }, {  1.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
		{ {  0.5f,  0.5f,  0.5f }, {  1.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
		{ {  0.5f, -0.5f,  0.5f }, {  1.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
	};

	m_cubeIndices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,

		8, 9, 10, 10, 11, 8,
		12, 13, 14, 14, 15, 12,

		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20
	};
}

void DX::DX11::GraphicsEngine::LoadDefaultShaderAndCreateBuffers(int width, int height)
{
	m_vertexShader = std::make_unique<Shader<ID3D11VertexShader>>(m_device.Get(), "DX/DX11/Shaders/CubeVS.cso");
	m_pixelShader = std::make_unique<Shader<ID3D11PixelShader>>(m_device.Get(), "DX/DX11/Shaders/CubePS.cso");

	BufferFactory::CreateBuffer(m_device.Get(), D3D11_BIND_VERTEX_BUFFER, static_cast<UINT>(sizeof(Vertex) * m_cubeVertices.size()), D3D11_USAGE_DEFAULT, m_vertexBuffer, m_cubeVertices.data());

	BufferFactory::CreateBuffer(m_device.Get(), D3D11_BIND_INDEX_BUFFER, static_cast<UINT>(sizeof(UINT) * m_cubeIndices.size()), D3D11_USAGE_DEFAULT, m_indexBuffer, m_cubeIndices.data());

	BufferFactory::CreateInputLayout(m_device.Get(), Vertex::GetInputElementDescs(), m_vertexShader->GetRawByteCode().data(), sizeof(char) * m_vertexShader->GetRawByteCode().size(), m_inputLayout);

	m_mvp = MVP();
	auto model = DirectX::XMMatrixIdentity();
	auto rotation_x = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(45.0f));
	auto rotation_y = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(-45.0f));
	auto rotation_z = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(45.0f));
	model = rotation_y * model;
	DirectX::XMStoreFloat4x4A(&m_mvp.Model, model);

	auto eye = DirectX::XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);
	auto look_at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	auto up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMStoreFloat4x4A(&m_mvp.View, DirectX::XMMatrixLookAtLH(eye, look_at, up));

	static constexpr auto fov = DirectX::XMConvertToRadians(45.0f);
	auto perspective = DirectX::XMMatrixPerspectiveFovLH(fov, static_cast<float>(width) / height, 0.1f, 100.0f);
	DirectX::XMStoreFloat4x4A(&m_mvp.Projection, perspective);

	BufferFactory::CreateBuffer(m_device.Get(), D3D11_BIND_CONSTANT_BUFFER, static_cast<UINT>(sizeof(m_mvp)), D3D11_USAGE_DYNAMIC, m_mvpConstantBuffer, &m_mvp, D3D11_CPU_ACCESS_WRITE);

	auto diffuse = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	auto light_direction = DirectX::XMVectorSet(0.0f, 5.0f, 0.0f, 0.0f);
	auto ambient_intensity = 0.1f;
	auto specular_intensity = 0.5f;
	DirectX::XMStoreFloat4(&m_lightObject.Diffuse, diffuse);
	DirectX::XMStoreFloat3(&m_lightObject.LightDirection, light_direction);
	m_lightObject.AmbientIntensity = ambient_intensity;
	m_lightObject.SpecularIntensity = specular_intensity;

	BufferFactory::CreateBuffer(m_device.Get(), D3D11_BIND_CONSTANT_BUFFER, static_cast<UINT>(sizeof(m_lightObject)), D3D11_USAGE_DEFAULT, m_directionalLightConstantBuffer, &m_lightObject);
}

void DX::DX11::GraphicsEngine::CreateRasterizerState()
{
	auto desc = D3D11_RASTERIZER_DESC2();
	ZeroMemory(&desc, sizeof(D3D11_RASTERIZER_DESC2));
	desc.AntialiasedLineEnable = TRUE;
	desc.ConservativeRaster = D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	desc.CullMode = D3D11_CULL_BACK;
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0.0f;
	desc.DepthClipEnable = FALSE;
	desc.FillMode = D3D11_FILL_SOLID;
	desc.ForcedSampleCount = 0;
	desc.FrontCounterClockwise = FALSE;
	desc.MultisampleEnable = TRUE;
	desc.ScissorEnable = FALSE;
	desc.SlopeScaledDepthBias = 0.0f;

	ThrowIfFailed(m_device->CreateRasterizerState2(&desc, m_rasterizerState.GetAddressOf()), "Failed to create rasterizer state.");
}

void DX::DX11::GraphicsEngine::CreateDepthStencilState()
{
	auto desc = D3D11_DEPTH_STENCIL_DESC();
	ZeroMemory(&desc, sizeof(desc));
	desc.DepthEnable = TRUE;
	desc.DepthFunc = D3D11_COMPARISON_LESS;
	desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc.StencilEnable = FALSE;
	
	ThrowIfFailed(m_device->CreateDepthStencilState(&desc, m_depthStencilState.GetAddressOf()), "Failed to create depth stencil state.");
}
