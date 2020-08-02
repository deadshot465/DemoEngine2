#include <iostream>
#include <spdlog/spdlog.h>
#ifdef _WIN32
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
			game.LoadContent();

			while (game.IsInitialized())
				game.Run();
		}
	}
	catch (const std::exception& ex)
	{
		spdlog::error(ex.what());
		return -1;
	}

    return 0;
}