#pragma once
#include <GLFW/glfw3.h>
#include <memory>
#include "../Interfaces/IWindow.h"
#include "VK/GraphicsEngineVK.h"

namespace GLVK
{
	namespace VK
	{
		class GraphicsEngine;
	}

	class Window :
		public IWindow
	{
	public:
		Window(std::wstring_view title, int width, int height, bool fullScreen);
		~Window();

		// IWindow ����Čp������܂���
		virtual bool Initialize() override;
		virtual void Run() override;

	protected:
		virtual void Setup() override;
		virtual void Update() override;
		virtual void Render() override;
		virtual void Dispose() override;

	private:
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod);
		void Create();

		GLFWwindow* m_handle = nullptr;
		std::unique_ptr<VK::GraphicsEngine> m_graphicsEngineVk = nullptr;
	};
}

