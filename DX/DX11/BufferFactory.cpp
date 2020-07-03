#include "BufferFactory.h"
#include "UtilsDX11.h"

void DX::DX11::BufferFactory::CreateBuffer(ID3D11Device5* device, D3D11_BIND_FLAG bindFlags, UINT byteWidth, D3D11_USAGE usage, Microsoft::WRL::ComPtr<ID3D11Buffer>& buffer, const void* initialData, UINT cpuAccessFlags)
{
    auto desc = D3D11_BUFFER_DESC();
    desc.BindFlags = bindFlags;
    desc.ByteWidth = byteWidth;
    desc.CPUAccessFlags = cpuAccessFlags;
    desc.MiscFlags = NULL;
    desc.StructureByteStride = 0;
    desc.Usage = usage;

    auto data = D3D11_SUBRESOURCE_DATA();
    data.pSysMem = initialData;

    auto res = device->CreateBuffer(&desc, initialData ? &data : nullptr, buffer.ReleaseAndGetAddressOf());
    ThrowIfFailed(res, "Failed to create buffer.");
}

void DX::DX11::BufferFactory::CreateInputLayout(ID3D11Device5* device, const std::array<D3D11_INPUT_ELEMENT_DESC, 3>& inputElementDescs, const void* shaderByteCode, SIZE_T byteCodeLength, Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
    auto res = device->CreateInputLayout(inputElementDescs.data(), static_cast<UINT>(inputElementDescs.size()), shaderByteCode, byteCodeLength, inputLayout.ReleaseAndGetAddressOf());
    ThrowIfFailed(res, "Failed to create input layout.");
}
