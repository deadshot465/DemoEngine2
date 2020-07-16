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
		virtual void Run() override;
		virtual void Setup(IGraphics* graphics) override;

	protected:
		virtual void Update() override;
		virtual void Render() override;
		virtual void Dispose() override;

		IGraphics* Graphics = nullptr;
		
	private:
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		ATOM Register();
		void Create();
		void SetHwnd(HWND hwnd);

		DEVMODEW m_devMode = {};
		
		//std::unique_ptr<DX11::GraphicsEngine> m_graphicsEngineDX11 = nullptr;
	};
}

