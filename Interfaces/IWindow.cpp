#include "IWindow.h"

IWindow::IWindow(std::wstring_view title, int width, int height, bool fullScreen)
	: m_title(title), m_width(width), m_height(height), m_isFullScreen(fullScreen)
{
}
