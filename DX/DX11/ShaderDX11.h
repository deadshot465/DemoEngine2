#pragma once
#include <d3d11_4.h>
#include <string_view>
#include <type_traits>
#include <wrl/client.h>
#include "../../UtilsCommon.h"
#include "UtilsDX11.h"

namespace DX
{
	namespace DX11
	{
		template <typename T>
		class Shader
		{
		public:
			Shader(ID3D11Device5* device, std::string_view filePath);
			~Shader();

			T* GetShader() const noexcept;
			const std::vector<char>& GetRawByteCode() const noexcept;

		private:
			Microsoft::WRL::ComPtr<T> m_shader = nullptr;
			std::vector<char> m_shaderByteCode;
		};

		template<typename T>
		inline Shader<T>::Shader(ID3D11Device5* device, std::string_view filePath)
		{
			m_shaderByteCode = ReadFromFile(filePath);
			HRESULT res = S_OK;

			if constexpr (std::is_same_v<T, ID3D11VertexShader>)
			{
				res = device->CreateVertexShader(m_shaderByteCode.data(), static_cast<SIZE_T>(m_shaderByteCode.size()), nullptr, m_shader.GetAddressOf());
			}
			else if constexpr (std::is_same_v<T, ID3D11PixelShader>)
			{
				res = device->CreatePixelShader(m_shaderByteCode.data(), static_cast<SIZE_T>(m_shaderByteCode.size()), nullptr, m_shader.GetAddressOf());
			}
		}

		template<typename T>
		inline Shader<T>::~Shader()
		{
			m_shader.Reset();
		}

		template<typename T>
		inline T* Shader<T>::GetShader() const noexcept
		{
			return m_shader.Get();
		}

		template<typename T>
		inline const std::vector<char>& Shader<T>::GetRawByteCode() const noexcept
		{
			return m_shaderByteCode;
		}
	}
}

