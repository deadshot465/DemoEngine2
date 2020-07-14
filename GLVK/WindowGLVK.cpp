#include "WindowGLVK.h"
#include <stdexcept>

GLVK::Window::Window(std::wstring_view title, int width, int height, bool fullScreen)
	: IWindow(title, width, height, fullScreen)
{

}

GLVK::Window::~Window()
{
	glfwDestroyWindow(m_handle);
	glfwTerminate();
}

bool GLVK::Window::Initialize()
{
	try
	{
		Create();

		return m_isInitialized;
	}
	catch (const std::exception&)
	{
		throw;
	}
}

void GLVK::Window::Run()
{
	while (!glfwWindowShouldClose(m_handle))
	{
		glfwPollEvents();
		
		Update();
		Render();
	}

	Dispose();
}

void GLVK::Window::Setup()
{
	try
	{
		m_graphicsEngineVk = std::make_unique<VK::GraphicsEngine>(m_handle, m_width, m_height);
	}
	catch (const std::exception&)
	{
		throw;
	}
}

void GLVK::Window::Update()
{
    using namespace std::chrono;

    auto current_frame = high_resolution_clock::now();
    auto elapsed = duration<float, seconds::period>(current_frame - m_lastFrameTime).count();

    try {
        m_graphicsEngineVk->Update(elapsed);
    }
    catch (const std::exception&)
    {
        throw;
    }

    m_lastFrameTime = current_frame;
}

void GLVK::Window::Render()
{
    try {
        m_graphicsEngineVk->Render();
    }
    catch (const std::exception&)
    {
        throw;
    }
}

void GLVK::Window::Dispose()
{
	m_isInitialized = false;
}

void GLVK::Window::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	default:
		break;
	}
}

void GLVK::Window::Create()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_handle = glfwCreateWindow(m_width, m_height, WstringToString(m_title).data(), nullptr, nullptr);
	if (!m_handle)
	{
		throw std::runtime_error("Failed to create window with GLFW.\n");
	}

	Setup();

	glfwShowWindow(m_handle);
	glfwFocusWindow(m_handle);
	glfwMakeContextCurrent(m_handle);

	glfwSetKeyCallback(m_handle, KeyCallback);
	
	m_isInitialized = true;
	m_lastFrameTime = std::chrono::high_resolution_clock::now();
}
