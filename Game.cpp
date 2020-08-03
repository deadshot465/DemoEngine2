#include "Game.h"
#include "GLVK/WindowGLVK.h"
#include "GLVK/VK/GraphicsEngineVK.h"
#include "Interfaces/IResourceManager.h"

Game::Game(std::wstring_view title, int width, int height, bool fullScreen)
	: m_lastFrameTime(std::chrono::steady_clock::now())
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

	/*m_graphics->CreateCube(Vector3(0.5f, 0.0f, 0.5f), Vector3(1.0f), Vector3(0.0f), Vector4(0.0f, 1.0f, 1.0f, 1.0f));*/
	m_graphics->LoadModel("Models/Tank/tank.fbx", Vector3(0.5f, 0.0f, 0.5f), Vector3(1.5f), Vector3(45.0f), Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	m_graphics->LoadModel("Models/Tank/tank.fbx", Vector3(-0.5f, 0.0f, -0.5f), Vector3(1.75f), Vector3(-45.0f), Vector4(1.0f, 0.0f, 1.0f, 1.0f));
	m_graphics->LoadModel("Models/Tank/tank.fbx", Vector3(-0.25f, 0.0f, 0.25f), Vector3(2.0f), Vector3(30.0f), Vector4(0.0f, 1.0f, 1.0f, 1.0f));
	//m_graphics->LoadModel("Models/Rainier-AK-3D/RAINIER AK _ Low4.fbx");
	//m_graphics->LoadModel("Models/Pistol/Handgun_fbx_7.4_binary.fbx");
	//m_graphics->LoadModel("Models/Wolf/Wolf.fbx");

	m_graphics->Initialize();
}

bool Game::IsInitialized() const noexcept
{
	return m_window->IsInitialized();
}

void Game::Update()
{
	using namespace std::chrono;

	if (!m_window) return;

	try
	{
		m_window->Update(m_deltaTime);
	}
	catch (const std::exception&)
	{
		throw;
	}
}

void Game::Render()
{
	if (!m_window) return;
	
	try
	{
		m_window->Render(m_deltaTime);
	}
	catch (const std::exception&)
	{
		throw;
	}
}

void Game::Run()
{
	using namespace std::chrono;

	while (m_window->IsRunning(m_deltaTime))
	{
		m_currentFrameTime = steady_clock::now();
		m_deltaTime = duration<float, seconds::period>(m_currentFrameTime - m_lastFrameTime).count();

		Update();
		Render();

		m_lastFrameTime = m_currentFrameTime;
	}

	m_window->Dispose();
}
