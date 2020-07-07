#include "WindowGLVK.h"
#include <stdexcept>
#include "../UtilsCommon.h"

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
	
}

void GLVK::Window::Render()
{
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
}
