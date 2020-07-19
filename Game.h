#pragma once
#include <memory>
#include <string_view>
#include "Interfaces/IGraphics.h"
#include "Interfaces/IResourceManager.h"
#include "Interfaces/IWindow.h"

class IWindow;
class IGraphics;
class IResourceManager;

class Game
{
public:
	Game(std::wstring_view title, int width, int height, bool fullScreen);
	~Game();

	bool Initialize();
	bool IsInitialized() const noexcept;
	void Run();
	
private:
	std::unique_ptr<IWindow> m_window = nullptr;
	std::unique_ptr<IGraphics> m_graphics = nullptr;
	std::unique_ptr<IResourceManager> m_resourceManager = nullptr;
};

