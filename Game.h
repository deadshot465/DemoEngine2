#pragma once
#include <chrono>
#include <memory>
#include <string_view>
#include "Interfaces/IGraphics.h"
#include "Interfaces/IResourceManager.h"
#include "Interfaces/ISceneManager.h"
#include "Interfaces/IWindow.h"

class IWindow;
class IGraphics;
class IResourceManager;
class ISceneManager;

class Game
{
public:
	Game(std::wstring_view title, int width, int height, bool fullScreen);
	~Game();

	bool Initialize();
	void LoadContent();
	bool IsInitialized() const noexcept;
	void Update();
	void Render();
	void Run();
	
private:
	std::unique_ptr<IWindow> m_window = nullptr;
	std::unique_ptr<IGraphics> m_graphics = nullptr;
	std::unique_ptr<IResourceManager> m_resourceManager = nullptr;
	std::unique_ptr<ISceneManager> m_sceneManager = nullptr;

	std::chrono::time_point<std::chrono::steady_clock> m_lastFrameTime;
	std::chrono::time_point<std::chrono::steady_clock> m_currentFrameTime;
	float m_deltaTime = 0.0f;
};

