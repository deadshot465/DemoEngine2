#include <iostream>
#ifdef WIN32
#include "DX/WindowDX.h"
#endif
#include "Game.h"

int main()
{
	Game game{ L"Demo Engine", 1024, 768, false };

	try
	{
		if (game.Initialize())
		{
			while (game.IsInitialized())
				game.Run();
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An error occurred: " << ex.what() << '\n';
		return -1;
	}

    return 0;
}