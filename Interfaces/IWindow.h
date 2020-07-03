#pragma once
#include <chrono>
#include <string>
#include <string_view>

class IWindow
{
public:
	IWindow(std::wstring_view title, int width, int height, bool fullScreen);
	virtual ~IWindow() = default;

	virtual bool Initialize() = 0;
	virtual void Run() = 0;

	constexpr int GetWidth() { return m_width; }
	constexpr int GetHeight() { return m_height; }
	constexpr bool IsInitialized() { return m_isInitialized; }

protected:
	virtual void Setup() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void Dispose() = 0;

	int m_width = 0;
	int m_height = 0;
	bool m_isInitialized = false;
	bool m_isFullScreen = false;

	std::chrono::time_point<std::chrono::steady_clock> m_lastFrameTime;
	
	std::wstring m_title = L"";
};

