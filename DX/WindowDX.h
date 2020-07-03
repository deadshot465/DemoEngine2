#pragma once
#include <memory>
#include <Windows.h>
#include "../Interfaces/IWindow.h"

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
		
	protected:
		virtual void Setup() override;
		virtual void Update() override;
		virtual void Render() override;
		virtual void Dispose() override;
		
	private:
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

		ATOM Register();
		void Create();
		void SetHwnd(HWND hwnd);

		HWND m_handle = nullptr;
		DEVMODEW m_devMode = {};
		
		std::unique_ptr<DX11::GraphicsEngine> m_graphicsEngineDX11 = nullptr;
	};
}

