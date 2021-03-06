#pragma once
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

		virtual bool IsRunning(float deltaTime) noexcept override;
		virtual bool Initialize() override;
		virtual void Setup(IGraphics* graphics) override;

	protected:
		virtual void Update(float deltaTime) override;
		virtual void Render(float deltaTime) override;
		virtual void Dispose() override;

	private:
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod);
		void Create();
	};
}

