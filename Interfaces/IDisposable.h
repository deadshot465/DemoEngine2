#pragma once
#include <string>
#include <string_view>

class IDisposable
{
public:
	IDisposable(void* pUserData)
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