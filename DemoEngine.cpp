#include <iostream>
#include "DX/WindowDX.h"
#include "GLVK/WindowGLVK.h"

int main()
{
	//DX::Window window{ L"Demo Engine", 1024, 768, false };
	GLVK::Window window{ L"Demo Engine", 1024, 768, false };

	try
	{
		if (window.Initialize())
		{
			while (window.IsInitialized())
				window.Run();
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An error occurred: " << ex.what() << '\n';
		return -1;
	}

    return 0;
}