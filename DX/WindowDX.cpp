#include "WindowDX.h"
#include <stdexcept>
#include <cassert>
#include "../Interfaces/IGraphics.h"
#include "DX11/GraphicsEngineDX11.h"

RECT DX::Window::GetClientWindowRect(HWND hwnd) noexcept
{
	auto rect = RECT();
	::GetClientRect(hwnd, &rect);
	return rect;
}

DX::Window::Window(std::wstring_view title, int width, int height, bool fullScreen)
	: IWindow(title, width, height, fullScreen)
{

}

DX::Window::~Window()
{
	::DestroyWindow(reinterpret_cast<HWND>(m_handle));
	::UnregisterClassW(L"DemoEngineClass", nullptr);
}

bool DX::Window::Initialize()
{
	try
	{
		if (!Register())
		{
			throw std::runtime_error("Failed to register the window class.");
		}

		Create();

		return m_isInitialized;
	}
	catch (const std::exception&)
	{
		throw;
	}
}

void DX::Window::Run()
{
	auto msg = MSG();
	
	while (::PeekMessageW(&msg, reinterpret_cast<HWND>(m_handle), NULL, NULL, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
	}

	::Sleep(1);
}

void DX::Window::Setup(IGraphics* graphics)
{
	auto rect = GetClientWindowRect(reinterpret_cast<HWND>(m_handle));
	auto width = static_cast<int>(rect.right - rect.left);
	auto height = static_cast<int>(rect.bottom - rect.top);

	try
	{
		Graphics = graphics;
	}
	catch (const std::exception&)
	{
		throw;
	}
}

void DX::Window::Update()
{
	using namespace std::chrono;

	if (!Graphics) return;

	auto current_frame = high_resolution_clock::now();
	auto elapsed = duration<float, seconds::period>(current_frame - m_lastFrameTime).count();

	Graphics->Update(elapsed);

	m_lastFrameTime = current_frame;
}

void DX::Window::Render()
{
	if (!Graphics) return;
	Graphics->Render();
}

void DX::Window::Dispose()
{
	m_isInitialized = false;
}

LRESULT DX::Window::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	try
	{
		switch (msg)
		{
		case WM_PAINT:
		{
			auto window = reinterpret_cast<Window*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
			window->Update();
			window->Render();
			break;
		}
		case WM_CREATE:
		{
			auto window = reinterpret_cast<Window*>(reinterpret_cast<LPCREATESTRUCTW>(lparam)->lpCreateParams);
			window->SetHwnd(hwnd);
			::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
			break;
		}
		case WM_DESTROY:
		{
			auto window = reinterpret_cast<Window*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));
			window->Dispose();
			::PostQuitMessage(0);
			break;
		}
		default:
			return ::DefWindowProcW(hwnd, msg, wparam, lparam);
		}

		return NULL;
	}
	catch (const std::exception&)
	{
		throw;
	}
}

ATOM DX::Window::Register()
{
	auto wc = WNDCLASSEXW();
	ZeroMemory(&wc, sizeof(WNDCLASSEXW));
	wc.cbClsExtra = NULL;
	wc.cbSize = static_cast<UINT>(sizeof(WNDCLASSEXW));
	wc.cbWndExtra = NULL;
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
	wc.hCursor = nullptr;
	wc.hIcon = nullptr;
	wc.hIconSm = nullptr;
	wc.hInstance = nullptr;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"DemoEngineClass";
	wc.lpszMenuName = L"";
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

	return ::RegisterClassExW(&wc);
}

void DX::Window::Create()
{
	DWORD width = ::GetSystemMetrics(SM_CXSCREEN);
	DWORD height = ::GetSystemMetrics(SM_CYSCREEN);
	DWORD x = 0;
	DWORD y = 0;

	if (m_isFullScreen)
	{
		ZeroMemory(&m_devMode, sizeof(DEVMODEW));
		m_devMode.dmSize = sizeof(DEVMODEW);
		m_devMode.dmBitsPerPel = 32;
		m_devMode.dmFields = DM_BITSPERPEL | DM_PELSHEIGHT | DM_PELSWIDTH;
		m_devMode.dmPelsHeight = height;
		m_devMode.dmPelsWidth = width;

		::ChangeDisplaySettingsW(&m_devMode, CDS_FULLSCREEN);
	}
	else
	{
		width = m_width;
		height = m_height;
		x = (::GetSystemMetrics(SM_CXSCREEN) - width) / 2;
		y = (::GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	}

	m_handle = ::CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, L"DemoEngineClass", m_title.c_str(), WS_OVERLAPPEDWINDOW, x, y, width, height, nullptr, nullptr, nullptr, this);

	if (!m_handle)
	{
		throw std::runtime_error("Failed to create window.");
	}

	::SetWindowLongPtrW(reinterpret_cast<HWND>(m_handle), GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(LoadCursorW(nullptr, IDC_ARROW)));
	::SetWindowLongPtrW(reinterpret_cast<HWND>(m_handle), GCLP_HICON, reinterpret_cast<LONG_PTR>(LoadIconW(nullptr, IDI_WINLOGO)));
	::SetWindowLongPtrW(reinterpret_cast<HWND>(m_handle), GCLP_HICONSM, reinterpret_cast<LONG_PTR>(LoadIconW(nullptr, IDI_WINLOGO)));

	::ShowWindow(reinterpret_cast<HWND>(m_handle), SW_SHOW);
	::SetForegroundWindow(reinterpret_cast<HWND>(m_handle));
	::SetFocus(reinterpret_cast<HWND>(m_handle));

	::UpdateWindow(reinterpret_cast<HWND>(m_handle));

	m_isInitialized = true;
	m_lastFrameTime = std::chrono::high_resolution_clock::now();
}

void DX::Window::SetHwnd(HWND hwnd)
{
	m_handle = hwnd;
}
