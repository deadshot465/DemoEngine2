#pragma once
#include <chrono>
#include <GLFW/glfw3.h>
#include <memory>
#include "../Interfaces/IWindow.h"

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

		virtual bool Initialize() override;
		virtual void Run() override;
		virtual void Setup(IGraphics* graphics) override;

	protected:
		virtual void Update() override;
		virtual void Render() override;
		virtual void Dispose() override;

	private:
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod);
		void Create();

		std::chrono::time_point<std::chrono::steady_clock> m_lastFrameTime;
		//std::unique_ptr<VK::GraphicsEngine> m_graphicsEngineVk = nullptr;
	};
}

