#pragma once
#include <string>
#include <string_view>

class IDisposable
{
public:
	IDisposable(void* pUserData = nullptr)
		: m_handle(pUserData)
	{

	}

	virtual ~IDisposable() = default;
	virtual void Dispose() = 0;

	std::string Name = "";

protected:
	void* m_handle = nullptr;
	bool m_isDisposed = false;
};

template <typename T>
concept Disposable = std::is_base_of_v<IDisposable, T> && std::is_convertible_v<const volatile T*, const volatile IDisposable*>;