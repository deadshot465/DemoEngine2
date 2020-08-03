#pragma once
#include <memory>
#include <Windows.h>
#include "../Interfaces/IWindow.h"

class IGraphics;

namespace DX
{
	namespace DX11
	{
		class GraphicsEngine;
	}

	class Window :
		public IWindow
	{
	public:
		static RECT GetClientWindowRect(HWND hwnd) noexcept;
		
		Window(std::wstring_view title, int width, int height, bool fullScreen);
		~Window();

		// IWindow ÇâÓÇµÇƒåpè≥Ç≥ÇÍÇ‹ÇµÇΩ
		virtual bool Initialize() override;
		virtual void Setup(IGraphics* graphics) override;
		virtual void Update(float deltaTime) override;
		virtual void Render(float deltaTime) override;
		virtual void Dispose() override;

		bool IsRunning(float deltaTime) noexcept override;

	private:
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		ATOM Register();
		void Create();
		void SetHwnd(HWND hwnd);

		DEVMODEW m_devMode = {};
		float m_deltaTime = 0.0f;
		
		//std::unique_ptr<DX11::GraphicsEngine> m_graphicsEngineDX11 = nullptr;
	};
}

