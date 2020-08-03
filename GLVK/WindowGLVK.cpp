#include "WindowGLVK.h"
#include <stdexcept>

GLVK::Window::Window(std::wstring_view title, int width, int height, bool fullScreen)
	: IWindow(title, width, height, fullScreen)
{

}

GLVK::Window::~Window()
{
	glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(m_handle));
	glfwTerminate();
}

bool GLVK::Window::IsRunning(float deltaTime) noexcept
{
	return !glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(m_handle));
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

void GLVK::Window::Setup(IGraphics* graphics)
{
	try
	{
		m_graphics = graphics;
	}
	catch (const std::exception&)
	{
		throw;
	}
}

void GLVK::Window::Update(float deltaTime)
{
    using namespace std::chrono;

	glfwPollEvents();

	if (!m_graphics) return;

    try {
        m_graphics->Update(deltaTime);
    }
    catch (const std::exception&)
    {
        throw;
    }
}

void GLVK::Window::Render(float deltaTime)
{
	if (!m_graphics) return;

    try {
        m_graphics->Render();
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

	glfwShowWindow(reinterpret_cast<GLFWwindow*>(m_handle));
	glfwFocusWindow(reinterpret_cast<GLFWwindow*>(m_handle));
	glfwMakeContextCurrent(reinterpret_cast<GLFWwindow*>(m_handle));

	glfwSetKeyCallback(reinterpret_cast<GLFWwindow*>(m_handle), KeyCallback);
	
	m_isInitialized = true;
	m_lastFrameTime = std::chrono::steady_clock::now();
}
