#pragma once
#include <chrono>
#include <string>
#include <string_view>
#include "../Interfaces/IGraphics.h"

class IGraphics;

class IWindow
{
public:
	IWindow(std::wstring_view title, int width, int height, bool fullScreen);
	virtual ~IWindow() = default;

	virtual bool Initialize() = 0;
	virtual void Setup(IGraphics* graphics) = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render(float deltaTime) = 0;
	virtual void Dispose() = 0;

	virtual bool IsRunning(float deltaTime) noexcept = 0;
	constexpr int GetWidth() const noexcept { return m_width; }
	constexpr int GetHeight() const noexcept { return m_height; }
	constexpr bool IsInitialized() const noexcept { return m_isInitialized; }
	constexpr void* GetHandle() const noexcept { return m_handle; }
	constexpr IGraphics* GetGraphics() const noexcept { return m_graphics; }

protected:
	int m_width = 0;
	int m_height = 0;
	bool m_isInitialized = false;
	bool m_isFullScreen = false;
	void* m_handle = nullptr;
	IGraphics* m_graphics = nullptr;

	std::chrono::time_point<std::chrono::steady_clock> m_lastFrameTime;
	
	std::wstring m_title = L"";
};

