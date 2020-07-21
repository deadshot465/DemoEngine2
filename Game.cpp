#include "Game.h"
#include "GLVK/WindowGLVK.h"
#include "GLVK/VK/GraphicsEngineVK.h"
#include "Interfaces/IResourceManager.h"

Game::Game(std::wstring_view title, int width, int height, bool fullScreen)
{
	m_resourceManager = std::make_unique<IResourceManager>();
	m_window = std::make_unique<GLVK::Window>(title, width, height, fullScreen);
}

Game::~Game()
{
	if (m_resourceManager) m_resourceManager.reset();
	if (m_graphics) m_graphics.reset();
	if (m_window) m_window.reset();
}

bool Game::Initialize()
{
	m_window->Initialize();
	m_graphics = std::make_unique<GLVK::VK::GraphicsEngine>(reinterpret_cast<GLFWwindow*>(m_window->GetHandle()), m_window->GetWidth(), m_window->GetHeight(), m_resourceManager.get());
	m_window->Setup(m_graphics.get());
	return m_window->IsInitialized();
}

void Game::LoadContent()
{
	if (!IsInitialized()) return;

	//m_graphics->CreateCube();
	//m_graphics->LoadModel("Models/Tank/tank.fbx");
	m_graphics->LoadModel("Models/Rainier-AK-3D/RAINIER AK _ Low4.fbx");

	m_graphics->Initialize();
}

bool Game::IsInitialized() const noexcept
{
	return m_window->IsInitialized();
}

void Game::Run()
{
	m_window->Run();
}
